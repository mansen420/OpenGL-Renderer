#version 460 core
layout (location = 0) in vec3 vertexPos;
layout (location = 0) in vec2 vertex_tex_coords;
out vec3 vertex_color;
out vec2 tex_coords;
void main()
{
    tex_coords = vertex_tex_coords;
    vertex_color = (vertexPos + 1.0)/2.0;
    gl_Position = vec4(vertexPos, 1.0);
}