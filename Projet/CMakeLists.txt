cmake_minimum_required (VERSION 3.20)

add_executable (TestAirport "main.cpp" "ATC.cpp" "ATC.hpp")

target_link_libraries(TestAirport SFML::Graphics SFML::Window SFML::System)

if(WIN32)
    add_custom_command(TARGET TestAirport POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/lib"
        "$<TARGET_FILE_DIR:TestAirport>/lib"
    )
endif()