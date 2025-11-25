add_executable(AirportLosSantos
    main.cpp
 "avion.cpp" "world.cpp")

# ====== INCLUSION DU DOSSIER hpp ======
target_include_directories(AirportLosSantos PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/hpp
)