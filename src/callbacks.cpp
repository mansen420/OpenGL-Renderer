#include "callbacks.h"
void frame_buffer_resize_callback(GLFWwindow* window, int width, int height)
{
    //TODO play with this
    //glViewport(OPENGL_VIEWPORT_X, OPENGL_VIEWPORT_Y-(WINDOW_H-height), OPENGL_VIEWPORT_W, OPENGL_VIEWPORT_H);
    //OPENGL_VIEWPORT_Y = OPENGL_VIEWPORT_Y+(WINDOW_H-height);
    WINDOW_H = height; WINDOW_W = width;
}
void register_GLFW_callbacks()
{
    glfwSetFramebufferSizeCallback(myWindow, frame_buffer_resize_callback);
}