#pragma once
#include "GLFW/glfw3.h"
constexpr float aspect_ratio = 16.0/9.0;

extern int OPENGL_VIEWPORT_H;
extern int OPENGL_VIEWPORT_W;

extern int OPENGL_VIEWPORT_X;
extern int OPENGL_VIEWPORT_Y;

extern int WINDOW_H;
extern int WINDOW_W;

extern GLFWwindow* myWindow;    

//event stuff
extern bool should_import;
extern bool should_quit;