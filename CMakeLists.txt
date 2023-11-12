cmake_minimum_required(VERSION 3.26)
project(3DGame C)

set(CMAKE_C_STANDARD 11)

add_executable(3DGame
        main.c
        main_3dgame.c
        main_3dgame.h
        glm.h)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

target_link_libraries(3DGame "-framework OpenGL")
target_link_libraries(3DGame "-framework GLUT")

