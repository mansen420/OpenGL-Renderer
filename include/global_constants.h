#ifndef GLOBAL_CONST
#define GLOBAL_CONST

constexpr float aspect_ratio = 16.0/9.0;
constexpr int OPENGL_VIEWPORT_H = 720;
constexpr int OPENGL_VIEWPORT_W = aspect_ratio * OPENGL_VIEWPORT_H;

int OPENGL_VIEWPORT_X = 0;
int OPENGL_VIEWPORT_Y = 0;

int WINDOW_H = 1.05*OPENGL_VIEWPORT_H;
int WINDOW_W = 1.2*OPENGL_VIEWPORT_W;

GLFWwindow* myWindow;

//event stuff
bool should_import = false;
bool should_quit = false;

#endif