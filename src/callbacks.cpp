#include "callbacks.h"
#include "global_constants.h"
#include "renderer.h"
void frame_buffer_resize_callback(GLFWwindow* window, int width, int height)
{
    WINDOW_H = height; WINDOW_W = width;
    //solution : change the texture coordinates of the screen target
    OPENGL_VIEWPORT_H = WINDOW_H;
    glViewport(OPENGL_VIEWPORT_X, OPENGL_VIEWPORT_Y, OPENGL_VIEWPORT_W, OPENGL_VIEWPORT_H);
    renderer::update_screen_tex_coords();
}
void register_GLFW_callbacks()
{
    glfwSetFramebufferSizeCallback(myWindow, frame_buffer_resize_callback);
}