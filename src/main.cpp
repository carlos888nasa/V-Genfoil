#include <iostream>
#include "geometry/parsec.h"

int main() {
    std::cout << "Iniciando V-Genfoil V1...\n";
    std::cout << "Cargando modulo de geometria PARSEC...\n";

    // Creamos un perfil de prueba y le asignamos un valor
    ParsecParameters test_airfoil;
    test_airfoil.r_le = 0.015;

    std::cout << "Prueba de enlace exitosa. Radio del borde de ataque: " << test_airfoil.r_le << "\n";

    return 0;
}
