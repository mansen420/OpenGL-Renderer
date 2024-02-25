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

//TODO find a way to synchronize camera movement. preferable synchronize it to system time.
void renderer::camera::init()
{
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
    internal_state = renderer::camera::CAMERA_PARAMS;
    
    internal_state.RESISTANCE_FACTOR = internal_state.RESISTANCE_FACTOR < 0 ? 0.0f : internal_state.RESISTANCE_FACTOR;
    internal_state.MAX_SPEED = internal_state.MAX_SPEED < 0.0f ? 0.0f : internal_state.MAX_SPEED;
    //calculate pos
    acceleration.x = internal_state.ACC_THETA;
    acceleration.y =   internal_state.ACC_PHI;

    velocity += acceleration;

    velocity = glm::length(velocity) > internal_state.MAX_SPEED ? 
    internal_state.MAX_SPEED * glm::normalize(velocity) : velocity;

    resistance = glm::length(velocity) <= internal_state.RESISTANCE_FACTOR ? -velocity 
    : internal_state.RESISTANCE_FACTOR * - glm::normalize(velocity);
    velocity += resistance;

    internal_state.THETA += velocity.x;
    internal_state.PHI   += velocity.y;

    internal_state.PHI = internal_state.PHI >  89.9 ?  89.9 : internal_state.PHI;
    internal_state.PHI = internal_state.PHI < -89.9 ? -89.9 : internal_state.PHI;

    //TODO for some reason the order of matrix multiplication is significant here. Why?
    renderer::camera::POS = (
    glm::rotate(glm::mat4(1.0), glm::radians(internal_state.THETA), glm::vec3(0.f, 1.f, 0.f)) 
    * glm::rotate(glm::mat4(1.0), glm::radians(internal_state.PHI), glm::vec3(1.f, 0.f, 0.f)) 
    * glm::vec4(0.f, 0.f, internal_state.DIST, 1.f));
    
    //clear acceleration
    internal_state.ACC_THETA = internal_state.DEFAULT_ACC_THETA;
    internal_state.ACC_PHI   =   internal_state.DEFAULT_ACC_PHI;
    //confirm state
    renderer::camera::CAMERA_PARAMS = internal_state;
}