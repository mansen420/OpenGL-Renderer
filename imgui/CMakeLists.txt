add_library(
    imgui STATIC
    imgui.cpp imgui_tables.cpp imgui_widgets.cpp imgui_demo.cpp
    imgui_draw.cpp
    backends/imgui_impl_opengl3.cpp backends/imgui_impl_glfw.cpp)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED true)
target_compile_options(my_renderer PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-O3") #TODO have compiler flags be global

target_include_directories(imgui PUBLIC "." "backends/" "../glfw/include")