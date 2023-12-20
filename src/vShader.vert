#version 460 core
layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec2 vertex_tex_coords;
out vec3 vertex_color;
out vec2 tex_coords;
uniform mat4 model_transform, view_transform, projection_transform;
void main()
{
    tex_coords = vertex_tex_coords;
    vertex_color = (vertexPos + 1.0)/2.0;
    gl_Position = projection_transform*view_transform*model_transform*vec4(vertexPos, 1.0);
}