#include "engine_interface.h"
#include <algorithm>        //for std::clamp
#include <map>
#include "shader_utils.h"
#include "object_interface.h"
#include "camera_module.h"
#include <fstream>
#include <thread>
#include <filesystem>
#include "read_file.h"
//TODO add error logging for all opengl calls
namespace renderer
{
    /*------------------------------------------------------------------------------*/
    /*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* Global Variables *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
    /*------------------------------------------------------------------------------*/

    static std::ofstream eng_log;

           engine_state_t ENGINE_SETTINGS;    //public state 
    static engine_state_t  internal_state;   //actual state of the engine

    //DO NOT initialize non-POD variables in the global scope. YOU HAVE BEEN WARNED!!  
    static shader_manager::shader_prg_t* postprocess_shader_program_ptr;
    static shader_manager::shader_prg_t*      object_shader_program_ptr;
    static shader_manager::shader_prg_t*  shadow_map_shader_program_ptr;

    static std::map<const unsigned int, shader_manager::shader_t*> shader_map;

    constexpr unsigned int     COLOR_TEX_IDX = 0, DEPTH_STENCIL_TEX_IDX = 1;
    static    unsigned int          offscr_tex_IDs[2];
    static    unsigned int     shadowmap_depth_tex_ID;

    constexpr unsigned int   SCR_FRAMEBUFFER_ID = 0;
    static    unsigned int offscreen_framebuffer_ID;
    static    unsigned int       screen_quad_vao_ID;
    static    unsigned int shadowmap_framebuffer_ID;

    //TODO find a better way to handle these uniforms
    static glm::mat4        view_transform;
    static glm::mat4 perspective_transform;
    static glm::mat4  lightspace_transform;

    class loadable_object 
    {
        public :

        object_3D::object* obj_ptr;
        loadable_object() : obj_ptr(new object_3D::object), obj_loader(nullptr), should_update(false){}
        //Reads object on a separate thread. 
        //Note that this function will not affect the current obj_ptr until update() has been called AND only if the object has been loaded by that point!
        void load(const char* path)
        {
            std::thread obj_thread(&loadable_object::prepare, this, path);
            obj_thread.detach();
        }
        //Swaps current obj_ptr with newly loaded object, if available.
        //Call this after load()
        void update()
        {
            if(!should_update)
                return;

            auto old_ptr = obj_ptr;

            //FIXME calling this in prepare() causes segfault, why? 
            //Calling send_data() in any non-main thread causes segfaults in general...
            obj_loader->send_data();
            obj_ptr = obj_loader;

            delete old_ptr;        //delete old address
            obj_loader = nullptr; //so that delete operations don't affect the current object address
            should_update = false;
        }
        ~loadable_object()
        {
            delete obj_loader;
            delete    obj_ptr;
        }
        
        private:
        bool should_update;
        object_3D::object* obj_loader;
        void prepare(const char* path)
        {
            delete obj_loader;
            obj_loader =  new object_3D::object;
            
            read_obj(std::string(path), *obj_loader);

            should_update = true;
        }
    };
    
    static loadable_object                       my_object;
    static object_3D::object* &obj_ptr = my_object.obj_ptr;
    static object_3D::array_drawable*     ground_plane_ptr;

    //screen quad texture data
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

    /*------------------------------------------------------------------------------*/
    /*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* Public interface *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
    /*------------------------------------------------------------------------------*/

