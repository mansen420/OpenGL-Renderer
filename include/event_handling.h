#pragma once

#include "glad/glad.h"  //include glad.h before glfw3.h
#include "GLFW/glfw3.h"
#include "global_constants.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "callbacks.h"

#include <iostream>
//event handler
namespace events
{
    void import()
    {
        std::cout << "IMPORT!" << std::endl;
    }
    void quit(){glfwSetWindowShouldClose(myWindow, true);}
    inline void poll()
    {
        if(should_import) {should_import = false; import();}
        if (should_quit){should_quit = false; quit();}
    }
}