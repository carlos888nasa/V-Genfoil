# V-Genfoil — Diseño y Convenciones de Ingeniería

Este documento cubre *cómo* se construye y documenta el proyecto: decisiones de arquitectura, convenciones de documentación de código, y cómo un código desarrollado en Linux se entrega como ejecutable nativo a usuarios que no usan Linux. La teoría física/matemática (derivación de PARSEC, formulación del panel method, ecuaciones de capa límite, formulación de NSGA-II) vive por separado en `docs/theory/`.

---

## 1. Visión general de la arquitectura

```
Vector de diseño (parámetros PARSEC)
        │
        ▼
  Generador de geometría  ──►  Coordenadas de la superficie del perfil
        │
        ▼
  Solver panel method ──►  Distribución de presión no viscosa
        │
        ▼
  Acoplamiento viscoso (Thwaites → Michel → Head) ──►  Cl, Cd, Cm, flag de pérdida
        │
        ▼
  Clasificación NSGA-II (dominancia de Pareto en toda la población)
        │
        ▼
  Siguiente generación  (se repite durante N generaciones)
        │
        ▼
  Frente de Pareto final  ──►  Una persona elige los finalistas
        │
        ▼
  Validación cruzada contra Xfoil (solo finalistas)
```

Cada etapa está deliberadamente desacoplada: el generador de geometría no sabe nada del solver, el solver no sabe nada de NSGA-II, y NSGA-II solo ve una estructura `Fitness{Cl, Cd, Cm, converged}`. Esto es lo que permite testear cada módulo de forma independiente contra soluciones analíticas/de referencia (ver §4).

### Por qué panel method y no CFD completo

Un panel method + acoplamiento de capa límite es órdenes de magnitud más barato por evaluación que resolver Navier-Stokes, lo cual importa porque NSGA-II necesita evaluar una población completa (decenas o cientos de candidatos) en cada generación, durante muchas generaciones. El coste asumido es precisión cerca de separación/entrada en pérdida, por eso los finalistas se contrastan con Xfoil y, cuando es posible, con datos de túnel de viento — esta herramienta es un *filtro rápido sobre un espacio de diseño grande*, no un sustituto de una validación de mayor fidelidad.

### Por qué NSGA-II y no optimización mono-objetivo

El espacio de diseño tiene objetivos que compiten entre sí (resistencia vs. sustentación vs. margen de entrada en pérdida). Un optimizador mono-objetivo obligaría a colapsar esto en un único score ponderado de antemano, ocultando el compromiso real al ingeniero. NSGA-II en cambio devuelve un frente de Pareto — un conjunto de candidatos donde ninguno domina a otro en todos los objetivos — de forma que la decisión de compromiso específica de la misión la toma explícitamente una persona, al final.

### Por qué PARSEC y no una parametrización polinómica genérica

Los coeficientes de PARSEC se calculan *a partir de* parámetros con significado físico (radio del borde de ataque, posición del espesor máximo, ángulos del borde de salida), en lugar de ser coeficientes polinómicos sueltos sin interpretación. Esto importa directamente para NSGA-II: cada gen del vector de diseño es interpretable físicamente y se puede acotar de forma independiente, de modo que mutación/crossover se pueden restringir para producir geometrías fabricables en vez de formas matemáticamente válidas pero físicamente absurdas.

---

## 2. Convenciones de documentación

La documentación a nivel de código usa **Doxygen** (el equivalente en C++ de Javadoc), no una wiki ni un documento de prosa aparte, porque queda físicamente pegada al código que describe y es mucho menos probable que se desincronice.

### Configuración de Doxygen

Ajustes clave forzados en el `Doxyfile`:
```
EXTRACT_ALL          = NO      # obliga a documentar explícitamente cada símbolo público
WARN_IF_UNDOCUMENTED = YES
WARN_AS_ERROR        = YES     # el build falla si la API pública no está documentada
USE_MDFILE_AS_MAINPAGE = docs/theory/index.md
```
`WARN_AS_ERROR` convierte la documentación en un requisito de compilación, no en una buena práctica opcional.

### Estilo de comentarios

Toda clase pública y función no trivial incluye:

```cpp
/**
 * @brief Evaluates lift and drag coefficients for a candidate airfoil.
 *
 * Runs the panel method for the inviscid pressure distribution, then
 * couples Thwaites/Michel/Head to correct for viscous drag. See
 * docs/theory/panel_method.md for the full derivation.
 *
 * @param candidate Airfoil candidate with valid PARSEC geometry
 * @param alpha_deg Angle of attack in degrees
 * @param reynolds  Reynolds number based on chord length
 * @return Fitness{Cl, Cd, Cm} for use in NSGA-II ranking
 *
 * @warning Not thread-safe if candidate's cached geometry buffer is shared
 *          across threads without external synchronization.
 * @note Stall/separation is signaled via Fitness::converged == false, not
 *       an exception — check this before trusting Cl/Cd.
 */
Fitness evaluateCandidate(AirfoilCandidate& candidate, double alpha_deg, double reynolds);
```

*(El comentario en sí se escribe en inglés, como es convención estándar en Doxygen/C++ open source — pero las reglas de estilo aplicadas son estas:)*

Reglas aplicadas de forma consistente:

