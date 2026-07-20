# V-Genfoil — Algoritmos y Matemáticas Utilizadas

Este documento resume **qué método se usa en cada parte del proyecto y por qué se eligió ese y no otro**, para que cualquiera del equipo pueda cuestionar las decisiones o proponer alternativas mejores. No es documentación de uso ni de arquitectura de código (eso está en `README.md` y `DESIGN.md`) — es específicamente el mapa matemático/algorítmico del proyecto.

Estado: **en fase de estudio previo a implementación**, ningún método está codificado todavía.

---

## 1. Generación de geometría — PARSEC

**Qué es:** parametrización de perfiles aerodinámicos mediante un polinomio de potencias semienteras (6 términos por superficie), cuyos coeficientes se calculan resolviendo un sistema de ecuaciones impuesto por 11-12 parámetros con significado físico directo (radio de borde de ataque, posición/altura/curvatura del punto de máximo espesor, ángulos y espesor del borde de salida).

**Por qué esta y no otra parametrización:**
- Cada parámetro de diseño es físicamente interpretable y acotable (a diferencia de, p. ej., splines de control points genéricos o modos de Hicks-Henne), lo cual importa porque estos parámetros son directamente los genes del algoritmo genético (§4) — poder poner límites físicamente sensatos a cada gen evita generar geometrías no fabricables.
- El sistema de ecuaciones es lineal (salvo una componente, ver abajo) y barato de resolver — relevante porque se genera una geometría nueva por cada individuo de cada generación.

**Alternativas que se podrían investigar:** CST (Class-Shape Transformation, Kulfan), que es más reciente que PARSEC y en algunos estudios da mejor cobertura del espacio de diseño con menos parámetros. Si alguien quiere profundizar, este sería el punto de comparación más directo.

**Matemática de base:**
- Resolución de sistemas lineales (LU / eliminación gaussiana) — sistema 5×5 por superficie tras aislar el coeficiente asociado al radio de borde de ataque, que es la única condición no lineal.
- Derivadas de funciones con exponentes fraccionarios (para las condiciones de pendiente y curvatura).

---

## 2. Solver aerodinámico — Panel Method (flujo potencial)

**Qué es:** se discretiza la superficie del perfil en paneles finitos, cada uno con una distribución de singularidades (vórtice y/o fuente), y se resuelve un sistema lineal que impone velocidad normal nula en cada punto de control más la condición de Kutta en el borde de salida. De ahí sale la distribución de presión no viscosa.

**Por qué este método y no CFD completo (Navier-Stokes / RANS):**
- Coste computacional: el panel method resuelve un sistema lineal N×N por evaluación; una simulación CFD completa tarda órdenes de magnitud más. Como el algoritmo genético (§4) necesita evaluar poblaciones enteras durante muchas generaciones, el coste por evaluación es la restricción dominante del diseño.
- Trade-off asumido: pierde precisión cerca de separación/entrada en pérdida (flujo fuertemente no lineal), lo cual se compensa con validación cruzada posterior contra Xfoil y, si es posible, túnel de viento (ver `DESIGN.md` §4).

