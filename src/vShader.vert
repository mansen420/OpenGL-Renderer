#version 460 core
layout (location = 0) in vec3 vertexPos;
out vec3 vertex_color;
uniform vec3 pos_offset;
void main()
{
    vertex_color = vertexPos + pos_offset;
    gl_Position = vec4(vertexPos + pos_offset, 1.0);
}