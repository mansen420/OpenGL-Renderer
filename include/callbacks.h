#pragma once 
//CALLBACKS 
#include "glad/glad.h"  //include glad.h before glfw3.h
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "global_constants.h"

void frame_buffer_resize_callback(GLFWwindow* window, int width, int height);
void register_GLFW_callbacks();