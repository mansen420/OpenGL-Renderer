#pragma once

#include "glad/glad.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "global_constants.h"

namespace renderer
{
    namespace settings 
    {
        extern bool DEPTH_CLR_ENBLD, COLOR_CLR_ENBLD, STENCIL_CLR_ENBLD;
        extern bool DEPTH_TEST_ENBLD, STENCIL_TEST_ENBLD;
        extern glm::vec4 CLR_COLOR;
        extern const unsigned int *active_object_shader, *active_pp_shader;
        extern bool display_object_colorbuffer, display_object_depthbuffer,
        display_object_stencilbuffer;   //only one should be true at any time!
        extern bool PP_ENBLD;
        extern unsigned int RENDER_W, RENDER_H;
    }
    void clear_buffers();
    void render_scene();
    void update_screen_tex_coords();
    int init();
}