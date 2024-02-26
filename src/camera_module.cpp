#include "engine_interface.h"
#include "camera_module.h"

glm::vec3 renderer::camera::POS;
glm::vec3 renderer::camera::UP;
glm::vec3 renderer::camera::LOOK_AT;


static glm::vec2     velocity;
static glm::vec2 acceleration;
static glm::vec2   resistance;

       renderer::camera::camera_parameter_t renderer::camera::CAMERA_PARAMS;
static renderer::camera::camera_parameter_t internal_state;

static double previous_time;

//TODO find a way to synchronize camera movement. preferable synchronize it to system time.
void renderer::camera::init()
{
    previous_time = glfwGetTime();

    internal_state = renderer::camera::camera_parameter_t();

    renderer::camera::POS     = glm::vec3(0.f, 0.f, internal_state.DIST);
    renderer::camera::UP      = glm::vec3(0.f, 1.f, 0.f);
    renderer::camera::LOOK_AT = glm::vec3(0.f);

    velocity     = glm::vec3(0.f);
    acceleration = glm::vec3(0.f);
    resistance   = glm::vec3(0.f);
}
void renderer::camera::update_camera()
{
    double time_interval = glfwGetTime() - previous_time;
    previous_time = glfwGetTime();

    internal_state = renderer::camera::CAMERA_PARAMS;

    //*****************calculate pos*****************
    {
        internal_state.RESISTANCE_FACTOR = internal_state.RESISTANCE_FACTOR < 0 ? 0.0f : internal_state.RESISTANCE_FACTOR;
        internal_state.MAX_SPEED = internal_state.MAX_SPEED < 0.0f ? 0.0f : internal_state.MAX_SPEED;

        acceleration.x = internal_state.ACC_THETA;
        acceleration.y =   internal_state.ACC_PHI;

        glm::vec2 previous_velocity = velocity;

        velocity += acceleration;

        float speed = glm::length(velocity);

        velocity = speed > internal_state.MAX_SPEED ? 
        internal_state.MAX_SPEED * glm::normalize(velocity) : velocity;

        resistance = speed <= 0.f  || velocity == glm::vec2(0.f) ? glm::vec2(0.f) 
        : float(time_interval) * internal_state.RESISTANCE_FACTOR * - glm::normalize(velocity);

        //TODO hypothetical problem: frame rate is so low, that the following addition turns velocity in the opposite direction
        if (glm::dot(velocity+resistance, velocity) > 0)
            velocity += resistance;
        else
            velocity = glm::vec2(0.f);
        
        internal_state.THETA += time_interval * (velocity.x + previous_velocity.x)/2.f;
        internal_state.PHI   += time_interval * (velocity.y + previous_velocity.y)/2.f;

       // internal_state.PHI = internal_state.PHI >  89.9 ?  89.9 : internal_state.PHI;
       // internal_state.PHI = internal_state.PHI < -89.9 ? -89.9 : internal_state.PHI;

        //TODO for some reason the order of matrix multiplication is significant here. Why?
        renderer::camera::POS = (
        glm::rotate(glm::mat4(1.0), glm::radians(internal_state.THETA), glm::vec3(0.f, 1.f, 0.f)) 
        * glm::rotate(glm::mat4(1.0), glm::radians(internal_state.PHI), glm::vec3(1.f, 0.f, 0.f)) 
        * glm::vec4(0.f, 0.f, internal_state.DIST, 1.f));
    }
    if (internal_state.CLEAR_ACC)
    {
        internal_state.ACC_THETA = internal_state.DEFAULT_ACC_THETA;
        internal_state.ACC_PHI   =   internal_state.DEFAULT_ACC_PHI;
    }
    if (internal_state.CLEAR_VELOCITY)
    {
        velocity.x = internal_state.DEFAULT_VELOCITY_THETA;
        velocity.y =   internal_state.DEFAULT_VELOCITY_PHI;       
    }
    //confirm state
    renderer::camera::CAMERA_PARAMS = internal_state;
}