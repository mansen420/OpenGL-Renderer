cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER "/bin/gcc") #TODO this should be autotamic

project(my_renderer VERSION 0.1 LANGUAGES CXX C)

add_executable(my_renderer src/main.cpp src/callbacks.cpp src/event_handling.cpp src/glad.c src/globals.cpp
    src/gui.cpp src/renderer.cpp src/window.cpp 
    )
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED true)
target_compile_options(my_renderer PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-O3") #TODO have different build configurations


set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw)
target_link_libraries(my_renderer glfw)
add_subdirectory(imgui)
target_link_libraries(my_renderer imgui)
target_include_directories( my_renderer PUBLIC "glfw/include/" "include/" "imgui/")