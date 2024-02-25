#include "input_handling.h"
#include "engine_interface.h"
#include "GLFW/glfw3.h"

void window::process_input()
{   
    static float acceleration = 0.1f;
    static float friction_factor = 0.01f;
    float max_speed = 2.0f;

    static glm::vec2     velocity(0.0f);


    velocity = glm::length(velocity) > max_speed ? max_speed * glm::normalize(velocity) 
    : velocity;

    glm::vec2 friction = friction_factor * -velocity;
    velocity += friction;

    renderer::ENGINE_SETTINGS.PHI   += velocity.y;
    renderer::ENGINE_SETTINGS.THETA += velocity.x;

    if(glfwGetKey(myWindow, GLFW_KEY_W))
        velocity.y += acceleration;
    if(glfwGetKey(myWindow, GLFW_KEY_S))
        velocity.y -= acceleration;
    if(glfwGetKey(myWindow, GLFW_KEY_A))
        velocity.x += acceleration;
    if(glfwGetKey(myWindow, GLFW_KEY_D))
        velocity.x -= acceleration;
}