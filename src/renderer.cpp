#include "engine_interface.h"
#include <algorithm>        //for std::clamp
#include <map>
//TODO fix these
#include "shader_utils.h"
#include "object_interface.h"
#include <fstream>
//TODO add error logging for all opengl calls
namespace renderer
{
     
    static std::ofstream eng_log;

           engine_state_t ENGINE_SETTINGS;    //public state 
    static engine_state_t  internal_state;   //actual state of the engine

    //DO NOT initialize global non-POD variables in the global namespace. YOU HAVE BEEN WARNED!!  
    static shader_manager::shader_prg_t* postprocess_shader_program_ptr;
    static shader_manager::shader_prg_t*      object_shader_program_ptr;

    static std::map<unsigned int, shader_manager::shader_t*> shader_map;

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

    //on-screen texture data
    static float scr_tex_top_edge   = 1.0, scr_tex_bottom_edge = 0.0;
    static float scr_tex_right_edge = 1.0, scr_tex_left_edge   = 0.0;
    //parameters
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
    void update_import();

    void send_uniforms()
    {
        using namespace glm;
        glm::vec3 cam_pos = glm::vec3(0.0, 0.0, 3.0);
        view_transform = lookAt(cam_pos, vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));

        glUniform3f(glGetUniformLocation(object_shader_program_ptr->get_ID(), "view_vector"), cam_pos.x, cam_pos.y, cam_pos.z);
        glUniformMatrix4fv(glGetUniformLocation(object_shader_program_ptr->get_ID(), "projection_transform"), 1, GL_FALSE,
        glm::value_ptr(perspective_transform));
        glUniformMatrix4fv(glGetUniformLocation(object_shader_program_ptr->get_ID(), "view_transform"), 1, GL_FALSE,
        glm::value_ptr(view_transform));
    }
    void offscreen_pass()
    {
        glViewport(OPENGL_VIEWPORT_X, OPENGL_VIEWPORT_Y, internal_state.RENDER_W, internal_state.RENDER_H);

        glBindFramebuffer(GL_FRAMEBUFFER, offscreen_framebuffer_ID);

        internal_state.DEPTH_TEST_ENBLD ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT*internal_state.COLOR_CLR_ENBLD | GL_DEPTH_BUFFER_BIT*internal_state.DEPTH_CLR_ENBLD | GL_STENCIL_BUFFER_BIT*internal_state.STENCIL_CLR_ENBLD);

        glUseProgram(object_shader_program_ptr->get_ID());

        model_transform = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.f, 1.f, 0.f));
        send_uniforms();
        obj_ptr->model_transform = model_transform;

        obj_ptr->draw(object_shader_program_ptr->get_ID());
    }
    void postprocess_pass()
    {
        if (internal_state.SHOW_REAL_RENDER_SIZE)
            glViewport(OPENGL_VIEWPORT_X, OPENGL_VIEWPORT_Y, internal_state.RENDER_W, internal_state.RENDER_H);
        else
            glViewport(OPENGL_VIEWPORT_X, OPENGL_VIEWPORT_Y, OPENGL_VIEWPORT_W, OPENGL_VIEWPORT_H);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        //if we do not disable the depth test here, the screen will draw over itself
        glDisable(GL_DEPTH_TEST);   
        glUseProgram(postprocess_shader_program_ptr->get_ID());

        glBindVertexArray(screen_quad_vao_ID);

        //TODO implement viewing depth and stencil textures (1/2)
        glActiveTexture(GL_TEXTURE0); 
        if(internal_state.DISPLAY_BUFFER == DEPTH)
        {
            glBindTexture(GL_TEXTURE_2D, offscr_tex_IDs[DEPTH_STENCIL_TEX_IDX]);
            glUniform1i(glGetUniformLocation(postprocess_shader_program_ptr->get_ID(), "rendering_depth"), GL_TRUE);
            glUniform3f(glGetUniformLocation(postprocess_shader_program_ptr->get_ID(), "depth_view_color"), 
            internal_state.DEPTH_VIEW_COLOR.r, internal_state.DEPTH_VIEW_COLOR.g, internal_state.DEPTH_VIEW_COLOR.b);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, offscr_tex_IDs[COLOR_TEX_IDX]);
            glUniform1i(glGetUniformLocation(postprocess_shader_program_ptr->get_ID(), "rendering_depth"), GL_FALSE);
        }

        glUniform1i(glGetUniformLocation(postprocess_shader_program_ptr->get_ID(), "screen_texture"), 0);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    void render_scene()
    {
        glClearColor(internal_state.CLR_COLOR.r, internal_state.CLR_COLOR.g, internal_state.CLR_COLOR.b, internal_state.CLR_COLOR.a);
        offscreen_pass();
        postprocess_pass();
    }
    
    void update_offscreen_tex_params()
    {
        glBindTexture(GL_TEXTURE_2D, offscr_tex_IDs[COLOR_TEX_IDX]);
        //TODO check that the magnification filter is not set to use mipmaps
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, internal_state.SCR_TEX_MIN_FLTR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, internal_state.SCR_TEX_MAG_FLTR);

        glBindTexture(GL_TEXTURE_2D, offscr_tex_IDs[DEPTH_STENCIL_TEX_IDX]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, internal_state.SCR_TEX_MIN_FLTR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, internal_state.SCR_TEX_MAG_FLTR);

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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, internal_state.SCR_TEX_MIN_FLTR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, internal_state.SCR_TEX_MAG_FLTR);

        //depth+stencil for maximum portability. glTexStorage2D is necessary to view the depth buffer
        glBindTexture(GL_TEXTURE_2D, offscr_tex_IDs[DEPTH_STENCIL_TEX_IDX]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, rendering_width, rendering_height); 
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, internal_state.SCR_TEX_MIN_FLTR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, internal_state.SCR_TEX_MAG_FLTR);

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
        perspective_transform = perspective(radians(internal_state.FOV), internal_state.RENDER_AR, internal_state.NEAR_PLANE, internal_state.FAR_PLANE);
    }
    //chef's kiss. works perfectly! love this function!
    //DO NOT touch this function. ever.
    void update_screen_tex_coords()
    {   
        if (internal_state.SCR_TEX_MAX_RATIO > 1.0 || internal_state.SCR_TEX_MAX_RATIO < internal_state.SCR_TEX_MIN_RATIO || internal_state.SCR_TEX_MIN_RATIO < 0)
        {
            //TODO throw error
            std::cout << "BAD TEX RATIOS"<<std::endl;
            return;
        } 
     
        scr_tex_top_edge   =  scr_tex_right_edge  = internal_state.SCR_TEX_MAX_RATIO;
        scr_tex_left_edge  =  scr_tex_bottom_edge = internal_state.SCR_TEX_MIN_RATIO;
        if(internal_state.RENDER_TO_VIEW_MODE == CROP)
        {
            float render_aspect_ratio   = float(internal_state.RENDER_W)/internal_state.RENDER_H;
            float viewport_aspect_ratio =   float(OPENGL_VIEWPORT_W)/OPENGL_VIEWPORT_H;
            float ratio                 =    render_aspect_ratio/viewport_aspect_ratio;

            ratio > 1.0 ? scr_tex_right_edge = std::clamp(1/ratio, internal_state.SCR_TEX_MIN_RATIO, internal_state.SCR_TEX_MAX_RATIO)
            :             scr_tex_top_edge   = std::clamp(ratio  , internal_state.SCR_TEX_MIN_RATIO, internal_state.SCR_TEX_MAX_RATIO);

            //TODO 2 modes for viewport positioning : decide center then claculate shift vector,
            //     or decide shift vector arbitrarily.
            glm::vec2 view_center ((scr_tex_right_edge-scr_tex_left_edge)/2.0, (scr_tex_top_edge-scr_tex_bottom_edge)/2.0);
            const glm::vec2& render_center = internal_state.RENDER_VIEW_POS;  

            glm::vec2 shift_vec = render_center - view_center;

            scr_tex_right_edge  = std::min(shift_vec.x +  scr_tex_right_edge, internal_state.SCR_TEX_MAX_RATIO);
            scr_tex_left_edge   = std::max(shift_vec.x +   scr_tex_left_edge, internal_state.SCR_TEX_MIN_RATIO);
            scr_tex_top_edge    = std::min(shift_vec.y +    scr_tex_top_edge, internal_state.SCR_TEX_MAX_RATIO);
            scr_tex_bottom_edge = std::max(shift_vec.y + scr_tex_bottom_edge, internal_state.SCR_TEX_MIN_RATIO);
        }
        
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
        
        //update_projection();
        setup_offscreen_framebuffer(internal_state.RENDER_W, internal_state.RENDER_H);
        send_screen_coords();
    }
    void update_state()
    {
        bool should_update_import, should_update_scr_tex_coords, should_update_offscr_tex_params, should_update_projection;

        should_update_import = internal_state.PATH_TO_OBJ != ENGINE_SETTINGS.PATH_TO_OBJ;
        
        should_update_scr_tex_coords =  //yeah, that's a lot!
            internal_state.RENDER_H            != ENGINE_SETTINGS.RENDER_H          || 
            internal_state.RENDER_W            != ENGINE_SETTINGS.RENDER_W          ||
            internal_state.RENDER_VIEW_POS     != ENGINE_SETTINGS.RENDER_VIEW_POS   || 
            internal_state.SCR_TEX_MAX_RATIO   != ENGINE_SETTINGS.SCR_TEX_MAX_RATIO ||
            internal_state.SCR_TEX_MIN_RATIO   != ENGINE_SETTINGS.SCR_TEX_MIN_RATIO ||
            internal_state.RENDER_TO_VIEW_MODE != ENGINE_SETTINGS.RENDER_TO_VIEW_MODE;

        should_update_offscr_tex_params = 
            internal_state.SCR_TEX_MAG_FLTR != ENGINE_SETTINGS.SCR_TEX_MAG_FLTR ||
            internal_state.SCR_TEX_MIN_FLTR != ENGINE_SETTINGS.SCR_TEX_MIN_FLTR;

        should_update_projection = 
            internal_state.RENDER_AR  != ENGINE_SETTINGS.RENDER_AR  ||
            internal_state.FOV        != ENGINE_SETTINGS.FOV        ||
            internal_state.NEAR_PLANE != ENGINE_SETTINGS.NEAR_PLANE ||
            internal_state.FAR_PLANE  != ENGINE_SETTINGS.FAR_PLANE;

        internal_state = ENGINE_SETTINGS;

        if(should_update_import)
            update_import();
        if (should_update_offscr_tex_params)
            update_offscreen_tex_params();
        if (should_update_scr_tex_coords)
            update_screen_tex_coords();
        if (should_update_projection)
            update_projection();
    }
        
    int init()
    {
        bool shader_success = true;
        //TODO stop hardcoding shader paths

        static shader_manager::shader_t obj_vert_shader(VERTEX_SHADER);
        shader_success &= obj_vert_shader.load_source_from_path("src/shaders/gooch.vs");
        static shader_manager::shader_t obj_frag_shader(FRAGMENT_SHADER);
        shader_success &= obj_frag_shader.load_source_from_path("src/shaders/gooch.fs");

        shader_success &= obj_frag_shader.compile() && obj_vert_shader.compile();

        static shader_manager::shader_prg_t object_shader_program;
        object_shader_program.attach_shader(obj_frag_shader);
        object_shader_program.attach_shader(obj_vert_shader);
        shader_success &= object_shader_program.link();

        object_shader_program_ptr = &object_shader_program;

        static shader_manager::shader_t pp_vert_shader(VERTEX_SHADER);
        shader_success &= pp_vert_shader.load_source_from_path("src/shaders/screen_PP.vs");
        static shader_manager::shader_t pp_frag_shader(FRAGMENT_SHADER);
        shader_success &= pp_frag_shader.load_source_from_path("src/shaders/screen_PP.fs");

        shader_success &= pp_frag_shader.compile() && pp_vert_shader.compile();

        static shader_manager::shader_prg_t posprocess_shader_program;
        posprocess_shader_program.attach_shader(pp_frag_shader);
        posprocess_shader_program.attach_shader(pp_vert_shader);
        shader_success &= posprocess_shader_program.link();
        
        postprocess_shader_program_ptr = &posprocess_shader_program;

        shader_map.insert(std::pair<unsigned int, shader_manager::shader_t*>(obj_vert_shader.get_ID(), &obj_vert_shader));
        shader_map.insert(std::pair<unsigned int, shader_manager::shader_t*>(obj_frag_shader.get_ID(), &obj_frag_shader));

        shader_map.insert(std::pair<unsigned int, shader_manager::shader_t*>(pp_vert_shader.get_ID(), &pp_vert_shader));
        shader_map.insert(std::pair<unsigned int, shader_manager::shader_t*>(pp_frag_shader.get_ID(), &pp_frag_shader));

        if(!shader_success)
        {
            std::cout << "FAILED TO INITIALIZE SHADER PROGRAMS. TERMINATING." << std::endl;
            return false;
        }

        eng_log.open("engine_log.txt", std::ofstream::out | std::ofstream::trunc);

        read_obj(internal_state.PATH_TO_OBJ, *obj_ptr);
        obj_ptr->send_data();

        if (!setup_offscreen_framebuffer(internal_state.RENDER_W, internal_state.RENDER_H))
            return false;
        update_screen_tex_coords();       

        update_projection();
        return true;
    }
    void update_import()
    {
        //TODO write virutal destructor 
        delete obj_ptr;
        obj_ptr = new object_3D::object;
        read_obj(internal_state.PATH_TO_OBJ, *obj_ptr);
        obj_ptr->send_data(); 
    }
    void terminate()
    {
        eng_log.close();        
        delete obj_ptr;
        delete[] scr_quad;
    }
}