#include "shader_utils.h"
#include <algorithm>
#include "glad/glad.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include <vector>
#include <map>
#include "engine_interface.h"

using namespace renderer;

//old helper functions

//compiles and links shaders into program_id.
//Be warned that shader IDs will be inaccessable.
bool makeShaderProgram(const char* vertex_shader_path, const char* fragment_shader_path, unsigned int &program_id)
{
    unsigned int vertex_shader_id;
    char* vertex_shader_source;
    if(!readShaderFile(vertex_shader_path, vertex_shader_source))
        return false;
    if (!compileShader(VERTEX_SHADER, vertex_shader_id, vertex_shader_source))
        return false;

    unsigned int fragment_shader_id;
    char* fragment_shader_source;
    if(!readShaderFile(fragment_shader_path, fragment_shader_source))
        return false;
    if (!compileShader(FRAGMENT_SHADER, fragment_shader_id, fragment_shader_source))
        return false;
    linkShaders(program_id, vertex_shader_id, fragment_shader_id);
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);
    delete[] fragment_shader_source;
    delete[] vertex_shader_source;
    return true;
}

//Caller must ensure that file_contents_holder is delete[]`d!
bool readFile(const char* file_path, char* &file_contents_holder)
{
    std::ifstream reader;
    reader.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        reader.open(file_path);
        std::stringstream file_stream;
        file_stream << reader.rdbuf();
        reader.close();
        file_contents_holder = new char[strlen(file_stream.str().c_str())];
        strcpy(file_contents_holder, file_stream.str().c_str());
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "failed to open shader file : " << file_path << std::endl;
        return false;
    }
    return true;
}
bool compileShader(const shader_type_option shader, const unsigned int &shader_id, const char* const &shader_source)
{
    glShaderSource(shader_id, 1, &shader_source, NULL);
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
            return false;
        }
    }
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
    ID = glCreateShader(type);
    if (source != nullptr)
        this->source_code = std::string(source);  
}
bool shader_manager::shader_t::load_source_from_path(const char* const path)
{
    if(path != nullptr)
    {
        char* source_holder = nullptr;
        bool success = false;

        if (readFile(path, source_holder))
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