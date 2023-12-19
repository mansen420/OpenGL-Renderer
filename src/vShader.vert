#version 460 core
layout (location = 0) in vec3 vertexPos;
out vec3 vertex_color;
void main()
{
    vertex_color = vertexPos;
    gl_Position = vec4(vertexPos, 1.0);
}