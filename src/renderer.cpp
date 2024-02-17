#include "renderer.h"       //includes callback functions
#include "engine_state.h"   //includes engine state 
#include <algorithm>        //for std::clamp
//TODO fix these
#include "shader_utils.h"
#include "object_interface.h"
//TODO add error logging for all opengl calls
namespace renderer
{   
    namespace settings
    {
        glm::vec4   CLR_COLOR(0.6, 0.3, 0.3, 1.0);
        glm::vec3 DEPTH_VIEW_COLOR(1.0, 1.0, 1.0);

        bool DEPTH_CLR_ENBLD  = 1, COLOR_CLR_ENBLD    = 1, STENCIL_CLR_ENBLD = 1;
        bool DEPTH_TEST_ENBLD = 1, STENCIL_TEST_ENBLD = 1;

        const unsigned int *ACTV_OBJ_SHDR_PRG_ID, *ACTV_PP_SHDR_PRG_ID;

        scr_display_mode_option DISPLAY_BUFFER   = COLOR;
        renderport_behaviour RENDER_TO_VIEW_MODE =  CROP;

        bool PP_ENBLD    = 1;

        size_t RENDER_W  = 1920, RENDER_H = 1080;
        float RENDER_AR  = 16.0/9.0;
        float NEAR_PLANE = 0.1f, FAR_PLANE = 100.0f;
        float FOV        = 45.0;

        bool USE_MIPMAPS = false;
        texture_filtering SCR_TEX_MAG_FLTR = LINEAR, SCR_TEX_MIN_FLTR = LINEAR;
    }

    static unsigned int       postprocess_shader_program_id;
    static unsigned int    default_object_shader_program_id;

    static    unsigned int     offscr_tex_IDs[2];
    constexpr unsigned int     COLOR_TEX_IDX = 0, DEPTH_STENCIL_TEX_IDX = 1;

    constexpr unsigned int   SCR_FRAMEBUFFER_ID = 0;
    static    unsigned int offscreen_framebuffer_ID;
    static    unsigned int       screen_quad_vao_ID;

    //TODO find a better way to handle uniforms
    static glm::mat4       model_transform;
    static glm::mat4        view_transform;
    static glm::mat4 perspective_transform;

    object_3D::object*    obj_ptr     = new object_3D::object;
    std::string settings::PATH_TO_OBJ =     "assets/cube.obj"; //default

    //on-screen texture data
    static float scr_tex_top_edge   = 1.0, scr_tex_bottom_edge = 0.0;
    static float scr_tex_right_edge = 1.0, scr_tex_left_edge   = 0.0;
    //parameters
    glm::vec2 settings::RENDER_VIEW_POS(0.5f, 0.5f);
        float settings::SCR_TEX_MAX_RATIO =     1.0;
        float settings::SCR_TEX_MIN_RATIO =     0.0;
    constexpr float LEFT_EDGE   = -1.0;
    constexpr float RIGHT_EDGE  =  1.0;
    constexpr float BOTTOM_EDGE = -1.0;
    constexpr float TOP_EDGE    =  1.0;

    static float* scr_quad  = new float[24] 
    {
        // positions               // texCoords
        LEFT_EDGE ,  TOP_EDGE   ,  scr_tex_left_edge ,    scr_tex_top_edge, 
        LEFT_EDGE ,  BOTTOM_EDGE,  scr_tex_left_edge , scr_tex_bottom_edge,
        TOP_EDGE  ,  BOTTOM_EDGE,  scr_tex_right_edge, scr_tex_bottom_edge, 

        LEFT_EDGE ,  TOP_EDGE   ,  scr_tex_left_edge ,    scr_tex_top_edge, 
        RIGHT_EDGE,  TOP_EDGE   ,  scr_tex_right_edge,    scr_tex_top_edge, 
        RIGHT_EDGE,  BOTTOM_EDGE,  scr_tex_right_edge, scr_tex_bottom_edge
    };
    
    void update_projection();

    void send_uniforms()
    {
        using namespace glm;
        glm::vec3 cam_pos = glm::vec3(0.0, 0.0, 3.0);
        view_transform = lookAt(cam_pos, vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));