    //TODO implement a system of identifiying programs and shaders for dynamic stuff
    //TODO might be better to stop using std::strings for shader code, not every user may want to include the string header
    //TODO shader could be nullptr
    shader_manager::shader_t* find_shader(shader_prg_option program_type, shader_type_option shader_type)
    {
        //search shader map for shaders attached to program of type shader.
        const shader_manager::shader_prg_t* prg_ptr = nullptr;
        if (program_type == POSTPROCESS_SHADER)
            prg_ptr = postprocess_shader_program_ptr;
        else if (program_type == OBJECT_SHADER)
            prg_ptr = object_shader_program_ptr;
        else //unknown program
            return nullptr;
        const unsigned int* shader_IDs = prg_ptr->get_attached_shader_IDs();
        const size_t IDs_size          =    prg_ptr->num_attached_shaders();

        shader_manager::shader_t* shader = nullptr;
        for (size_t i = 0; i < IDs_size; ++i)
        {
            shader_manager::shader_t* shader_candidate = shader_map.at(shader_IDs[i]);
            if (shader_candidate->get_type() == shader_type)
            {
                shader = shader_candidate;
                break;
            }
        }
        return shader;
    }
    const char* get_shader_source_reflection(shader_prg_option program_type, shader_type_option shader_type)
    {  
        const shader_manager::shader_t* shader = find_shader(program_type, shader_type);
        if(shader == nullptr)
            return nullptr;
        return shader->source_code.c_str();
    }
    std::string get_shader_source_copy(shader_prg_option program_type, shader_type_option shader_type)
    {
        const shader_manager::shader_t* shader = find_shader(program_type, shader_type);
        return shader->source_code;
    }
    std::string* get_shader_source_reference(shader_prg_option program_type, shader_type_option shader_type)
    {
        shader_manager::shader_t* shader = find_shader(program_type, shader_type);
        if(shader == nullptr)
            return nullptr;
        return &shader->source_code;
    }
    bool update_shader(shader_prg_option program_type, shader_type_option shader_type, const char* source)
    {
        shader_manager::shader_t* shader = find_shader(program_type, shader_type);

        const std::string backup = shader->source_code;
        shader->source_code = std::string(source);
        
        if(shader->compile())
            return true;
        else
        {
            shader->source_code = backup;
            shader->compile();
            return false;
        }
    }
    bool unroll_includes(shader_prg_option program_type, shader_type_option shader_type)
    {
        shader_manager::shader_t* shader = find_shader(program_type, shader_type);
        if (nullptr == shader)
        {
            return false;
        }
        shader->unroll_includes();
        return true;
    }
    //TODO it would be better to relink the program upon any attempt to compile any of its attached shaders.
    bool link_program(shader_prg_option program_type)
    {
        shader_manager::shader_prg_t* program;
        if(program_type == OBJECT_SHADER)
            program = object_shader_program_ptr;
        else if(program_type == POSTPROCESS_SHADER)
            program = postprocess_shader_program_ptr;
        else 
            return false;
        return program->link();
    }
    bool load_source(const char* filename, char* &source_holder)
    {   
        if(!readFile(std::string(SHADER_DIR_PATH).append(filename).c_str(), source_holder))
            return false;
        return true;
    }
    
    //TODO make these return size_t 
    void calculate_object_dimensions()
    {
        obj_ptr->calculate_dimensions();
        ENGINE_SETTINGS.OBJ_DIMENSIONS = obj_ptr->dimensions;
        ENGINE_SETTINGS.OBJ_CENTER     = obj_ptr->center;
    }
    void center_object()
    {
        ENGINE_SETTINGS.OBJ_DISPLACEMENT = -obj_ptr->center;
    }
    void rescale_object(float scale)
    {
        float max_dimension = 
        std::max(obj_ptr->dimensions[0],std::max(obj_ptr->dimensions[1], obj_ptr->dimensions[2]));
        ENGINE_SETTINGS.OBJ_SCALE_FACTOR = scale/max_dimension; 
    }
    size_t object_nr_vertices()
    {
        return obj_ptr->nr_vertices();
    }
    size_t object_nr_triangles()
    {
        return obj_ptr->nr_triangles();
    }
    
    /*---------------------------------------------------------------------------------*/
    /*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* internal procedures *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
    /*---------------------------------------------------------------------------------*/
    void update_projection();
    void update_import();

