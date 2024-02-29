add_executable(my_renderer 
    main.cpp callbacks.cpp event_handling.cpp globals.cpp
    gui.cpp renderer.cpp window.cpp stb_image.cpp tiny_obj_loader.cpp glad.c
    object_interface.cpp shader_utils.cpp input_handling.cpp camera_module.cpp
    )   
target_compile_options(my_renderer PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-O3") #TODO have different build configurations

target_link_libraries(my_renderer glfw)
target_link_libraries(my_renderer imgui)

target_include_directories(my_renderer PUBLIC "glfw/include/" "imgui/")
