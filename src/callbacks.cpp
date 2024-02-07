#include "callbacks.h"
#include "global_constants.h"
#include "renderer.h"
void frame_buffer_resize_callback(GLFWwindow* window, int width, int height)
{
    WINDOW_H = height; WINDOW_W = width;
    OPENGL_VIEWPORT_H = WINDOW_H;
    renderer::update_screen_tex_coords();   
}
void register_GLFW_callbacks()
{
    glfwSetFramebufferSizeCallback(myWindow, frame_buffer_resize_callback);
}