    void send_shadow_map_uniforms()
    {
        using namespace glm;
        glm::mat4 light_view_transform = lookAt(internal_state.LIGHT_POS, glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0));
        glm::mat4 light_perspective_transform = glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.1f, 10.f);

        //glm::mat4 light_perspective_transform = glm::perspective(glm::radians(45.f), 1.f, 0.1f, 100.f);

        glUniformMatrix4fv(glGetUniformLocation(shadow_map_shader_program_ptr->get_ID(), "projection_transform"), 1, GL_FALSE,
        glm::value_ptr(light_perspective_transform));
        glUniformMatrix4fv(glGetUniformLocation(shadow_map_shader_program_ptr->get_ID(), "view_transform"), 1, GL_FALSE,
        glm::value_ptr(light_view_transform));

        lightspace_transform = light_perspective_transform*light_view_transform;
    }
    void shadow_pass()
    {
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, internal_state.SHADOW_MAP_W, internal_state.SHADOW_MAP_H);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_framebuffer_ID);
        glClear(GL_DEPTH_BUFFER_BIT);

        glUseProgram(shadow_map_shader_program_ptr->get_ID());
        send_shadow_map_uniforms();

        obj_ptr->draw(shadow_map_shader_program_ptr->get_ID());
        ground_plane_ptr->draw(shadow_map_shader_program_ptr->get_ID());
    }
    void send_offscr_uniforms()
    {
        using namespace glm;
        view_transform = lookAt(camera::POS, camera::LOOK_AT, camera::UP);

        glUniform3f(glGetUniformLocation(object_shader_program_ptr->get_ID(), "light_pos"), 
        internal_state.LIGHT_POS.x, internal_state.LIGHT_POS.y, internal_state.LIGHT_POS.z);

        glUniform3f(glGetUniformLocation(object_shader_program_ptr->get_ID(), "view_vector"), camera::POS.x, camera::POS.y, camera::POS.z);
        glUniformMatrix4fv(glGetUniformLocation(object_shader_program_ptr->get_ID(), "projection_transform"), 1, GL_FALSE,
        glm::value_ptr(perspective_transform));
        glUniformMatrix4fv(glGetUniformLocation(object_shader_program_ptr->get_ID(), "view_transform"), 1, GL_FALSE,
        glm::value_ptr(view_transform));
        glUniformMatrix4fv(glGetUniformLocation(object_shader_program_ptr->get_ID(), "lightspace_transform"), 1, GL_FALSE,
        glm::value_ptr(lightspace_transform));
    }
    void offscreen_pass()
    {
        glViewport(OPENGL_VIEWPORT_X, OPENGL_VIEWPORT_Y, internal_state.RENDER_W, internal_state.RENDER_H);

        glBindFramebuffer(GL_FRAMEBUFFER, offscreen_framebuffer_ID);

        internal_state.DEPTH_TEST_ENBLD ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT*internal_state.COLOR_CLR_ENBLD | GL_DEPTH_BUFFER_BIT*internal_state.DEPTH_CLR_ENBLD | GL_STENCIL_BUFFER_BIT*internal_state.STENCIL_CLR_ENBLD);

        glUseProgram(object_shader_program_ptr->get_ID());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shadowmap_depth_tex_ID);

        send_offscr_uniforms();

        obj_ptr->draw(object_shader_program_ptr->get_ID());
        ground_plane_ptr->draw(object_shader_program_ptr->get_ID());
    }
    void postprocess_pass()
    {
        if (internal_state.SHOW_REAL_RENDER_SIZE)
            glViewport(OPENGL_VIEWPORT_X, OPENGL_VIEWPORT_Y, internal_state.RENDER_W, internal_state.RENDER_H);
        else
            glViewport(OPENGL_VIEWPORT_X, OPENGL_VIEWPORT_Y, OPENGL_VIEWPORT_W, OPENGL_VIEWPORT_H);
        glBindFramebuffer(GL_FRAMEBUFFER, SCR_FRAMEBUFFER_ID);
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
            //glBindTexture(GL_TEXTURE_2D, shadowmap_depth_tex_ID);
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
        shadow_pass();
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
    bool setup_shadowmap_framebuffer(const unsigned int resolution_w, const unsigned int resolution_h)
    {
        if (glIsFramebuffer(shadowmap_framebuffer_ID) == GL_TRUE)
        {
            glDeleteFramebuffers(1, &shadowmap_framebuffer_ID);
        }
        glGenFramebuffers(1, &shadowmap_framebuffer_ID);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_framebuffer_ID);

        if(glIsTexture(shadowmap_depth_tex_ID) == GL_TRUE)
        {
            glDeleteTextures(1, &shadowmap_depth_tex_ID);
        }
        glGenTextures(1, &shadowmap_depth_tex_ID);

        glBindTexture(GL_TEXTURE_2D, shadowmap_depth_tex_ID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, resolution_w, resolution_h, 0, GL_DEPTH_COMPONENT,
        GL_FLOAT, NULL);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, internal_state.SCR_TEX_MIN_FLTR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, internal_state.SCR_TEX_MAG_FLTR);

        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowmap_depth_tex_ID, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "SHADOW MAP FRAMEBUFFER INCOMPLETE : "<< glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
            return false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, SCR_FRAMEBUFFER_ID);
        return true;
    }
    bool setup_offscreen_framebuffer(const size_t rendering_width, const size_t rendering_height)
    {
        if (glIsFramebuffer(offscreen_framebuffer_ID) == GL_TRUE)
        {
            glDeleteFramebuffers(1, &offscreen_framebuffer_ID);
        }
        glGenFramebuffers(1, &offscreen_framebuffer_ID);
        glBindFramebuffer(GL_FRAMEBUFFER, offscreen_framebuffer_ID);

        //TODO maybe we don't need to delete the old texture?
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
            std::cout << "OFF-SCREEN FRAMEBUFFER INCOMPLETE : "<< glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
            return false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, SCR_FRAMEBUFFER_ID);
        return true;
    }
    void send_screen_coords()
    {
        static unsigned int vbo_ID;

        if (glIsBuffer(vbo_ID))
            glDeleteBuffers(1, &vbo_ID);
            
        glGenBuffers(1, &vbo_ID);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ID);
        //TODO 24 magic number, other magic nubmers 
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
    void update_screen_tex_coords()
    {   
        if (internal_state.SCR_TEX_MAX_RATIO > 1.0 || internal_state.SCR_TEX_MAX_RATIO < internal_state.SCR_TEX_MIN_RATIO || internal_state.SCR_TEX_MIN_RATIO < 0)
        {
            //TODO make this function return a boolean value for this failure
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
    void update_object_model_transform()
    {
        if (obj_ptr == nullptr)
            return;
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0), glm::radians(internal_state.OBJ_ROTATION.x), 
        glm::vec3(1.0, 0.0, 0.0));
        rotation = glm::rotate(rotation, glm::radians(internal_state.OBJ_ROTATION.y),
        glm::vec3(0.0, 1.0, 0.0));
        rotation = glm::rotate(rotation, glm::radians(internal_state.OBJ_ROTATION.z),
        glm::vec3(0.0, 0.0, 1.0));
        obj_ptr->model_transform = glm::translate(glm::mat4(1.0), internal_state.OBJ_DISPLACEMENT) * rotation
        * glm::scale(glm::mat4(1.0), glm::vec3(internal_state.OBJ_SCALE_FACTOR));
    }
    void update_shadow_map_resolution()
    {
        setup_shadowmap_framebuffer(internal_state.SHADOW_MAP_W, internal_state.SHADOW_MAP_H);
    }
    void update_state()
    {   //TODO make this more sophisticated?
        camera::update_camera();
        my_object.update();

        bool should_update_import, should_update_scr_tex_coords, should_update_offscr_tex_params,
        should_update_projection, should_update_obj_mdl_trnsfrm, should_update_shadow_map_resolution;

        should_update_import = 
            internal_state.OBJECT_PATH != ENGINE_SETTINGS.OBJECT_PATH;
        
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

        should_update_obj_mdl_trnsfrm = 
            internal_state.OBJ_SCALE_FACTOR != ENGINE_SETTINGS.OBJ_SCALE_FACTOR||
            internal_state.OBJ_DISPLACEMENT != ENGINE_SETTINGS.OBJ_DISPLACEMENT||
            internal_state.OBJ_ROTATION     != ENGINE_SETTINGS.OBJ_ROTATION;
        
        should_update_shadow_map_resolution = 
            internal_state.SHADOW_MAP_H != ENGINE_SETTINGS.SHADOW_MAP_H||
            internal_state.SHADOW_MAP_W != ENGINE_SETTINGS.SHADOW_MAP_W;

        internal_state = ENGINE_SETTINGS;

        if(should_update_import)
            update_import();
        if (should_update_offscr_tex_params)
            update_offscreen_tex_params();
        if (should_update_scr_tex_coords)
            update_screen_tex_coords();
        if (should_update_projection)
            update_projection();
        if(should_update_obj_mdl_trnsfrm)
            update_object_model_transform();
        if(should_update_shadow_map_resolution)
            update_shadow_map_resolution();

        ENGINE_SETTINGS = internal_state;
    }
    void setup_ground_plane()
    {
        //TODO parameterize this 
        float vertices[]
        {
            -1.0, -0.05,  1.0,     0.0, 1.0, 0.0,      0.0, 0.0,  //bottom left
             1.0, -0.05,  1.0,     0.0, 1.0, 0.0,      1.0, 0.0,  //bottom right
            -1.0, -0.05, -1.0,     0.0, 1.0, 0.0,      0.0, 1.0,  //top left

             1.0, -0.05, -1.0,     0.0, 1.0, 0.0,      1.0, 1.0,  //top right
             1.0, -0.05,  1.0,     0.0, 1.0, 0.0,      1.0, 0.0,  //botom right
            -1.0, -0.05, -1.0,     0.0, 1.0, 0.0,      0.0, 0.0,  //top left
        };
        static object_3D::array_drawable ground_plane(vertices, sizeof(vertices));
        ground_plane.send_data();
        ground_plane_ptr = &ground_plane;
        ground_plane.model_transform = glm::scale(glm::mat4(1.0), glm::vec3(100.f, 1.f, 100.f));
    }
    
    //only call this function after initializing the program pointers!
    bool attach_shader_library()
    {
        for(auto entry : std::filesystem::recursive_directory_iterator(SHADER_LIBRARY_DIR_PATH))
        {
            //FIXME hardcoding this can't be good
            if(entry.path().extension() == ".fs")
            {
                shader_manager::shader_t shader(FRAGMENT_SHADER);
                if(!shader.load_source_from_path(entry.path().filename().c_str()))
                    return false;
                if (!shader.compile())
                    return false;
                object_shader_program_ptr->attach_shader(shader);
                postprocess_shader_program_ptr->attach_shader(shader);
                shadow_map_shader_program_ptr->attach_shader(shader);
            }
            //TODO add the other extensions 
        }
        return true;
    }
    int init()
    {
        //TODO  use fs::current_dir to do some useful stuff
        my_object.load(internal_state.OBJECT_PATH.c_str());
        camera::init();
        {
            //for the love of god make this a function or something man  
            bool shader_success = true;
            
            static shader_manager::shader_t obj_vert_shader(VERTEX_SHADER);
            shader_success &= obj_vert_shader.load_source_from_path("gooch.vs");
            static shader_manager::shader_t obj_frag_shader(FRAGMENT_SHADER);
            shader_success &= obj_frag_shader.load_source_from_path("gooch.fs");

            shader_success &= obj_frag_shader.compile() && obj_vert_shader.compile();

            static shader_manager::shader_prg_t object_shader_program;
            object_shader_program.attach_shader(obj_frag_shader);
            object_shader_program.attach_shader(obj_vert_shader);

            object_shader_program_ptr = &object_shader_program;
            
            static shader_manager::shader_t pp_vert_shader(VERTEX_SHADER);
            shader_success &= pp_vert_shader.load_source_from_path("screen_PP.vs");
            static shader_manager::shader_t pp_frag_shader(FRAGMENT_SHADER);
            shader_success &= pp_frag_shader.load_source_from_path("screen_PP.fs");

            shader_success &= pp_frag_shader.compile() && pp_vert_shader.compile();

            static shader_manager::shader_prg_t posprocess_shader_program;
            posprocess_shader_program.attach_shader(pp_frag_shader);
            posprocess_shader_program.attach_shader(pp_vert_shader);

            postprocess_shader_program_ptr = &posprocess_shader_program;


            static shader_manager::shader_t shadow_map_vert_shader(VERTEX_SHADER);
            shader_success &= shadow_map_vert_shader.load_source_from_path("shadow_map.vs");
            static shader_manager::shader_t shadow_map_frag_shader(FRAGMENT_SHADER);
            shader_success &= shadow_map_frag_shader.load_source_from_path("shadow_map.fs");

            shader_success &= shadow_map_frag_shader.compile() && shadow_map_vert_shader.compile();

            static shader_manager::shader_prg_t shadow_map_shader_program;
            shadow_map_shader_program.attach_shader(shadow_map_vert_shader);
            shadow_map_shader_program.attach_shader(shadow_map_frag_shader);

            shadow_map_shader_program_ptr = &shadow_map_shader_program;      

            if(!attach_shader_library())
            {
                std::cout << "Failed to compile shader library." << std::endl;
                return false;
            }
            //TODO ensure all dynamically generated shader programs (if any) are linked against the library
            //dont forget to attach the library before linking
            shader_success &= object_shader_program_ptr->link();
            shader_success &= postprocess_shader_program_ptr->link();
            shader_success &= shadow_map_shader_program_ptr->link();


            shader_map.insert(std::pair<const unsigned int, shader_manager::shader_t*>(shadow_map_vert_shader.get_ID(), &shadow_map_vert_shader));
            shader_map.insert(std::pair<const unsigned int, shader_manager::shader_t*>(shadow_map_frag_shader.get_ID(), &shadow_map_frag_shader));
            
            shader_map.insert(std::pair<const unsigned int, shader_manager::shader_t*>(obj_vert_shader.get_ID(), &obj_vert_shader));
            shader_map.insert(std::pair<const unsigned int, shader_manager::shader_t*>(obj_frag_shader.get_ID(), &obj_frag_shader));

            shader_map.insert(std::pair<const unsigned int, shader_manager::shader_t*>(pp_vert_shader.get_ID(), &pp_vert_shader));
            shader_map.insert(std::pair<const unsigned int, shader_manager::shader_t*>(pp_frag_shader.get_ID(), &pp_frag_shader));

            if(!shader_success)
            {
                std::cout << "FAILED TO INITIALIZE SHADER PROGRAMS. TERMINATING." << std::endl;
                return false;
            }
        }
        
        eng_log.open("engine_log.txt", std::ofstream::out | std::ofstream::trunc);

        if (!setup_offscreen_framebuffer(internal_state.RENDER_W, internal_state.RENDER_H))
            return false;

        update_projection();

        if(!setup_shadowmap_framebuffer(internal_state.SHADOW_MAP_W, internal_state.SHADOW_MAP_H))
            return false; //or proceed without shadow map?
         
        setup_ground_plane();
        update_screen_tex_coords();       

        return true;
    }
    void update_import()
    {
        my_object.load(internal_state.OBJECT_PATH.c_str());
    }
    void terminate()
    {
        eng_log.close();        
        delete[] scr_quad;
    }
}