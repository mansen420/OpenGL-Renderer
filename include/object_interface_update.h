#ifndef OBJ_INTRFC 
#define OBJ_INTRFC
#include "glad/glad.h"
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace object
{
    using namespace glm;
    struct vertex
    {
        vec3 pos_coords;
        vec3 normal_coords;
        vec2 tex_coords;
    };
    class drawable
    {
    protected:
        //Binds VAOs, if any, for the draw call.
        virtual void bind_VAO() const = 0;
        //overridable draw command. this function should handle the drawing of your object(s).
        virtual void gl_draw(const unsigned int &program_id) const = 0;
        //sends all uniforms to the shader programs 
        virtual void send_uniforms(const unsigned int &program_id) const = 0;
    public:
        //generates VAO(s) and/or sends buffer data.
        virtual void send_data() = 0;

        //caller must ensure that send_data() has been called before this. 
        virtual void draw(const unsigned int &program_id) const final 
        {
            glUseProgram(program_id);
            send_uniforms(program_id);
            bind_VAO();
            gl_draw(program_id);
        }
    };
    class mesh : public drawable
    {
        unsigned int VAO_id;
        virtual void bind_VAO() const override { glBindVertexArray(VAO_id);}
        virtual void gl_draw(const unsigned int &program_id) const override{ glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0); }
        virtual void send_uniforms (const unsigned int &program_id) const{}
    public :
        std::vector<unsigned int> indices;

        mesh(){}

        virtual void send_data()
        {   //glVertexAttribPointer will only have effect on the data sent by the last call to glBufferData
            //VAOs only store the last call to glVertexAttribPointer
            glGenVertexArrays(1, &VAO_id);
            glBindVertexArray(VAO_id);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal_coords));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, tex_coords));
            glEnableVertexAttribArray(2);         

            unsigned int EBO_id;
            glGenBuffers(1, &EBO_id);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_id);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

            glBindVertexArray(0);
        }
    };
}
#endif