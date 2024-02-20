#pragma once

#include "glad/glad.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>

enum shader_type_option
{
    VERTEX_SHADER,
    FRAGMENT_SHADER,
    GEOMETRY_SHADER
};
//reads file into file_contents_holder
//WARNING : caller must ensure that file_contents_holder is properly delete[]`d.
bool readShaderFile(const char* file_path, char* &file_contents_holder);
bool compileShader(shader_type_option shader, unsigned int &shader_id, const char* const &shader_source);
bool compileShaderFromPath(shader_type_option shader, unsigned int &shader_id, const char* file_path);
bool linkShaders(unsigned int &program_id, const unsigned int& vertex_shader_id, const unsigned int& fragment_shader_id);
//compiles and links shaders into program_id.
//Be warned that shader IDs will be inaccessable.
bool makeShaderProgram(const char* vertex_shader_path, const char* fragment_shader_path, unsigned int &program_id);