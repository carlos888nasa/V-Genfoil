# Documentación Técnica de V-Genfoil

Este es el índice de la documentación generada a partir del código (Doxygen).

Si buscas qué es V-Genfoil, cómo compilarlo o la estructura de carpetas del repositorio, esa información vive en el `README.md` del proyecto, no aquí. Esta página es el punto de entrada para navegar por las clases y módulos reales del código fuente.

Para profundizar en el porqué de cada decisión, la base matemática o los algoritmos, no se repite aquí — están documentados aparte:
- **Justificación de arquitectura:** \ref design
- **Algoritmos y matemáticas:** \ref math
- **Derivaciones teóricas completas:** `docs/theory/`

---

## Mapa de módulos

El proyecto se organiza en cinco áreas, correspondientes a `src/`:

### 1. Geometría (`src/geometry/`)
Genera las coordenadas del perfil a partir de los 11 parámetros PARSEC.
- **Clase principal:** \ref Parsec — resuelve el sistema lineal por superficie (extradós/intradós) y evalúa `z(x)` para cualquier punto de la cuerda.
- **Estructura de entrada:** \ref ParsecParameters

### 2. Solver (`src/solver/`)
Panel method propio acoplado con capa límite viscosa (Thwaites + Michel + Head).
*Aún en fase de estudio matemático — sin clases implementadas todavía. Cuando exista la primera clase del solver, enlázala aquí con `\ref`.*

### 3. Optimizador (`src/optimizer/`)
Implementación de NSGA-II sobre el espacio de diseño PARSEC.
*Pendiente de implementación.*

### 4. Visualización (`src/viz/`)
Renderizado OpenGL con sistema de partículas lagrangianas para el campo de flujo.
*Pendiente de implementación.*

### 5. Concurrencia (`src/concurrency/`)
Utilidades de `std::thread` / `std::atomic` / `std::mutex` para evaluar la población de candidatos en paralelo entre generaciones. Arquitectura establecida en la Fase 0.
*Pendiente de referenciar clases concretas.*

---

## Estado actual
Fase 0 (arquitectura de threading) completa. El módulo de geometría (PARSEC) es, de momento, el único con clases documentadas en este árbol de Doxygen. El resto de módulos se irán enlazando aquí a medida que tengan clases reales que referenciar — evita añadir descripciones de "cómo funcionará" antes de que exista el código, para que esta página no se desincronice con `docs/DESIGN.md`.