        glUniform3f(glGetUniformLocation(*settings::ACTV_OBJ_SHDR_PRG_ID, "view_vector"), cam_pos.x, cam_pos.y, cam_pos.z);
        glUniformMatrix4fv(glGetUniformLocation(*settings::ACTV_OBJ_SHDR_PRG_ID, "projection_transform"), 1, GL_FALSE,
        glm::value_ptr(perspective_transform));
        glUniformMatrix4fv(glGetUniformLocation(*settings::ACTV_OBJ_SHDR_PRG_ID, "view_transform"), 1, GL_FALSE,
        glm::value_ptr(view_transform));
    }
    void offscreen_pass()
    {
        glViewport(OPENGL_VIEWPORT_X, OPENGL_VIEWPORT_Y, settings::RENDER_W, settings::RENDER_H);

        glBindFramebuffer(GL_FRAMEBUFFER, offscreen_framebuffer_ID);

        settings::DEPTH_TEST_ENBLD ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT*settings::COLOR_CLR_ENBLD | GL_DEPTH_BUFFER_BIT*settings::DEPTH_CLR_ENBLD | GL_STENCIL_BUFFER_BIT*settings::STENCIL_CLR_ENBLD);

        glUseProgram(*settings::ACTV_OBJ_SHDR_PRG_ID);

        model_transform = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.f, 1.f, 0.f));
        send_uniforms();
        obj_ptr->model_transform = model_transform;

        obj_ptr->draw(*settings::ACTV_OBJ_SHDR_PRG_ID);
    }
    void postprocess_pass()
    {
        glViewport(OPENGL_VIEWPORT_X, OPENGL_VIEWPORT_Y, OPENGL_VIEWPORT_W, OPENGL_VIEWPORT_H);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        //if we do not disable the depth test here, the screen will draw over itself
        glDisable(GL_DEPTH_TEST);   
        glUseProgram(postprocess_shader_program_id);

        glBindVertexArray(screen_quad_vao_ID);

        //TODO implement viewing depth and stencil textures (1/2)
        glActiveTexture(GL_TEXTURE0); 
        if(settings::DISPLAY_BUFFER == DEPTH)
        {
            glBindTexture(GL_TEXTURE_2D, offscr_tex_IDs[DEPTH_STENCIL_TEX_IDX]);
            glUniform1i(glGetUniformLocation(*settings::ACTV_PP_SHDR_PRG_ID, "rendering_depth"), GL_TRUE);
            glUniform3f(glGetUniformLocation(*settings::ACTV_PP_SHDR_PRG_ID, "depth_view_color"), 
            settings::DEPTH_VIEW_COLOR.r, settings::DEPTH_VIEW_COLOR.g, settings::DEPTH_VIEW_COLOR.b);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, offscr_tex_IDs[COLOR_TEX_IDX]);
            glUniform1i(glGetUniformLocation(*settings::ACTV_PP_SHDR_PRG_ID, "rendering_depth"), GL_FALSE);
        }

        glUniform1i(glGetUniformLocation(postprocess_shader_program_id, "screen_texture"), 0);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    void render_scene()
    {
        glClearColor(settings::CLR_COLOR.r, settings::CLR_COLOR.g, settings::CLR_COLOR.b, settings::CLR_COLOR.a);
        offscreen_pass();
        postprocess_pass();
        update_projection(); //TODO maybe handle this elsewhere
    }
    void update_offscreen_tex_params()
    {
        glBindTexture(GL_TEXTURE_2D, offscr_tex_IDs[COLOR_TEX_IDX]);
        //TODO check that the magnification filter is not set to use mipmaps
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, settings::SCR_TEX_MIN_FLTR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, settings::SCR_TEX_MAG_FLTR);

        glBindTexture(GL_TEXTURE_2D, offscr_tex_IDs[DEPTH_STENCIL_TEX_IDX]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, settings::SCR_TEX_MIN_FLTR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, settings::SCR_TEX_MAG_FLTR);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
    bool setup_offscreen_framebuffer(const size_t rendering_width, const size_t rendering_height)
    {
        if (glIsFramebuffer(offscreen_framebuffer_ID) == GL_TRUE)
        {
            glDeleteFramebuffers(1, &offscreen_framebuffer_ID);
        }
        glGenFramebuffers(1, &offscreen_framebuffer_ID);
        glBindFramebuffer(GL_FRAMEBUFFER, offscreen_framebuffer_ID);

        if(glIsTexture(offscr_tex_IDs[COLOR_TEX_IDX]) == GL_TRUE)
        {
            glDeleteTextures(2, offscr_tex_IDs);
        }
        glGenTextures(2, offscr_tex_IDs);

        //color attachment
        glBindTexture(GL_TEXTURE_2D, offscr_tex_IDs[COLOR_TEX_IDX]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rendering_width, rendering_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glGenerateMipmap(GL_TEXTURE_2D);

        //TODO check that the magnification filter is not set to use mipmaps
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, settings::SCR_TEX_MIN_FLTR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, settings::SCR_TEX_MAG_FLTR);

        //depth+stencil for maximum portability. glTexStorage2D is necessary to view the depth buffer
        glBindTexture(GL_TEXTURE_2D, offscr_tex_IDs[DEPTH_STENCIL_TEX_IDX]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, rendering_width, rendering_height); 
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, settings::SCR_TEX_MIN_FLTR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, settings::SCR_TEX_MAG_FLTR);

        //attaching a texture that is bound might cause undefined behaviour
        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, offscr_tex_IDs[COLOR_TEX_IDX], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, offscr_tex_IDs[DEPTH_STENCIL_TEX_IDX], 0);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "FRAMEBUFFER INCOMPLETE : "<< glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
            //TODO throw some eror or smth 
            return false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return true;
    }
    void send_screen_coords()
    {
        static unsigned int vbo_ID;

        if (glIsBuffer(vbo_ID))
            glDeleteBuffers(1, &vbo_ID);
            
        glGenBuffers(1, &vbo_ID);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ID);
        //TODO maybe hardcoding the 24 floats in is not the best solution
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*24, scr_quad, GL_STATIC_DRAW);
        
        if (glIsVertexArray(screen_quad_vao_ID))
            glDeleteVertexArrays(1, &screen_quad_vao_ID);

        glGenVertexArrays(1, &screen_quad_vao_ID);

        glBindVertexArray(screen_quad_vao_ID);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)(sizeof(float)*2));
        glEnableVertexAttribArray(2);

        //unbind your buffer ONLY after calling the attribute pointer. you risk severe consequences otherwise!
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
    }
    void update_projection()
    {
        using namespace glm;
        perspective_transform = perspective(radians(settings::FOV), settings::RENDER_AR, settings::NEAR_PLANE, settings::FAR_PLANE);
    }
    //chef's kiss. works perfectly! love this function!
    //DO NOT touch this function. ever.
    void update_screen_tex_coords()
    {   
        if (settings::SCR_TEX_MAX_RATIO > 1.0 || settings::SCR_TEX_MAX_RATIO < settings::SCR_TEX_MIN_RATIO || settings::SCR_TEX_MIN_RATIO < 0)
        {
            //TODO throw error
            std::cout << "BAD TEX RATIOS"<<std::endl;
            return;
        } 
        
        float render_aspect_ratio   = float(settings::RENDER_W)/settings::RENDER_H;
        float viewport_aspect_ratio =   float(OPENGL_VIEWPORT_W)/OPENGL_VIEWPORT_H;
        float ratio                 =    render_aspect_ratio/viewport_aspect_ratio;

        scr_tex_top_edge   =  scr_tex_right_edge  = settings::SCR_TEX_MAX_RATIO;
        scr_tex_left_edge  =  scr_tex_bottom_edge = settings::SCR_TEX_MIN_RATIO;
        if(settings::RENDER_TO_VIEW_MODE == CROP)
        {
            ratio > 1.0 ? scr_tex_right_edge = std::clamp(1/ratio, settings::SCR_TEX_MIN_RATIO, settings::SCR_TEX_MAX_RATIO)
            :             scr_tex_top_edge   = std::clamp(ratio  , settings::SCR_TEX_MIN_RATIO, settings::SCR_TEX_MAX_RATIO);

            //TODO 2 modes for viewport positioning : decide center then claculate shift vector,
            //     or decide shift vector arbitrarily.
            glm::vec2 view_center ((scr_tex_right_edge-scr_tex_left_edge)/2.0, (scr_tex_top_edge-scr_tex_bottom_edge)/2.0);
            const glm::vec2& render_center = settings::RENDER_VIEW_POS;  

            glm::vec2 shift_vec = render_center - view_center;

            scr_tex_right_edge   = shift_vec.x + scr_tex_right_edge    > settings::SCR_TEX_MAX_RATIO ? 
            settings::SCR_TEX_MAX_RATIO : shift_vec.x +  scr_tex_right_edge;
            scr_tex_left_edge    = shift_vec.x + scr_tex_left_edge     < settings::SCR_TEX_MIN_RATIO ? 
            settings::SCR_TEX_MIN_RATIO : shift_vec.x +   scr_tex_left_edge;
            scr_tex_top_edge     = shift_vec.y + scr_tex_top_edge      > settings::SCR_TEX_MAX_RATIO ? 
            settings::SCR_TEX_MAX_RATIO : shift_vec.y +    scr_tex_top_edge;
            scr_tex_bottom_edge  = shift_vec.y + scr_tex_bottom_edge   < settings::SCR_TEX_MIN_RATIO ? 
            settings::SCR_TEX_MIN_RATIO : shift_vec.y + scr_tex_bottom_edge;
        }
        std::cout << scr_tex_top_edge       << ' ' << scr_tex_bottom_edge << '\t' << scr_tex_right_edge << ' ' << scr_tex_left_edge << std::endl;
        std::cout << settings::RENDER_W     << ' ' << settings::RENDER_H <<std::endl;
        std::cout << OPENGL_VIEWPORT_W << ' ' << OPENGL_VIEWPORT_H <<std::endl;
        std::cout << "*\t*" <<std::endl; 

        //free old memory, alloc new memory. Maybe better to simply modify the old memory?
        delete[] scr_quad; 
        scr_quad = new float[24] 
        {
            // positions               // texCoords
            LEFT_EDGE ,  TOP_EDGE   ,  scr_tex_left_edge ,    scr_tex_top_edge, 
            LEFT_EDGE ,  BOTTOM_EDGE,  scr_tex_left_edge , scr_tex_bottom_edge,
            RIGHT_EDGE,  BOTTOM_EDGE,  scr_tex_right_edge, scr_tex_bottom_edge, 

            LEFT_EDGE ,  TOP_EDGE   ,  scr_tex_left_edge ,    scr_tex_top_edge, 
            RIGHT_EDGE,  TOP_EDGE   ,  scr_tex_right_edge,    scr_tex_top_edge, 
            RIGHT_EDGE,  BOTTOM_EDGE,  scr_tex_right_edge, scr_tex_bottom_edge
        };
        
        update_projection();
        setup_offscreen_framebuffer(settings::RENDER_W, settings::RENDER_H);
        send_screen_coords();
    }
    int init()
    {
        read_obj(settings::PATH_TO_OBJ, *obj_ptr);
        obj_ptr->send_data();

        if (!setup_offscreen_framebuffer(settings::RENDER_W, settings::RENDER_H))
            return false;
        update_screen_tex_coords();       

        makeShaderProgram("src/shaders/gooch.vs",
        "src/shaders/gooch.fs", default_object_shader_program_id);
        settings::ACTV_OBJ_SHDR_PRG_ID = &default_object_shader_program_id;
        makeShaderProgram("src/shaders/screen_PP.vs",
        "src/shaders/screen_PP.fs", postprocess_shader_program_id);
        settings::ACTV_PP_SHDR_PRG_ID = &postprocess_shader_program_id;

        return true;
    }
    void update_import()
    {
        //TODO write virutal destructor 
        delete obj_ptr;
        obj_ptr = new object_3D::object;
        read_obj(settings::PATH_TO_OBJ, *obj_ptr);
        obj_ptr->send_data(); 
    }
    void terminate()
    {
        delete obj_ptr;
        delete[] scr_quad;
    }
}