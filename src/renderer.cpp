#include "renderer.h"
//TODO fix these
#include "shader_utils.h"
#include "object_interface.h"
namespace renderer
{   
    //renderer configuration
    namespace settings
    {
        glm::vec4 CLR_COLOR(0.6, 0.3, 0.3, 1.0);
        glm::vec3 DEPTH_VIEW_COLOR(1.0, 1.0, 1.0);

        bool DEPTH_CLR_ENBLD = 1, COLOR_CLR_ENBLD = 1, STENCIL_CLR_ENBLD = 1;
        bool DEPTH_TEST_ENBLD = 1, STENCIL_TEST_ENBLD = 1;

        const unsigned int *active_object_shader, *active_pp_shader;

        scr_display_mode_option scr_display_mode = COLOR;
        renderport_behaviour rndrprt_behaviour = CONSTANT_ASPECT_RATIO;

        bool PP_ENBLD = 1;
        size_t RENDER_W = 1920, RENDER_H = 1080;
        float RENDER_AR = 16.0/9.0;
        float near_plane = 0.1f, far_plane = 100.0f;
        float fov = 45.0;

        bool use_mipmaps = false;
        texture_filtering scr_tex_mag_filter = LINEAR, scr_tex_min_filter = LINEAR;
    }
    using namespace settings;

    static unsigned int       postprocess_shader;
    static unsigned int    default_object_shader;
    static unsigned int     offscreen_tex_ids[2];  // [0] = color, [1] = depth+stencil
    static unsigned int               depth_view;
    static unsigned int offscreen_framebuffer_id;
    static unsigned int               screen_vao;
    static unsigned int               screen_vbo;

    glm::mat4 model_transform;
    glm::mat4 view_transform;
    glm::mat4 perspective_transform;

    object_3D::object* my_object = new object_3D::object;
    std::string path_to_object = "assets/cube.obj"; //default

    //on-screen texture data
    static float SCR_TEX_TOP   = 1.0, SCR_TEX_BOTTOM = 0.0;
    static float SCR_TEX_RIGHT = 1.0, SCR_TEX_LEFT   = 0.0;
    //parameters
              float SCR_TEX_MAX_RATIO   =  1.0;
              float SCR_TEX_MIN_RATIO   =  0.0;
    constexpr float LEFT_EDGE           = -1.0;
    constexpr float RIGHT_EDGE          =  1.0;
    constexpr float BOTTOM_EDGE         = -1.0;
    constexpr float TOP_EDGE            =  1.0;

    static float*   SCR_COORDS  = new float[24] 
    {
        // positions           // texCoords
        LEFT_EDGE ,  TOP_EDGE   ,  SCR_TEX_LEFT ,    SCR_TEX_TOP, 
        LEFT_EDGE ,  BOTTOM_EDGE,  SCR_TEX_LEFT , SCR_TEX_BOTTOM,
        TOP_EDGE  ,  BOTTOM_EDGE,  SCR_TEX_RIGHT, SCR_TEX_BOTTOM, 

        LEFT_EDGE ,  TOP_EDGE   ,  SCR_TEX_LEFT ,    SCR_TEX_TOP, 
        RIGHT_EDGE,  TOP_EDGE   ,  SCR_TEX_RIGHT,    SCR_TEX_TOP, 
        RIGHT_EDGE,  BOTTOM_EDGE,  SCR_TEX_RIGHT, SCR_TEX_BOTTOM
    };
    
    void update_projection();

    void send_uniforms()
    {
        using namespace glm;
        view_transform = lookAt(vec3(0.f, 0.f, 3.f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));

        glUniformMatrix4fv(glGetUniformLocation(*active_object_shader, "projection_transform"), 1, GL_FALSE,
        glm::value_ptr(perspective_transform));
        glUniformMatrix4fv(glGetUniformLocation(*active_object_shader, "view_transform"), 1, GL_FALSE,
        glm::value_ptr(view_transform));
    }
    void offscreen_pass()
    {
        glViewport(OPENGL_VIEWPORT_X, OPENGL_VIEWPORT_Y, RENDER_W, RENDER_H);
        glBindFramebuffer(GL_FRAMEBUFFER, offscreen_framebuffer_id);

        DEPTH_TEST_ENBLD ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT*COLOR_CLR_ENBLD | GL_DEPTH_BUFFER_BIT*DEPTH_CLR_ENBLD | GL_STENCIL_BUFFER_BIT*STENCIL_CLR_ENBLD);

        glUseProgram(*active_object_shader);

