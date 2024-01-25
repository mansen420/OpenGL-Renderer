#ifndef OBJ_INTRFC 
#define OBJ_INTRFC
#include "glad/glad.h"
namespace object_3D
{
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
}
#endif