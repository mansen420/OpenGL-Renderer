#pragma once

#include "glad/glad.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "global_constants.h" 
#include <string> 

namespace renderer
{
    enum shader_prg_option
    {
        POSTPROCESS_SHADER,
        OBJECT_SHADER
    };
    //TODO maybe this should be an intrnal enum?
    enum shader_type_option
    {
        VERTEX_SHADER   = GL_VERTEX_SHADER  ,
        FRAGMENT_SHADER = GL_FRAGMENT_SHADER,
        GEOMETRY_SHADER = GL_GEOMETRY_SHADER
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
        glm::vec3 LIGHT_POS;

        std::string  OBJECT_PATH;
        glm::vec3 OBJ_DIMENSIONS;
        glm::vec3     OBJ_CENTER;

        glm::vec3 OBJ_DISPLACEMENT;
        float     OBJ_SCALE_FACTOR;
        glm::vec3 OBJ_ROTATION;

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
        unsigned int SHADOW_MAP_W, SHADOW_MAP_H;

        float NEAR_PLANE, FAR_PLANE, FOV;

        glm::vec2   RENDER_VIEW_POS;
        bool  SHOW_REAL_RENDER_SIZE;
        float     SCR_TEX_MAX_RATIO;
        float     SCR_TEX_MIN_RATIO;
                
        engine_state_t()
        {
            OBJECT_PATH = "assets/stanford-bunny.obj"; 
            
            LIGHT_POS = glm::vec3(1.0);

            CLR_COLOR        = glm::vec4(0.6, 0.3, 0.3, 1.0);
            DEPTH_VIEW_COLOR = glm::vec3(1.0, 1.0, 1.0);

            DEPTH_CLR_ENBLD  = 1, COLOR_CLR_ENBLD    = 1, STENCIL_CLR_ENBLD = 1;
            DEPTH_TEST_ENBLD = 1, STENCIL_TEST_ENBLD = 1;

            DISPLAY_BUFFER      = COLOR;
            RENDER_TO_VIEW_MODE =  CROP;

            PP_ENBLD = 1;

            RENDER_W   = 1920, RENDER_H = 1080;
            RENDER_AR  = 16.0/9.0;
            SHADOW_MAP_H =    SHADOW_MAP_W = 1024;
            NEAR_PLANE = 0.1f, FAR_PLANE = 100.0f;
            FOV        = 45.0;

            USE_MIPMAPS      = false;
            SCR_TEX_MAG_FLTR = NEAREST, SCR_TEX_MIN_FLTR = LINEAR;

            RENDER_VIEW_POS   = glm::vec2(0.5f, 0.5f);
            SCR_TEX_MAX_RATIO =     1.0;
            SCR_TEX_MIN_RATIO =     0.0; 

            SHOW_REAL_RENDER_SIZE = false;

            OBJ_SCALE_FACTOR =             1.0f;
            OBJ_DISPLACEMENT =   glm::vec3(0.0);
            OBJ_DIMENSIONS   = glm::vec3(-1.0f);
            OBJ_CENTER       =  glm::vec3(0.0f);
            OBJ_ROTATION     =   glm::vec3(0.0);
        }
    };
    extern engine_state_t ENGINE_SETTINGS;

    //TODO should state be manipulated by variables? or functions?
    //if state is manipulated by varaibles, that is more straightforward,
    //and we do not need to provide read-only access to state.
    //if state is manipulated by functions, we do not have to sync internal and external data...
    
    namespace camera
    {
        struct camera_parameter_t
        {   
            //If this is set to true, camera acceleration will be cleared to DEFAULT_ACC_* every frame.
            bool CLEAR_ACC        =  true;
            //If this is set to true, camera velocity will be cleared to DEFAULT_VELOCITY_* every frame.
            bool CLEAR_VELOCITY   = false;
            //If this is set to true, camera velocity and acceleration will be cleared to DEFAULT_VELOCITY_* 
            //and DEFAULT_ACC_* when THETA or PHI is directly manipulated.
            bool CLEAR_ON_OVERRIDE =  true;

            //If CLEAR_ACC is set to true, this value will be written to ACC_THETA every frame.
            float DEFAULT_ACC_THETA = 0.f;
            //If CLEAR_ACC is set to true, this value will be written to ACC_PHI every frame.
            float DEFAULT_ACC_PHI   = 0.f;

            //If CLEAR_VELOCITY is set to true, this value will be written to VELOCITY_THETA every frame.
            float DEFAULT_VELOCITY_THETA = 0.f;
            //If CLEAR_VELOCITY is set to true, this value will be written to VELOCITY_PHI every frame.
            float DEFAULT_VELOCITY_PHI   = 0.f;

            //Angular acceleration with relation to the y-axis. This value is added to the camera's velocity on the next call to update_camera.
            //If CLEAR_ACC is true, this parameter is set to DEFAULT_ACC_THETA every frame.
            float ACC_THETA = 0.f;
            //Angular acceleration with relation to the x-axis. This value is added to the camera's velocity on the next call to update_camera.
            //If CLEAR_ACC is true, this parameter is set to DEFAULT_ACC_PHI every frame.
            float ACC_PHI   = 0.f;

            //Deg/s of resistance. The resistance is always in the direction opposite to the current velocity 
            //of the camera. This controls the magnitude of the resistance vector.
            float RESISTANCE_FACTOR = 700.f;
            //Maximum magnitude of camera velocity in deg/s.
            float MAX_SPEED         = 200.0f ;

            //Angular position, in degrees, with respect to the x-axis. 
            float   PHI = 0.f;
            //Angular position, in degrees, with respect to the y-axis.
            float THETA = 0.f;
            //Radial distance from the origin.
            float  DIST = 3.f;
        };
        extern camera_parameter_t CAMERA_PARAMS;
    }

    size_t object_nr_vertices();
    size_t object_nr_triangles();   
    void calculate_object_dimensions();
    void center_object();
    void rescale_object(float scale);

    //TODO improve shader public interface

    //Returns const internal source data of specified shader.
    const char* get_shader_source_reflection(shader_prg_option program_type, shader_type_option shader_type);
    //Returns modifieble copy of specified shader.
    std::string get_shader_source_copy(shader_prg_option program_type, shader_type_option shader_type);
    //Returns pointer to shader source for direct manipulation. Handling compilation is the responsibility of the caller.
    std::string* get_shader_source_reference(shader_prg_option program_type, shader_type_option shader_type);
    //Attempts to compile specified shader with source. Returns false and reverts to old source upon unsuccessful compilation.
    bool update_shader(shader_prg_option program_type, shader_type_option shader_type, const char* source);
    //Attempts to compile specified shader. Returns false upon unsuccessful compilation.
    bool compile_shader(shader_prg_option program_type, shader_type_option shader_type);
    //Attempts to link program. Returns false upon unsuccessful linkage.
    bool link_program(shader_prg_option program_type);

    //Initializes internal engine state. Call this only once, and only after you call window::init.
    int          init();
    //Frees resources. Call this only once, and only after renderer::init.
    void    terminate();
    //Invokes OpenGL pipeline.
    void render_scene();
    //Updates internal engine state. Call this once per frame, or whenever engine parameters change.
    void update_state();
}