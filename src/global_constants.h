#pragma once
//TODO solve problem where some headers include glad after glfw 
#include "GLFW/glfw3.h"

constexpr unsigned int MAX_RENDER_W = GL_MAX_FRAMEBUFFER_WIDTH;
constexpr unsigned int MAX_RENDER_H = GL_MAX_FRAMEBUFFER_HEIGHT;

constexpr float aspect_ratio = 16.0/9.0;

extern int OPENGL_VIEWPORT_H;
extern int OPENGL_VIEWPORT_W;

extern int OPENGL_VIEWPORT_X;
extern int OPENGL_VIEWPORT_Y;

extern int WINDOW_H;
extern int WINDOW_W;

extern GLFWwindow* myWindow;