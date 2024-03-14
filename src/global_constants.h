#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"
constexpr const char* SHADER_DIR_PATH           =          "shaders/";
constexpr const char* UNROLLED_SHADER_DIR_PATH  = "unrolled_shaders/";
constexpr const char* SHADER_HEADER_DIR_PATH    =  "shaders/headers/";
constexpr const char* SHADER_LIBRARY_DIR_PATH   =  "shaders/library/";

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