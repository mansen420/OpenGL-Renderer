cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER "/bin/gcc") #TODO this should be autotamic

project(my_renderer VERSION 0.1 LANGUAGES CXX C)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED true)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(src)
add_subdirectory(glfw)
add_subdirectory(imgui)