//
// Created by carlo on 22/07/2026.
//

#ifndef V_GENFOIL_CONFIG_H
#define V_GENFOIL_CONFIG_H

/**
 * @file config.h
 * @brief Definición de constantes de configuración globales para el proyecto vgenfoil.
 * @details Este archivo contiene rutas de directorios, nombres de archivos de registro
 *          y otras constantes globales calculadas en tiempo de compilación.
 */

#include <string_view>

/**
 * @namespace Config
 * @brief Espacio de nombres que agrupa todas las variables de configuración del sistema.
 */
namespace Config {

    /**
     * @brief Ruta del directorio donde se guardarán los archivos de los perfiles aerodinámicos generados.
     * @note Asegúrate de que el directorio exista o que el programa tenga permisos para crearlo.
     */
    constexpr std::string_view AIRFOIL_OUTPUT_DIR = "output/airfoils/";

    /**
     * @brief Ruta completa y nombre del archivo de registro (log) de la aplicación.
     */
    constexpr std::string_view LOG_FILE = "output/vgenfoil.log";
}

#endif //V_GENFOIL_CONFIG_H