        model_transform = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), 
        glm::vec3(0.f, 1.f, 0.f));
        send_uniforms();
        my_object->model_transform = model_transform;

        my_object->draw(*active_object_shader);
    }
    void main_pass()
    {
        glViewport(OPENGL_VIEWPORT_X, OPENGL_VIEWPORT_Y, OPENGL_VIEWPORT_W, OPENGL_VIEWPORT_H);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        //if we do not disable the depth test here, the screen will draw over itself
        glDisable(GL_DEPTH_TEST);   
        glUseProgram(postprocess_shader);

        glBindVertexArray(screen_vao);

        //TODO implement viewing depth and stencil textures (1/2)
        glActiveTexture(GL_TEXTURE0); 
        if(scr_display_mode == DEPTH)
        {
            glBindTexture(GL_TEXTURE_2D, offscreen_tex_ids[1]);
            glUniform1i(glGetUniformLocation(*active_pp_shader, "rendering_depth"), GL_TRUE);
            glUniform3f(glGetUniformLocation(*active_pp_shader, "depth_view_color"), DEPTH_VIEW_COLOR.r, DEPTH_VIEW_COLOR.g, DEPTH_VIEW_COLOR.b);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, offscreen_tex_ids[0]);
            glUniform1i(glGetUniformLocation(*active_pp_shader, "rendering_depth"), GL_FALSE);
        }
        glUniform1i(glGetUniformLocation(postprocess_shader, "screen_texture"), 0);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    void render_scene()
    {
        glClearColor(CLR_COLOR.r, CLR_COLOR.g, CLR_COLOR.b, CLR_COLOR.a);
        offscreen_pass();
        main_pass();
        update_projection(); //TODO maybe handle this elsewhere
    }
    void update_offscreen_tex_params()
    {
        glBindTexture(GL_TEXTURE_2D, offscreen_tex_ids[0]);
        //TODO check that the magnification filter is not set to use mipmaps
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, scr_tex_min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, scr_tex_mag_filter);

        glBindTexture(GL_TEXTURE_2D, offscreen_tex_ids[1]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, scr_tex_min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, scr_tex_mag_filter);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
    bool setup_offscreen_framebuffer(const size_t rendering_width, const size_t rendering_height)
    {
        if (glIsFramebuffer(offscreen_framebuffer_id) == GL_TRUE)
        {
            glDeleteFramebuffers(1, &offscreen_framebuffer_id);
        }
        glGenFramebuffers(1, &offscreen_framebuffer_id);
        glBindFramebuffer(GL_FRAMEBUFFER, offscreen_framebuffer_id);

        if(glIsTexture(offscreen_tex_ids[0]) == GL_TRUE)
        {
            glDeleteTextures(2, offscreen_tex_ids);
        }
        glGenTextures(2, offscreen_tex_ids);

        //color attachment
        glBindTexture(GL_TEXTURE_2D, offscreen_tex_ids[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rendering_width, rendering_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glGenerateMipmap(GL_TEXTURE_2D);

        //TODO check that the magnification filter is not set to use mipmaps
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, scr_tex_min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, scr_tex_mag_filter);

        //depth+stencil for maximum portability. glTexStorage2D is necessary to view the depth buffer
        glBindTexture(GL_TEXTURE_2D, offscreen_tex_ids[1]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, rendering_width, rendering_height); 
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, scr_tex_min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, scr_tex_mag_filter);

        //attaching a texture that is bound might cause undefined behaviour
        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, offscreen_tex_ids[0], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, offscreen_tex_ids[1], 0);

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
        if (glIsBuffer(screen_vbo))
            glDeleteBuffers(1, &screen_vbo);
        glGenBuffers(1, &screen_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, screen_vbo);
        //TODO maybe hardcoding the 24 floats in is not the best solution
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*24, SCR_COORDS, GL_STATIC_DRAW);
        
        if (glIsVertexArray(screen_vao))
            glDeleteVertexArrays(1, &screen_vao);
        glGenVertexArrays(1, &screen_vao);

        glBindVertexArray(screen_vao);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)(sizeof(float)*2));
        glEnableVertexAttribArray(1);

        //unbind your buffer ONLY after calling the attribute pointer, you risk severe consequences otherwise!
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
    }
    void update_projection()
    {
        //TODO
        // setting the aspect to RENDER_W/RENDER_H causes the image to stop shearing at high width,
        // setting it to OPENGL_W/OPENGL_H causes the image to "fit" to the viewport
        // I decided to set it to a user-specified value intead, and completely separate it from
        // window or render dimensions. This gives the best behaviour
        using namespace glm;
        perspective_transform = perspective(radians(fov), RENDER_AR, near_plane, far_plane);
    }
    //chef's kiss. works perfectly! love this function!
    //DO NOT touch this function. ever.
    void update_screen_tex_coords()
    {    
        if (SCR_TEX_MAX_RATIO > 1.0 || SCR_TEX_MAX_RATIO < SCR_TEX_MIN_RATIO || SCR_TEX_MIN_RATIO < 0)
        {
            //TODO throw error
            std::cout << "BAD TEX RATIOS"<<std::endl;
            return;
        } 
        
        float render_aspect_ratio   =                   float(RENDER_W)/RENDER_H;
        float viewport_aspect_ratio = float(OPENGL_VIEWPORT_W)/OPENGL_VIEWPORT_H;
        float ratio                 =  render_aspect_ratio/viewport_aspect_ratio;

        //reset coords 
        SCR_TEX_TOP   =  SCR_TEX_RIGHT  = SCR_TEX_MAX_RATIO;
        SCR_TEX_LEFT  =  SCR_TEX_BOTTOM = SCR_TEX_MIN_RATIO;

        //determine renderport cut-out of render target
        ratio > 1.0 ? SCR_TEX_RIGHT = std::min(std::max(1.0f/ratio, SCR_TEX_MIN_RATIO), SCR_TEX_MAX_RATIO) 
                    : SCR_TEX_TOP   = std::max(std::min(   ratio,   SCR_TEX_MAX_RATIO), SCR_TEX_MIN_RATIO);

        //TODO 2 modes for viewport positioning : decide center then claculate shift vector, or decide shift vector arbitrarily.
        //shift viewport into appropriate location
        glm::vec2 viewport_center ((SCR_TEX_RIGHT-SCR_TEX_LEFT)/2.0, (SCR_TEX_TOP-SCR_TEX_BOTTOM)/2.0);
        glm::vec2 render_center   (0.5, 0.5);       

        glm::vec2 shift_vec = render_center - viewport_center;

        SCR_TEX_RIGHT   = shift_vec.x + SCR_TEX_RIGHT    > SCR_TEX_MAX_RATIO ? SCR_TEX_MAX_RATIO : shift_vec.x +  SCR_TEX_RIGHT;
        SCR_TEX_LEFT    = shift_vec.x + SCR_TEX_LEFT     < SCR_TEX_MIN_RATIO ? SCR_TEX_MIN_RATIO : shift_vec.x +   SCR_TEX_LEFT;
        SCR_TEX_TOP     = shift_vec.y + SCR_TEX_TOP      > SCR_TEX_MAX_RATIO ? SCR_TEX_MAX_RATIO : shift_vec.y +    SCR_TEX_TOP;
        SCR_TEX_BOTTOM  = shift_vec.y + SCR_TEX_BOTTOM   < SCR_TEX_MIN_RATIO ? SCR_TEX_MIN_RATIO : shift_vec.y + SCR_TEX_BOTTOM;
        
        std::cout << SCR_TEX_TOP       << ' ' << SCR_TEX_BOTTOM << '\t' << SCR_TEX_RIGHT << ' ' << SCR_TEX_LEFT << std::endl;
        std::cout << RENDER_W          << ' ' << RENDER_H <<std::endl;
        std::cout << OPENGL_VIEWPORT_W << ' ' << OPENGL_VIEWPORT_H <<std::endl;
        std::cout << "*\t*" <<std::endl; 

        //free old memory, alloc new memory. Maybe better to simply modify the old memory?
        delete[] SCR_COORDS; 
        SCR_COORDS = new float[24] 
        {
            // positions               // texCoords
            LEFT_EDGE ,  TOP_EDGE   ,  SCR_TEX_LEFT ,    SCR_TEX_TOP, 
            LEFT_EDGE ,  BOTTOM_EDGE,  SCR_TEX_LEFT , SCR_TEX_BOTTOM,
            RIGHT_EDGE,  BOTTOM_EDGE,  SCR_TEX_RIGHT, SCR_TEX_BOTTOM, 

            LEFT_EDGE ,  TOP_EDGE   ,  SCR_TEX_LEFT ,    SCR_TEX_TOP, 
            RIGHT_EDGE,  TOP_EDGE   ,  SCR_TEX_RIGHT,    SCR_TEX_TOP, 
            RIGHT_EDGE,  BOTTOM_EDGE,  SCR_TEX_RIGHT, SCR_TEX_BOTTOM
        };

        update_projection();
        setup_offscreen_framebuffer(RENDER_W, RENDER_H);
        send_screen_coords();
    }
    int init()
    {
        read_obj(path_to_object, *my_object);
        my_object->send_data();

        if (!setup_offscreen_framebuffer(RENDER_W, RENDER_H))
            return false;
        update_screen_tex_coords();       

        glClearColor(CLR_COLOR.r, CLR_COLOR.g, CLR_COLOR.b, CLR_COLOR.a);

        makeShaderProgram("src/shaders/default.vs",
        "src/shaders/default.fs", default_object_shader);
        active_object_shader = &default_object_shader;
        makeShaderProgram("src/shaders/screen_PP.vs",
        "src/shaders/screen_PP.fs", postprocess_shader);
        active_pp_shader = &postprocess_shader;

        return true;
    }
    void update_import()
    {
        //TODO write virutal destructor 
        delete my_object;
        my_object = new object_3D::object;
        read_obj(path_to_object, *my_object);
        my_object->send_data(); 
    }
    void terminate()
    {
        delete my_object;
        delete[] SCR_COORDS;
    }
}