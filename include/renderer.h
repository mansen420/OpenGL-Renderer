#ifndef RENDERER
#define RENDERER

#include "glad/glad.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "shader_utils.h"
#include "object_interface.h"

#include "global_constants.h"

//render settings
bool DEPTH_CLR_ENBLD = 1, COLOR_CLR_ENBLD = 1;
bool DEPTH_TEST_ENBLD = 1;
glm::vec4 CLR_COLOR(0.6, 0.3, 0.3, 1.0);

namespace renderer
{
    unsigned int program_id;

    glm::mat4 model_transform;
    glm::mat4 view_transform;
    glm::mat4 perspective_transform;

    object_3D::object my_object;

    inline void clear_buffers()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(CLR_COLOR.r, CLR_COLOR.g, CLR_COLOR.b, CLR_COLOR.a);
    }
    inline void send_uniforms()
    {
        glUniformMatrix4fv(glGetUniformLocation(program_id, "projection_transform"), 1, GL_FALSE,
        glm::value_ptr(perspective_transform));
        glUniformMatrix4fv(glGetUniformLocation(program_id, "view_transform"), 1, GL_FALSE,
        glm::value_ptr(view_transform));
    }
    inline void render_scene()
    {
        clear_buffers();

        glUseProgram(program_id);
        model_transform = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.f, 1.f, 0.f));
        send_uniforms();
        my_object.model_transform = model_transform;
        my_object.draw(program_id);
    }
    //we need abstraction for ()shaders, ()uniforms, (x)drawing
    inline void init()
    {
        read_obj("assets/backpack/backpack.obj", my_object);
        my_object.send_data();

        if(DEPTH_TEST_ENBLD)
            glEnable(GL_DEPTH_TEST);

        makeShaderProgram("src/shaders/default.vs", "src/shaders/default.fs", program_id);

        using namespace glm;
        model_transform = mat4(1.0);
        view_transform = lookAt(vec3(0.f, 0.f, 3.f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
        perspective_transform = perspective(radians(45.f), aspect_ratio, 0.1f, 100.f);
    }
}

#endif 