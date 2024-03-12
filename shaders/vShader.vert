#version 460 core
layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec2 vertex_tex_coord;

out vec3 vertex_color;
out vec3 surface_normal;
out vec3 frag_pos;
out vec2 tex_coord;

uniform mat4 view_transform, projection_transform, model_transform;

void main()
{
    surface_normal  = mat3(transpose(inverse(model_transform)))*vertex_normal;
    vertex_color = (vertexPos + 1.0)/2.0;
    frag_pos = vec3(model_transform*vec4(vertexPos, 1.0));
    tex_coord = vertex_tex_coord;
    gl_Position = projection_transform*view_transform*vec4(frag_pos, 1.0);
}