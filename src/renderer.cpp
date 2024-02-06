#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION ;
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

        bool DEPTH_CLR_ENBLD = 1, COLOR_CLR_ENBLD = 1, STENCIL_CLR_ENBLD = 1;
        bool DEPTH_TEST_ENBLD = 1, STENCIL_TEST_ENBLD = 1;

        const unsigned int *active_object_shader, *active_pp_shader;

        scr_display_mode display_mode = COLOR;
        renderport_behaviour rndrprt_behaviour = CONSTANT_ASPECT_RATIO;

        bool PP_ENBLD = 1;
        unsigned int RENDER_W = 1920, RENDER_H = 1080;
    }
    using namespace settings;

    static unsigned int postprocess_shader;
    static unsigned int default_object_shader;
    static unsigned int offscreen_tex_ids[2];  // [0] = color, [1] = depth+stencil
    static unsigned int offscreen_framebuffer_id;
    static unsigned int screen_vao;
    static unsigned int screen_vbo;

    glm::mat4 model_transform;
    glm::mat4 view_transform;
    glm::mat4 perspective_transform;

    object_3D::object my_object;

    static float SCR_TEX_TOP = 1.0 , SCR_TEX_BOTTOM = 0.0;
    static float SCR_TEX_RIGHT = 1.0, SCR_TEX_LEFT = 0.0;

    static float* screen_coords = new float[] 
    {
        // positions   // texCoords
        -1.0f,  1.0f,  SCR_TEX_LEFT, SCR_TEX_TOP, 
        -1.0f, -1.0f,  SCR_TEX_LEFT, SCR_TEX_BOTTOM,
         1.0f, -1.0f,  SCR_TEX_RIGHT, SCR_TEX_BOTTOM, 

        -1.0f,  1.0f,  SCR_TEX_LEFT, SCR_TEX_TOP, 
         1.0f,  1.0f,  SCR_TEX_RIGHT, SCR_TEX_TOP, 
         1.0f, -1.0f,  SCR_TEX_RIGHT, SCR_TEX_BOTTOM
    };
    
    void send_uniforms()
    {
        glUniformMatrix4fv(glGetUniformLocation(*active_object_shader, "projection_transform"), 1, GL_FALSE,
        glm::value_ptr(perspective_transform));
        glUniformMatrix4fv(glGetUniformLocation(*active_object_shader, "view_transform"), 1, GL_FALSE,
        glm::value_ptr(view_transform));
    }
    void offscreen_pass()
    {
        glViewport(OPENGL_VIEWPORT_X, OPENGL_VIEWPORT_Y, RENDER_W, RENDER_H);
        glBindFramebuffer(GL_FRAMEBUFFER, offscreen_framebuffer_id);
        if (DEPTH_TEST_ENBLD)
            glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT*COLOR_CLR_ENBLD | GL_DEPTH_BUFFER_BIT*DEPTH_CLR_ENBLD);

        glUseProgram(*active_object_shader);

        model_transform = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), 
        glm::vec3(0.f, 1.f, 0.f));
        send_uniforms();
        my_object.model_transform = model_transform;

        my_object.draw(*active_object_shader);
    }
    void main_pass()
    {
        glViewport(OPENGL_VIEWPORT_X, OPENGL_VIEWPORT_Y, OPENGL_VIEWPORT_W, OPENGL_VIEWPORT_H);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT*COLOR_CLR_ENBLD);
        glDisable(GL_DEPTH_TEST);
        glUseProgram(postprocess_shader);

        glBindVertexArray(screen_vao);

        //TODO implement viewing depth and stencil textures
        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, offscreen_tex_ids[0]);
        glUniform1i(glGetUniformLocation(postprocess_shader, "screen_texture"), 0);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    void render_scene()
    {
        offscreen_pass();
        main_pass();
    }
    bool setup_offscreen_framebuffer(const size_t rendering_width, const size_t rendering_height)
    {
        if (glIsFramebuffer(offscreen_framebuffer_id) == GL_TRUE)
        {
            glDeleteFramebuffers(1, &offscreen_framebuffer_id);
        }
        glGenFramebuffers(1, &offscreen_framebuffer_id);
        glBindFramebuffer(GL_FRAMEBUFFER, offscreen_framebuffer_id);

        glGenTextures(2, offscreen_tex_ids);

        //color attachment
        glBindTexture(GL_TEXTURE_2D, offscreen_tex_ids[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rendering_width, rendering_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        //depth+stencil since we need both, and the author's machine only supports the combined format in this case
        glBindTexture(GL_TEXTURE_2D, offscreen_tex_ids[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, rendering_width, rendering_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
        glGenerateMipmap(GL_TEXTURE_2D);

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
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*24, screen_coords, GL_STATIC_DRAW);
        
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
    //chef's kiss. works perfectly! next step is to be able to position the viewport inside the render space...
    void update_screen_tex_coords()
    {
        float render_aspect_ratio = float(RENDER_W)/RENDER_H;
        float viewport_aspect_ratio = float(OPENGL_VIEWPORT_W)/OPENGL_VIEWPORT_H;
        float ratio = render_aspect_ratio/viewport_aspect_ratio;

        //determine renderport cut-out 
        ratio > 1.0 ? SCR_TEX_RIGHT = 1/ratio :  SCR_TEX_TOP = ratio;

        //shift viewport into middle of renderport 
        float remainder = 1.0 - SCR_TEX_RIGHT;
        SCR_TEX_RIGHT += remainder/2;
        SCR_TEX_LEFT = remainder/2;

        remainder = 1.0 - SCR_TEX_TOP;
        SCR_TEX_TOP += remainder/2;
        SCR_TEX_BOTTOM = remainder;

        std::cout << SCR_TEX_TOP << ' ' << SCR_TEX_BOTTOM << '\t' << SCR_TEX_RIGHT << ' ' <<SCR_TEX_LEFT << std::endl;

        //free old memory, alloc new memory. Maybe better to simply modify the old memory?
        delete[] screen_coords; 
        screen_coords = new float[] 
        {
        // positions    // texCoords
        -1.0f,  1.0f,  SCR_TEX_LEFT, SCR_TEX_TOP, 
        -1.0f, -1.0f,  SCR_TEX_LEFT, SCR_TEX_BOTTOM,
         1.0f, -1.0f,  SCR_TEX_RIGHT, SCR_TEX_BOTTOM, 

        -1.0f,  1.0f,  SCR_TEX_LEFT, SCR_TEX_TOP, 
         1.0f,  1.0f,  SCR_TEX_RIGHT, SCR_TEX_TOP, 
         1.0f, -1.0f,  SCR_TEX_RIGHT, SCR_TEX_BOTTOM
        };

        send_screen_coords();
    }
    int init()
    {
        read_obj("assets/backpack/backpack.obj", my_object);
        my_object.send_data();

        if (!setup_offscreen_framebuffer(RENDER_W, RENDER_H))
            return false;
        update_screen_tex_coords();       
        send_screen_coords();

        glClearColor(CLR_COLOR.r, CLR_COLOR.g, CLR_COLOR.b, CLR_COLOR.a);

        makeShaderProgram("src/shaders/default.vs",
        "src/shaders/default.fs", default_object_shader);
        active_object_shader = &default_object_shader;
        makeShaderProgram("src/shaders/screen_PP.vs",
        "src/shaders/screen_PP.fs", postprocess_shader);
        active_pp_shader = &postprocess_shader;

        using namespace glm;
        model_transform = mat4(1.0);
        view_transform = lookAt(vec3(0.f, 0.f, 3.f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
        perspective_transform = perspective(radians(45.f), aspect_ratio, 0.1f, 100.f);
        return true;
    }
    void terminate()
    {
        delete[] screen_coords;
    }
}