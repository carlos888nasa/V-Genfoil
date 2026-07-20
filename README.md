# V-Genfoil

Una herramienta en C++ para generar y evaluar geometrías de perfiles aerodinámicos candidatos, combinando un panel method propio con un modelo de capa límite viscosa acoplado a una búsqueda genética multiobjetivo (NSGA-II), con visualización en tiempo real del campo de flujo mediante OpenGL. El resultado es un conjunto reducido de perfiles candidatos óptimos según Pareto, validados de forma cruzada con Xfoil, para que una persona elija entre ellos.

## Qué hace este proyecto

V-Genfoil **no** busca un único perfil "óptimo". Lo que hace es:

1. Generar una población de geometrías de perfiles candidatos, cada una parametrizada con coeficientes **PARSEC** (11-12 variables de diseño con significado físico directo por superficie: radio del borde de ataque, posición/altura del punto de máximo espesor, ángulos del borde de salida, etc.).
2. Evaluar cada candidato mediante un solver aerodinámico propio (panel method + acoplamiento de capa límite viscosa) para obtener Cl, Cd, Cm.
3. Usar **NSGA-II** (un algoritmo genético multiobjetivo) para evolucionar la población a lo largo de varias generaciones, clasificando a los candidatos por dominancia de Pareto en lugar de un único valor escalar.
4. Producir un **frente de Pareto** final: un puñado de candidatos no dominados que representan distintos compromisos (p. ej. mínima resistencia vs. máxima sustentación vs. margen de entrada en pérdida).
5. Dejar la selección final entre esos candidatos al ingeniero, según los requisitos concretos de la misión.

Los finalistas se contrastan con **Xfoil** como referencia independiente (no se usa dentro del propio bucle de optimización), y, cuando es posible, se validan además con datos de túnel de viento antes de fabricar.

## Hoja de ruta

| Versión | Alcance | Estado |
|---|---|---|
| V1 | Generación y evaluación de perfiles 2D (panel method + acoplamiento viscoso + NSGA-II + OpenGL) | En progreso |
| V2 | Extensión con Vortex Lattice Method (VLM) a alas 3D | Diferida |
| V3 | Modelo sustituto con red neuronal para acelerar la evaluación | Diferida |

## Componentes principales

- **Geometría (PARSEC):** genera las coordenadas de la superficie del perfil a partir de los parámetros de diseño, resolviendo dos sistemas lineales cerrados (uno por superficie). Ver `docs/DESIGN.md` para la derivación completa.
- **Solver:** panel method propio para la distribución de presión no viscosa, acoplado con el método de Thwaites (capa límite laminar), el criterio de Michel (transición) y el método de Head (capa límite turbulenta) para estimar resistencia y predecir entrada en pérdida.
- **Optimizador:** algoritmo genético multiobjetivo NSGA-II sobre el espacio de diseño PARSEC.
- **Visualización:** renderizado OpenGL con un sistema de partículas lagrangianas para visualizar el comportamiento del flujo en tiempo real.
- **Concurrencia:** `std::thread` / `std::atomic` / `std::mutex` para evaluar la población en paralelo a lo largo de las generaciones.

## Compilación

El proyecto se desarrolla en Linux (WSL2/Ubuntu), pero se distribuye también como binario nativo con enlazado estático para otras plataformas. Ver `docs/DESIGN.md` → "Build & distribution" para la explicación completa y la configuración del toolchain.

Referencia rápida:
```bash
# Build nativo en Linux (desarrollo)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Build cross-compilado para Windows (desde Linux, vía MinGW-w64)
cmake -B build-win -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64-toolchain.cmake
cmake --build build-win
```
Los binarios precompilados para Linux, Windows y macOS se publican automáticamente en cada release etiquetado (ver el workflow de CI).

## Documentación

- **Documentación de código:** Doxygen, generada a partir de los comentarios del código fuente. Ver `docs/DESIGN.md` → "Documentation conventions" para el estilo de comentarios usado en todo el código.
- **Justificación de diseño, decisiones de arquitectura y detalles de build/distribución:** `docs/DESIGN.md`.
- **Algoritmos y matemáticas usadas, con su justificación:** `docs/ALGORITHMS_AND_MATH.md`.
- **Teoría (derivación de PARSEC, formulación del panel method, ecuaciones de capa límite, formulación de NSGA-II):** `docs/theory/` (se documenta aparte).

## Estructura del proyecto

```
V-Genfoil/
├── src/
│   ├── geometry/        # Parametrización PARSEC
│   ├── solver/          # Panel method + acoplamiento viscoso (Thwaites/Michel/Head)
│   ├── optimizer/        # Implementación de NSGA-II
│   ├── viz/              # Renderizado OpenGL + sistema de partículas lagrangianas
│   └── concurrency/      # Utilidades de threading
├── cmake/                # Toolchain files (p. ej. cross-compilación con MinGW-w64)
├── validation/           # Casos de validación con Xfoil (solo referencia, no en runtime)
├── docs/
│   ├── DESIGN.md
│   ├── ALGORITHMS_AND_MATH.md
│   └── theory/
└── .github/workflows/    # CI: matriz de build multiplataforma
```

## Estado

Proyecto activo del equipo VANTUS AeroDesign Team. La Fase 0 (arquitectura de threading) está completa; el foco actual está en el estudio previo de la base matemática (flujo potencial, capa límite, NSGA-II) antes de empezar a implementar.

## Licencia

Por definir.