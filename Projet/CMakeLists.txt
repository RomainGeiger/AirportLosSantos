find_package(SFML 3 COMPONENTS Graphics Window System Audio REQUIRED)

set(SOURCES
    main.cpp 
    ATC.cpp 
    ATC.hpp
)

add_executable(AirportLosSantosV2 ${SOURCES})

target_link_libraries(AirportLosSantosV2 
    SFML::Graphics 
    SFML::Window 
    SFML::System 
    SFML::Audio
)

if(WIN32)
    add_custom_command(TARGET AirportLosSantosV2 POST_BUILD
        # 1. Copie des DLLs SFML
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SFML::Graphics> $<TARGET_FILE_DIR:AirportLosSantosV2>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SFML::Window>   $<TARGET_FILE_DIR:AirportLosSantosV2>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SFML::System>   $<TARGET_FILE_DIR:AirportLosSantosV2>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SFML::Audio>    $<TARGET_FILE_DIR:AirportLosSantosV2>

        # 2. --- AJOUT ICI : COPIE DE LA POLICE ---
        # On prend arial.ttf dans le dossier source et on le met à côté de l'exe
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/arial.ttf"
        $<TARGET_FILE_DIR:AirportLosSantosV2>
    )
endif()