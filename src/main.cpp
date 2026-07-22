#include <iostream>
#include <string>
#include "geometry/parsec.h"
#include "utils/config.h"

int main() {
    std::cout << "Iniciando V-Genfoil V1...\n";

    ParsecParameters params;
    params.r_le = 0.01;      // Radio del borde de ataque
    params.x_up = 0.4;       // Posición X del espesor máximo superior
    params.y_up = 0.06;      // Altura máxima superior
    params.d2y_up = -0.3;    // Curvatura superior
    params.x_lo = 0.4;       // Posición X inferior
    params.y_lo = -0.06;     // Altura mínima inferior
    params.d2y_lo = 0.3;     // Curvatura inferior
    params.y_te = 0.0;       // Altura del borde de salida
    params.dy_te = 0.002;    // Espesor del borde de salida
    params.alpha_te = -0.05; // Ángulo de la bisectriz
    params.beta_te = 0.1;    // Ángulo de apertura de la cuña

    // Inicializar el objeto pasándole los parámetros requeridos
    Parsec perfil(params);

    std::string ruta_salida = std::string(Config::AIRFOIL_OUTPUT_DIR) + "perfil_prueba.dat";

    perfil.exportToDat(ruta_salida,100);

    return 0;
}
