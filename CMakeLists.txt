cmake_minimum_required(VERSION 3.10)
project(CIR2_PROG C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- CONSIGNE DU PDF ---
# On indique le chemin VERS le dossier contenant "SFMLConfig.cmake"
# Si ton dossier est C:\SFML_3.0.2, alors le chemin est :
set(SFML_DIR "${CMAKE_SOURCE_DIR}/External/SFML/lib/cmake/SFML")

add_subdirectory(Projet)