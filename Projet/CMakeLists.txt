add_executable(AirportLosSantos
    main.cpp
)

# <-- ICI : on ajoute ton dossier hpp pour que #include fonctionne
target_include_directories(AirportLosSantos PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Projet/hpp
)

