#include "input_handling.h"
#include "engine_interface.h"
#include "GLFW/glfw3.h"

#include <iostream>


void window::process_input()
{   
    if(glfwGetKey(myWindow, GLFW_KEY_W) == GLFW_PRESS)
        renderer::camera::CAMERA_PARAMS.ACC_PHI   = -0.1f;
        
    if(glfwGetKey(myWindow, GLFW_KEY_S) == GLFW_PRESS)
        renderer::camera::CAMERA_PARAMS.ACC_PHI   =  0.1f;

    if(glfwGetKey(myWindow, GLFW_KEY_A) == GLFW_PRESS)
        renderer::camera::CAMERA_PARAMS.ACC_THETA = -0.1f;

    if(glfwGetKey(myWindow, GLFW_KEY_D) == GLFW_PRESS)
        renderer::camera::CAMERA_PARAMS.ACC_THETA =  0.1f;
}