#pragma once

#include "glad/glad.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "global_constants.h"

namespace renderer
{
    enum scr_display_mode
    {
        COLOR,
        DEPTH,
        STENCIL
    };
    enum renderport_behaviour
    {
        FOLLOW_VIEWPORT,
        CONSTANT_ASPECT_RATIO
    };
    enum screen_texture_filtering
    { //TODO fill this
    };
    namespace settings 
    {
        extern bool DEPTH_CLR_ENBLD, COLOR_CLR_ENBLD, STENCIL_CLR_ENBLD;
        extern bool DEPTH_TEST_ENBLD, STENCIL_TEST_ENBLD;
        extern glm::vec4 CLR_COLOR;
        extern const unsigned int *active_object_shader, *active_pp_shader;
        extern scr_display_mode display_mode;
        extern renderport_behaviour rndrprt_behaviour;
        extern bool PP_ENBLD;
        extern unsigned int RENDER_W, RENDER_H;
    }
    void terminate();
    void clear_buffers();
    void render_scene();
    void update_screen_tex_coords();
    int init();
}