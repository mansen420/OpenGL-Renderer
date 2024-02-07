cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER "/bin/gcc")

project(my_renderer VERSION 0.1 LANGUAGES CXX C)


add_executable(my_renderer src/main.cpp src/callbacks.cpp src/event_handling.cpp src/glad.c src/globals.cpp
    src/gui.cpp src/renderer.cpp src/window.cpp 
    imgui/imgui.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/backends/imgui_impl_opengl3.cpp imgui/backends/imgui_impl_glfw.cpp)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED true)


set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw)
target_link_libraries(my_renderer glfw)
target_include_directories( my_renderer PUBLIC "glfw/include/" "include/" "imgui/")

