#version 460 core
layout (location = 0) in vec3 vertexPos;
void main()
{
    gl_Position = vec4(vertexPos, 1.0);
}