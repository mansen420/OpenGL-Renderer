#include "shader_utils.h"
bool readShaderFile(const char* file_path, char* &file_contents_holder)
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
bool compileShader(shader_type_option shader, unsigned int &shader_id, const char* const &shader_source)
{
    GLenum shader_type;
    if (shader == VERTEX_SHADER)
        shader_type = GL_VERTEX_SHADER;
    if (shader == FRAGMENT_SHADER)
        shader_type = GL_FRAGMENT_SHADER;

    shader_id = glCreateShader(shader_type);
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
bool compileShaderFromPath(shader_type_option shader, unsigned int &shader_id, const char* file_path)
{
    char* shader_source;
    if (!readShaderFile(file_path, shader_source))
        return false;
    if (!compileShader(shader, shader_id, shader_source))
        return false;
    delete[]shader_source;
    return true;
}
bool linkShaders(unsigned int &program_id, const unsigned int& vertex_shader_id, const unsigned int& fragment_shader_id)
{
    program_id  = glCreateProgram();
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
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