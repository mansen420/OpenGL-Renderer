#include "input_handling.h"
#include "engine_interface.h"
#include "global_constants.h"
#include <iostream>

float window::input_camera_acceleration = 50.f;

void window::process_input()
{   
    if(glfwGetKey(myWindow, GLFW_KEY_W) == GLFW_PRESS)
        renderer::camera::CAMERA_PARAMS.ACC_PHI   = -input_camera_acceleration;
        
    if(glfwGetKey(myWindow, GLFW_KEY_S) == GLFW_PRESS)
        renderer::camera::CAMERA_PARAMS.ACC_PHI   =  input_camera_acceleration;

    if(glfwGetKey(myWindow, GLFW_KEY_A) == GLFW_PRESS)
        renderer::camera::CAMERA_PARAMS.ACC_THETA = -input_camera_acceleration;

    if(glfwGetKey(myWindow, GLFW_KEY_D) == GLFW_PRESS)
        renderer::camera::CAMERA_PARAMS.ACC_THETA =  input_camera_acceleration;
}