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
    
    struct engine_state_t
    {
        std::string PATH_TO_OBJ;

        bool DEPTH_CLR_ENBLD, COLOR_CLR_ENBLD, STENCIL_CLR_ENBLD;
        bool                DEPTH_TEST_ENBLD, STENCIL_TEST_ENBLD;

        glm::vec4        CLR_COLOR;
        glm::vec3 DEPTH_VIEW_COLOR;

        scr_display_mode_option               DISPLAY_BUFFER;
        renderport_behaviour             RENDER_TO_VIEW_MODE;
        texture_filtering SCR_TEX_MAG_FLTR, SCR_TEX_MIN_FLTR;
        bool                                     USE_MIPMAPS;

        bool PP_ENBLD;

        size_t RENDER_W, RENDER_H;
        float  RENDER_AR;

        float NEAR_PLANE, FAR_PLANE, FOV;

        glm::vec2   RENDER_VIEW_POS;
        float     SCR_TEX_MAX_RATIO;
        float     SCR_TEX_MIN_RATIO;
        engine_state_t()
        { 
            PATH_TO_OBJ = "assets/cube.obj"; 

            CLR_COLOR        = glm::vec4(0.6, 0.3, 0.3, 1.0);
            DEPTH_VIEW_COLOR = glm::vec3(1.0, 1.0, 1.0);

            DEPTH_CLR_ENBLD  = 1, COLOR_CLR_ENBLD    = 1, STENCIL_CLR_ENBLD = 1;
            DEPTH_TEST_ENBLD = 1, STENCIL_TEST_ENBLD = 1;

            DISPLAY_BUFFER   = COLOR;
            RENDER_TO_VIEW_MODE =  CROP;

            PP_ENBLD    = 1;

            RENDER_W   = 1920, RENDER_H = 1080;
            RENDER_AR  = 16.0/9.0;
            NEAR_PLANE = 0.1f, FAR_PLANE = 100.0f;
            FOV        = 45.0;

            USE_MIPMAPS = false;
            SCR_TEX_MAG_FLTR = LINEAR, SCR_TEX_MIN_FLTR = LINEAR;

            RENDER_VIEW_POS   = glm::vec2(0.5f, 0.5f);
            SCR_TEX_MAX_RATIO =     1.0;
            SCR_TEX_MIN_RATIO =     0.0; 
        }
    };
    extern engine_state_t ENGINE_SETTINGS;

    //FIXME this should go into its own interface header
    //we also need to ensure the code will NOT be modified (i.e. should be const)
    char* get_source(shader_prg_options prg_type, shader_options shader_type);
    //FIXME! temporary function
    bool update_shader(char* source, shader_options shader_type, shader_prg_options prg_type);

    int init();
    void terminate();
    void render_scene();
    void update_state();
}