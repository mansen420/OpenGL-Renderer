#pragma once 
//CALLBACKS 
#include "glad/glad.h"  //include glad.h before glfw3.h
#include "GLFW/glfw3.h"
#include "global_constants.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

namespace window 
{
    void frame_buffer_resize_callback(GLFWwindow* window, int width, int height)
    {
        WINDOW_H = height; WINDOW_W = width;
    }
    inline void register_GLFW_callbacks()
    {
        glfwSetFramebufferSizeCallback(myWindow, frame_buffer_resize_callback);
    }
}