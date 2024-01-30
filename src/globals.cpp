#include "global_constants.h"
int OPENGL_VIEWPORT_X = 0;
int OPENGL_VIEWPORT_Y = 0;

int WINDOW_H = 1.05*OPENGL_VIEWPORT_H;
int WINDOW_W = 1.2*OPENGL_VIEWPORT_W;

GLFWwindow* myWindow = nullptr;

//event stuff
bool should_import = false;
bool should_quit = false;