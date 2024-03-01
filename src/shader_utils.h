#pragma once
#include "engine_interface.h"
#include <vector>

//reads file into file_contents_holder
//WARNING : caller must ensure that file_contents_holder is properly delete[]`d.
bool readShaderFile(const char* file_path, char* &file_contents_holder);
bool compileShader(renderer::shader_type_option shader, unsigned int &shader_id, const char* const &shader_source);
bool compileShaderFromPath(renderer::shader_type_option shader, unsigned int &shader_id, const char* file_path);
bool linkShaders(unsigned int &program_id, const unsigned int& vertex_shader_id, const unsigned int& fragment_shader_id);
//compiles and links shaders into program_id.
//Be warned that shader IDs will be inaccessable.
bool makeShaderProgram(const char* vertex_shader_path, const char* fragment_shader_path, unsigned int &program_id);

namespace renderer
{
    namespace shader_manager
    {
        class shader_t
        {
            public :
            shader_t(renderer::shader_type_option type, const char* source = nullptr);
            bool load_source_from_path(const char* const path);
            bool compile() const;
            const unsigned int get_ID()const {return ID;}
            const renderer::shader_type_option get_type()const {return type;}
            std::string source_code;
            ~shader_t();

            private:
            
            renderer::shader_type_option type;
            unsigned int ID = 0;
        };
        class shader_prg_t
        {
            public :
            shader_prg_t();

            void attach_shader(const shader_t &shader);
            void attach_shader(const unsigned int &shader_ID);
            void detach_shader(const unsigned int &shader_ID);
            void detach_shader(const shader_t &shader);

            bool link();

            ~shader_prg_t();

            const unsigned int* get_attached_shader_IDs()const{return attached_shader_IDs.data();}
            size_t num_attached_shaders()const{return attached_shader_IDs.size();}
            const unsigned int get_ID()const{return ID;}
            private :
            std::vector<unsigned int> attached_shader_IDs;
            unsigned int ID = 0;   //0 for sensible GL behavior
        };
    }
}