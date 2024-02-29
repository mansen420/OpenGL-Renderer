#pragma once
#include "glm/glm.hpp"
namespace renderer
{
    namespace camera
    {
        extern glm::vec3     POS;
        extern glm::vec3      UP;
        extern glm::vec3 LOOK_AT;
        //call this every frame. or maybe on every change to camera state?
        void update_camera();
        //call once, before update_camera.
        void          init();
    }
}