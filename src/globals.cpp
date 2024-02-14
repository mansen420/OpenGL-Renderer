#include "global_constants.h"
int OPENGL_VIEWPORT_X = 0;
int OPENGL_VIEWPORT_Y = 0;

int OPENGL_VIEWPORT_H = 720;
int OPENGL_VIEWPORT_W = aspect_ratio*OPENGL_VIEWPORT_H;

int WINDOW_W = 1.2*OPENGL_VIEWPORT_W;
int WINDOW_H = OPENGL_VIEWPORT_H;

GLFWwindow* myWindow = nullptr;