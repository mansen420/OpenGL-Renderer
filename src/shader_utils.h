#pragma once
#include "engine_interface.h"
#include <vector>

namespace renderer
{
    namespace shader_manager
    {
        class shader_t
        {
            public :
            std::string source_code;
        
            shader_t(renderer::shader_type_option type, const char* source = nullptr);

            bool load_source_from_path(const char* const filename);
            bool compile() const;
            //Note that after calling this, it becomes impossible to access the old source code.
            bool unroll_includes();
            const unsigned int get_ID()const {return ID;}
            const renderer::shader_type_option get_type()const {return type;}
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