#include "shader_utils.h"
#include <algorithm>
#include "glad/glad.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <filesystem>
#include "engine_interface.h"
#include "shader_preprocessor.h"
#include "global_constants.h"
#include "read_file.h"
#include "gl_enum_converters.h"
using namespace renderer;

//helper functions
bool compileShader(const shader_type_option shader, const unsigned int &shader_id, const char* shader_source)
{
    char* processed_shader_source = nullptr;
    if (!preprocessor::process_shader(shader_source, processed_shader_source))
    {
        delete[] processed_shader_source;
        return false;
    }

    glShaderSource(shader_id, 1, &processed_shader_source, NULL);
    glCompileShader(shader_id);
    {
        int success_status;
        char infoLog[512];
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success_status);
        if(!success_status)
        { 
            glGetShaderInfoLog(shader_id, 512, NULL, infoLog);
            std::cout << "compilation failed : " << (shader == VERTEX_SHADER ? "vertex  shader" : "fragment shader") 
            << "\n" << infoLog << std::endl;
            delete[] processed_shader_source;
            return false;
        }
    }
    delete[] processed_shader_source;
    return true;
}
bool linkShaders(unsigned int &program_id, std::vector<unsigned int> shader_ids)
{
    glLinkProgram(program_id);
    {  
        int success_status;
        char infoLog[512];
        glGetProgramiv(program_id, GL_LINK_STATUS, &success_status);
        if(!success_status)
        {
            glGetProgramInfoLog(program_id, 512, NULL, infoLog);
            std::cout << "Linking failed : " << infoLog << std::endl;
            return false;
        }
    } 
    return true;
}


shader_manager::shader_t::shader_t(shader_type_option type, const char* const source)
{
    this->type = type;
    ID = glCreateShader(convert(type));
    if (source != nullptr)
        this->source_code = std::string(source);  
}
bool shader_manager::shader_t::load_source_from_path(const char* const filename)
{
    if(filename != nullptr)
    {
        char* source_holder = nullptr;
        bool success = false;
        
        std::filesystem::path path_to_shader;
        for(const auto& entry : std::filesystem::recursive_directory_iterator(SHADER_DIR_PATH))
        {
            if(entry.path().filename() == filename)
                path_to_shader = entry;
        }
        std::cout << "Loaded shader source  : " << path_to_shader.c_str() << std::endl;
        if (readFile(path_to_shader.c_str(), source_holder))
        {
            source_code = std::string(source_holder);
            success = true;
        }
        else
            success = false;
    
        delete[] source_holder;
        return success;
    }
    else 
        return false;
}  
bool shader_manager::shader_t::compile() const{return compileShader(type, ID, source_code.c_str());}
bool shader_manager::shader_t::unroll_includes()
{
    char* processed_source =  nullptr;
    renderer::preprocessor::process_shader(source_code.c_str(), processed_source);
    source_code = std::string(processed_source);
    delete[] processed_source;
    return true;
}
shader_manager::shader_t::~shader_t()
{
    if(glIsShader(ID) == GL_TRUE)
    {
        glDeleteShader(ID);
    }
}

shader_manager::shader_prg_t::shader_prg_t(){ID = glCreateProgram();}
void shader_manager::shader_prg_t::attach_shader(const shader_t &shader)
{
    attach_shader(shader.get_ID());
}
void shader_manager::shader_prg_t::attach_shader(const unsigned int &shader_ID)
{
    glAttachShader(ID, shader_ID);
    attached_shader_IDs.push_back(shader_ID);
}
void shader_manager::shader_prg_t::detach_shader(const unsigned int &shader_ID)
{
    glDetachShader(ID, shader_ID);
    auto it = std::find(attached_shader_IDs.begin(), attached_shader_IDs.end(), shader_ID);
    if (it != attached_shader_IDs.end())
        attached_shader_IDs.erase(it);
}
void shader_manager::shader_prg_t::detach_shader(const shader_t &shader)
{
    detach_shader(shader.get_ID());
}

bool shader_manager::shader_prg_t::link()
{
    return linkShaders(ID, attached_shader_IDs);
}
shader_manager::shader_prg_t::~shader_prg_t()
{
    if (glIsProgram(ID))
        glDeleteProgram(ID);
}