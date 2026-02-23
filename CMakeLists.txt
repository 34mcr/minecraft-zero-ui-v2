cmake_minimum_required(VERSION 3.16)
project(Project C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(include lib)

file(GLOB SOURCES src/*.c)

add_executable(project ${SOURCES})

if(WIN32)
    target_link_libraries(project
        opengl32
        glfw3
        glfw32
        gdi32
    )
endif()

add_custom_command(TARGET project POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:project>/assets
    )