- **`@see`, nunca re-derivar.** Los comentarios apuntan a `docs/theory/*.md` para las ecuaciones subyacentes en lugar de repetirlas — la teoría repetida se desincroniza en el momento en que se actualiza un lado y el otro no.
- **`@warning` para concurrencia y casos límite numéricos.** Todo lo que toque estado compartido (`std::atomic`/`std::mutex`) o comportamiento de convergencia se marca explícitamente, ya que son los modos de fallo más fáciles de reintroducir sin querer.
- **`@defgroup` por módulo físico, no por carpeta** (`panelmethod`, `boundarylayer`, `nsga2`), para que el HTML generado se pueda navegar pensando en la física, no en el árbol de ficheros.
- **Las invariantes se documentan, no solo las firmas.** Por ejemplo, si una cantidad debe ser monótona hasta la separación, esa asunción se indica explícitamente — si se viola en silencio, el solver produce resultados físicamente absurdos sin lanzar excepción ni fallar en compilación.

### CI

El HTML de Doxygen se regenera y publica automáticamente (GitHub Actions → GitHub Pages) en cada push a `main`, para que la referencia generada nunca se desincronice del código fuente.

---

## 3. Entorno de desarrollo

Desarrollado con **CLion sobre un toolchain WSL2 (Ubuntu)**: CLion es el editor/IDE (corre en Windows), pero la compilación, la depuración (GDB) y el análisis de memoria (Valgrind) ocurren dentro de WSL. Es una separación deliberada, no un compromiso:

- **Valgrind no existe en Windows nativo** — es solo Linux/macOS, y esto importa en un código con concurrencia manual (`std::thread`/`std::mutex`/`std::atomic`) donde los bugs de memoria y de carreras de datos son difíciles de detectar de otra forma.
- **GDB dentro de WSL** es más fiable que GDB vía MinGW en Windows nativo.
- Desarrollar en Linux mantiene el entorno coherente con el stack objetivo final (embedded/flight software, herramientas Linux-first), aunque V1 en sí sea una herramienta de escritorio.

Configuración: `Settings → Build, Execution, Deployment → Toolchains` → añadir un toolchain **WSL** apuntando a la distro Ubuntu; CLion detecta automáticamente `gcc`/`g++`/`gdb`/`cmake` dentro de ella. El directorio del proyecto debe vivir dentro del filesystem de WSL (`/home/<usuario>/...`), no en `/mnt/c/...`, para evitar I/O lenta entre filesystems y problemas de reconfiguración de CMake.

### Enfoque de depuración

La depuración se trata como comprobación sistemática de hipótesis, no como prueba y error con prints:

1. Formular una hipótesis explícita de qué está fallando y por qué, antes de tocar nada.
2. Usar GDB (vía el debugger visual de CLion) — breakpoints, `watch` sobre variables sospechosas (p. ej. `Cp[i]`, `theta`) para atrapar el punto exacto en el que un valor se vuelve no físico (NaN, negativo donde no debería), y `backtrace` en los crashes.
3. Usar Valgrind Memcheck (CLion: `Run → Valgrind Memcheck`) para problemas de memoria/carreras de datos, especialmente en torno al thread pool.
4. Bisecar el pipeline (geometría → panel method → acoplamiento viscoso → fitness → NSGA-II) en vez de revisar línea a línea, para localizar en qué etapa se introdujo un bug.
5. Validar los módulos numéricos contra soluciones analíticas conocidas (§4) como primera línea de defensa — si el panel method no reproduce el Cp analítico de un cilindro, el bug está ahí, no más adelante en NSGA-II.

---

## 4. Estrategia de validación

- **Casos analíticos:** la salida del panel method se comprueba contra soluciones cerradas (p. ej. flujo alrededor de un cilindro) sin acoplamiento de capa límite, aislando así la corrección de la parte de geometría/flujo potencial puro.
- **Validación cruzada con Xfoil:** se aplica solo a los finalistas del frente de Pareto, no a toda la población de cada generación — Xfoil es una referencia de validación, nunca se llama dentro del propio bucle de optimización.
- **Datos de túnel de viento (VANTUS):** la comprobación final antes de fabricar, cuando esté disponible — ninguna estimación numérica (panel method o Xfoil) se trata como sustituto de datos experimentales, sobre todo cerca de la entrada en pérdida, donde ambas son menos fiables (flujo separado, fuertemente no lineal).

---

## 5. Build y distribución

El desarrollo ocurre en Linux (WSL2), pero la herramienta se distribuye como ejecutable nativo para las plataformas que realmente usan los usuarios finales de aeroespacial — típicamente Windows — sin que tengan que montar ningún entorno Linux.

### Cross-compilación (MinGW-w64)

Desde el entorno de desarrollo en Linux:
```bash
sudo apt install mingw-w64
cmake -B build-win -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64-toolchain.cmake
cmake --build build-win
```
Esto genera un `.exe` nativo de Windows usando `x86_64-w64-mingw32-g++`, sin necesitar una máquina Windows para compilar. Eigen es header-only, así que no necesita tratamiento especial; GLFW (usado para la ventana/contexto de OpenGL) es multiplataforma por diseño y no requiere cambios de código fuente entre targets.

### Matriz de build en CI (preferido para releases)

GitHub Actions compila de forma nativa en runners `ubuntu-latest`, `windows-latest` y `macos-latest` en cada release etiquetado, produciendo tres binarios nativos sin depender de que la cross-compilación sea correcta. Es el patrón estándar para distribuir herramientas C++ de código abierto y es la vía principal de distribución de este proyecto.

### Enlazado estático

Los builds de release usan:
```
-static-libgcc -static-libstdc++ -static
```
para que el ejecutable entregado no tenga dependencias externas de DLLs en tiempo de ejecución — un doble clic debería ejecutarlo en una máquina limpia sin ningún paso de instalación de runtime aparte, ya que los usuarios finales son ingenieros aeroespaciales, no desarrolladores montando entornos de build.