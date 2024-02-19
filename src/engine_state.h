#pragma once

#include "glad/glad.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "global_constants.h" 
#include <string> 

namespace renderer
{
    enum shader_prg_options
    {
        POST_PROCESS,
        OFF_SCREEN
    };
    //FIXME this enum conflits with the one declared at shader_utils.h.
    //This is a tragedy waiting to happen. fix it. 
    enum shader_options
    {
        VERTEX,
        FRAGMENT
    };
    enum scr_display_mode_option
    {
        COLOR,
        DEPTH,
        STENCIL
    };
    enum renderport_behaviour
    {
        FIT_TO_VIEW,
        CROP
    };
    enum texture_filtering
    {
        LINEAR                 =                 GL_LINEAR,
        NEAREST                =                GL_NEAREST,
        MIPMAP_LINEAR_LINEAR   =   GL_LINEAR_MIPMAP_LINEAR,
        MIPMAP_NEAREST_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
        MIPMAP_LINEAR_NEAREST  =  GL_LINEAR_MIPMAP_NEAREST,
        MIPMAP_NEAREST_LINEAR  =  GL_NEAREST_MIPMAP_LINEAR
    };


    namespace settings 
    {
        extern std::string PATH_TO_OBJ;
        extern bool DEPTH_CLR_ENBLD, COLOR_CLR_ENBLD, STENCIL_CLR_ENBLD;
        extern bool DEPTH_TEST_ENBLD, STENCIL_TEST_ENBLD;
        extern glm::vec4 CLR_COLOR;
        extern glm::vec3 DEPTH_VIEW_COLOR;
        extern const unsigned int *actv_obj_shdr_prg_ID, *actv_pp_shdr_prg_ID;
        extern scr_display_mode_option DISPLAY_BUFFER;
        extern renderport_behaviour RENDER_TO_VIEW_MODE;
        extern texture_filtering SCR_TEX_MAG_FLTR, SCR_TEX_MIN_FLTR;
        extern bool USE_MIPMAPS;
        extern bool PP_ENBLD;
        extern size_t RENDER_W, RENDER_H;
        extern float RENDER_AR;
        extern float NEAR_PLANE, FAR_PLANE, FOV;
        extern glm::vec2   RENDER_VIEW_POS;
        extern float     SCR_TEX_MAX_RATIO;
        extern float     SCR_TEX_MIN_RATIO;
    }
}