**Matemática de base:**
- Flujo potencial 2D: ecuación de Laplace, potencial de velocidad, función de corriente.
- Singularidades elementales: fuente, sumidero, doblete, vórtice puntual, y su generalización a distribuciones continuas sobre un panel finito.
- Principio de superposición (linealidad de Laplace).
- Condición de Kutta (velocidad finita/tangencial igual en el borde de salida) — condición adicional impuesta para forzar solución física única, sin la cual el sistema estaría indeterminado.
- Teorema de Kutta-Joukowski ($L' = \rho V_\infty \Gamma$) para pasar de circulación resuelta a coeficiente de sustentación.
- Resolución de sistemas lineales (mismo bloque matemático que en PARSEC).

**Alternativas que se podrían investigar:** formulación de vórtices constantes vs. fuentes+vórtices combinados por panel (hay varias variantes clásicas — Hess-Smith, formulación de Lewis, etc. — con distintos trade-offs de estabilidad numérica). Si alguien quiere aportar algo con impacto real, revisar qué formulación concreta de panel method conviene más aquí sería el punto de mayor valor.

---

## 3. Acoplamiento viscoso — Capa límite

**Qué es:** tres métodos encadenados que corrigen el resultado no viscoso del panel method para estimar resistencia y predecir separación/pérdida:

| Método | Función |
|---|---|
| **Thwaites** | Desarrollo de la capa límite laminar (correlación cerrada) |
| **Michel** | Predicción del punto de transición laminar → turbulenta |
| **Head** | Desarrollo de la capa límite turbulenta (entrainment method, integración numérica) |

**Por qué esta combinación y no otra:**
- Es la combinación estándar en herramientas de referencia del campo (es, de hecho, la misma que usa Xfoil internamente), lo que facilita comparar resultados de forma consistente.
- Thwaites es una correlación cerrada (barata) para la parte laminar; Head requiere integración numérica pero solo se activa tras la transición — balance razonable de coste/precisión.

**Alternativas que se podrían investigar:** método $e^N$ para predicción de transición (más preciso que Michel pero bastante más caro computacionalmente, requiere resolver estabilidad lineal); si alguien quiere estudiar si el coste extra merece la pena para este caso de uso, es un punto abierto real.

**Matemática de base:**
- Ecuación integral de cantidad de movimiento (momentum integral equation) — base común de los tres métodos.
- Concepto de espesor de desplazamiento ($\delta^*$) y de cantidad de movimiento ($\theta$).
- Acoplamiento inviscido-viscoso: $\delta^*$ modifica la geometría efectiva que "ve" el panel method (esto conecta directamente con §2).
- Integración numérica (trapecio/Simpson) para el método de Head.
- Interpolación/búsqueda de raíces para localizar el punto exacto de transición o separación entre nodos discretos.

---

## 4. Optimización — NSGA-II

**Qué es:** algoritmo genético multiobjetivo. Genera una población de candidatos (vectores PARSEC), los evalúa (Cl, Cd, Cm vía §2+§3), y evoluciona la población durante N generaciones seleccionando por **dominancia de Pareto** en vez de un único score. El resultado final es un frente de Pareto de candidatos no dominados entre sí.

**Por qué NSGA-II y no optimización mono-objetivo o gradiente:**
- Los objetivos (mínima resistencia, máxima sustentación, margen de entrada en pérdida) compiten entre sí — un optimizador de un solo objetivo obligaría a fijar de antemano los pesos relativos entre ellos, ocultando el trade-off real. NSGA-II devuelve el conjunto de compromisos óptimos y deja la decisión final a un ingeniero según la misión.
- Es un método basado en población, no en gradiente — no requiere que el solver (§2+§3) sea diferenciable, lo cual es relevante porque el panel method + capa límite acoplada no da gradientes analíticos limpios.

**Alternativas que se podrían investigar:** MOEA/D o SPEA2 como alternativas multiobjetivo a NSGA-II (distintas estrategias de mantenimiento de diversidad); o CMA-ES si en algún momento se colapsa a un único objetivo compuesto. Punto abierto para quien quiera comparar rendimiento de convergencia.

**Matemática/algoritmos de base:**
- Dominancia de Pareto (definición formal).
- Fast non-dominated sorting (algoritmo de ordenación por frentes, el que da nombre al método).
- Crowding distance (métrica de diversidad dentro de un mismo frente, evita colapsar a un único punto del espacio de soluciones).
- Crossover simulado binario (SBX) y mutación polinómica — operadores genéticos estándar para variables de diseño continuas (los parámetros PARSEC).

---

## 5. Validación

**Qué es:** verificación del solver (§2+§3) contra:
1. Soluciones analíticas conocidas (cilindro con corriente uniforme + doblete, sin capa límite) — valida solo la parte de flujo potencial de forma aislada.
2. Xfoil, aplicado únicamente a los finalistas del frente de Pareto (no a toda la población de cada generación, por coste computacional).
3. Datos de túnel de viento (VANTUS), donde sea posible, como verificación final antes de fabricar.

**Por qué este esquema en capas:** aislar dónde está un error (¿en el flujo potencial puro? ¿en el acoplamiento viscoso?) requiere poder validar cada pieza por separado antes de validar el conjunto — comparar solo contra Xfoil de entrada no permite diferenciar en qué módulo está un fallo si los resultados no cuadran.

---

## 6. Resumen de dependencias matemáticas transversales

| Bloque matemático | Usado en |
|---|---|
| Sistemas lineales (LU) | PARSEC (§1), Panel method (§2) |
| Flujo potencial / singularidades | Panel method (§2) |
| Ecuaciones integrales de capa límite | Capa límite (§3) |
| Integración numérica | Capa límite (§3), cálculo de fuerzas |
| Interpolación / búsqueda de raíces | Capa límite (§3) |
| Teoría de dominancia / algoritmos genéticos | Optimizador (§4) |

---

## Puntos abiertos para investigación (resumen)

Si alguien del equipo quiere profundizar en algo con potencial de mejora real sobre lo ya decidido, estos son los puntos concretos, de más a menos impacto esperado:

1. **Formulación exacta del panel method** (vórtices constantes vs. fuente+vórtice, Hess-Smith vs. Lewis) — §2.
2. **CST como alternativa a PARSEC** para la parametrización geométrica — §1.
3. **Método $e^N$ vs. Michel** para transición, si el coste computacional se puede asumir — §3.
4. **MOEA/D o SPEA2 vs. NSGA-II** para el algoritmo multiobjetivo — §4.