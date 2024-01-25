#ifndef RENDERER
#define RENDERER

#include "glad/glad.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace renderer
{
    void render_background(float r, float g, float b, float a)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(r, g, b, a);
    }
    void render_scene();    //render every mesh
}
//what we need to draw a mesh : VAO/VBO/EBO, shader program

#endif 