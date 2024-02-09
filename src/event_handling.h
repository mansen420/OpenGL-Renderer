#pragma once

#include "glad/glad.h"  //include glad.h before glfw3.h
#include "GLFW/glfw3.h"
#include "global_constants.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "callbacks.h"

#include <iostream>
#include "window.h"
//event handler
namespace events
{
    void import();
    void quit();
    void quit_window();
    void react();
}