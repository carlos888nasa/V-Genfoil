#ifndef PARSEC_H
#define PARSEC_H

#include <array>

/**
 *@file parsec.h
 *@brief Definición de la clase Parsec y sus parámetros geométricos
 *@author Carlos Ortega Mantero
 *@date 20/07/2026
*/

/**
 *@struct ParsecParameters
 *@brief Define los parámetros físicos de la geometría PARSEC (formulación
 *       clásica de Sobieczky, 11 variables). Actúan como los genes en el
 *       algoritmo NSGA-II
*/
struct ParsecParameters {
    double r_le; /**< Radio del borde de ataque */

    //Extradós (Upper)
    double x_up; /**< Posición X de la cresta superior */
    double y_up; /**< Posición Y de la cresta superior */
    double d2y_up; /**< Curvatura en la cresta superior */

    //Intradós (Lower)
    double x_lo; /**< Posición X de la cresta inferior */
    double y_lo; /**< Posición Y de la cresta inferior */
    double d2y_lo; /**< Curvatura en la cresta inferior */

    //Borde de salida (Trailing Edge)
    double y_te; /**< Coordenada Y media del borde de la salida */
    double dy_te; /**< Espesor total del borde de salida */
    double alpha_te; /**< Ángulo de dirección (bisectriz), en radianes */
    double beta_te; /**< Ángulo de cuña (apertura), en radianes */
};

/**
 *@class Parsec
 *@brief Generador de perfiles aerodinámicos usando parametrización PARSEC
*/

class Parsec {
private:
    ParsecParameters params;
    std::array<double, 6> a_upper{};
    std::array<double, 6> a_lower{};

    /**
     *@brief Resuelve el sistema lineal 5x5 (a2…a6) para una superficie,
     *       con a1 fijado de antemano a partir de r_le e isUpper
     *
     *@param r_le Radio del borde de ataque
     *@param x_m Posición x del extremo (cresta o valle) de la superficie
     *@param y_m Posición y del extremo
     *@param d2y_m Curvatura en el extremo
     *@param y_te_surf Coordenada Y del borde de salida PARA ESTA superficie
     *                 (ya repartida a partir de y_te, dy_te)
     *@param th_te_surf Ángulo del borde de salida PARA ESTA superficie
     *                  (ya repartido a partir de alpha_te, beta_te), en radianes
     *@param isUpper True para extradós (a1 > 0), false para intrados (a1 < 0)
     *@return Array con los 6 coeficientes polinómicos [a1..a6]
    */
    static std::array<double, 6> calculateCoefficients(
        double r_le, double x_m, double y_m, double d2y_m,
        double y_te_surf, double th_te_surf, bool isUpper);

public:
    /**
     * @brief Constructor que iniciliza los parámetros y precalcula los coeficientes
     * @param parameters Estructura con los 11 genes físicos de la geometría
    */
    explicit Parsec(const ParsecParameters &parameters);

    /**
     *@brief Evalúa la coordenada Y del perfil para una X dada
     *@param x Coordenada normalizada a lo largo de la cuerda
     *@param isUpper True para el extradós, False para intradós
     *@return Coordenada Y calaculada correspondientes a la posición X
    */
    [[nodiscard]] double getY(double x, bool isUpper) const;

    /**
     * @brief Exporta las coordenadas del perfil a un archivo .dat compatible con Xfoil
     * @param filename Ruta y nombre del archivo (ej. "perfil_generado.dat")
     * @param num_points Número de puntos a evaluar por cada superficie (extradós e intradós)
     */
    void exportToDat(const std::string &filename, int num_points = 100) const;
};

#endif //PARSEC_H
