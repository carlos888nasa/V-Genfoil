cmake_minimum_required(VERSION 3.20)
project(V-Genfoil VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Buscar Eigen3 en el sistema
find_package(Eigen3 REQUIRED)

include_directories(src)

add_executable(V-Genfoil
        src/main.cpp
        src/geometry/parsec.cpp
        src/geometry/parsec.cpp
        src/geometry/parsec.cpp
        src/utils/config.h
)

# Enlazar Eigen3 a nuestro target
target_link_libraries(V-Genfoil PRIVATE Eigen3::Eigen)

target_compile_options(V-Genfoil PRIVATE -Wall -Wextra -Wpedantic)