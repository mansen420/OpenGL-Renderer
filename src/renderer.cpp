#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION ;
#include "renderer.h"

//TODO fix these
#include "shader_utils.h"
#include "object_interface.h"
//render settings
bool DEPTH_CLR_ENBLD = 1, COLOR_CLR_ENBLD = 1;
bool DEPTH_TEST_ENBLD = 1;
glm::vec4 CLR_COLOR(0.6, 0.3, 0.3, 1.0);
//TODO time to add an offscreen pass
namespace renderer
{
    static unsigned int postprocess_shader;
    static unsigned int default_object_shader;
    static unsigned int offscreen_tex_ids[2];  // [0] = color, [1] = depth+stencil
    static unsigned int offscreen_framebuffer_id;
    static unsigned int screen_vao;

    glm::mat4 model_transform;
    glm::mat4 view_transform;
    glm::mat4 perspective_transform;

    object_3D::object my_object;

    const static float screen_coords[] = {

    // positions    // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f, 
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f, 

    -1.0f,  1.0f,  0.0f, 1.0f, 
     1.0f,  1.0f,  1.0f, 1.0f, 
     1.0f, -1.0f,  1.0f, 0.0f
    };
    static const float screen_indices[]
    {
        0, 1, 2, 2, 1, 3 
    };
    void send_uniforms()
    {
        glUniformMatrix4fv(glGetUniformLocation(default_object_shader, "projection_transform"), 1, GL_FALSE,
        glm::value_ptr(perspective_transform));
        glUniformMatrix4fv(glGetUniformLocation(default_object_shader, "view_transform"), 1, GL_FALSE,
        glm::value_ptr(view_transform));
    }
    void offscreen_pass()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, offscreen_framebuffer_id);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(default_object_shader);

        model_transform = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.f, 1.f, 0.f));
        send_uniforms();
        my_object.model_transform = model_transform;

        my_object.draw(default_object_shader);
    }
    void main_pass()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glUseProgram(postprocess_shader);

        glBindVertexArray(screen_vao);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, offscreen_tex_ids[0]);
        glUniform1i(glGetUniformLocation(postprocess_shader, "screen_texture"), 0);
        
        glPointSize(10.0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    void render_scene()
    {
        offscreen_pass();
        main_pass();
    }
    int init()
    {
        read_obj("assets/backpack/backpack.obj", my_object);
        my_object.send_data();

        if(DEPTH_TEST_ENBLD)
            glEnable(GL_DEPTH_TEST);

        //set up off-screen frame buffer
        glGenFramebuffers(1, &offscreen_framebuffer_id);
        glBindFramebuffer(GL_FRAMEBUFFER, offscreen_framebuffer_id);

        glGenTextures(2, offscreen_tex_ids);

        unsigned int RENDER_W = OPENGL_VIEWPORT_W, RENDER_H = OPENGL_VIEWPORT_H;
        //color attachment
        glBindTexture(GL_TEXTURE_2D, offscreen_tex_ids[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, RENDER_W, RENDER_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glGenerateMipmap(GL_TEXTURE_2D);

        //depth+depth since we need both, and the author's machine only supports the combined format in this case
        glBindTexture(GL_TEXTURE_2D, offscreen_tex_ids[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, RENDER_W, RENDER_H, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
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
        // 
        glClearColor(CLR_COLOR.r, CLR_COLOR.g, CLR_COLOR.b, CLR_COLOR.a);

        //init screen drawable 
        unsigned int screen_vbo;
        glGenBuffers(1, &screen_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, screen_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(screen_coords), screen_coords, GL_STATIC_DRAW);

        glGenVertexArrays(1, &screen_vao);
        glBindVertexArray(screen_vao);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)(sizeof(float)*2));
        glEnableVertexAttribArray(1);

        //unbind your buffer ONLY after calling the attribute pointer, you risk severe consequences otherwise!
        glBindBuffer(GL_ARRAY_BUFFER, 0); 

        glBindVertexArray(0);
        //
        makeShaderProgram("src/shaders/default.vs", "src/shaders/default.fs", default_object_shader);
        makeShaderProgram("src/shaders/screen_PP.vs", "src/shaders/screen_PP.fs", postprocess_shader);

        using namespace glm;
        model_transform = mat4(1.0);
        view_transform = lookAt(vec3(0.f, 0.f, 3.f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
        perspective_transform = perspective(radians(45.f), aspect_ratio, 0.1f, 100.f);
        return true;
    }
}