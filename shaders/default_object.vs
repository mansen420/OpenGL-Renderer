#version 450 core
#include "object_shader_h.vs"
void main()
{
    normal         = normalize(vec3(transpose(inverse(model_transform))*vec4(vertexNormals, 1.0))); 
    vec4 frag_pos4 = model_transform*vec4(vertexPos, 1.0f);

    gl_Position =      projection_transform*view_transform*frag_pos4;

    frag_pos  = frag_pos4.xyz;
    tex_coords = vertexTex;
    lightspace_frag_pos = lightspace_transform*vec4(frag_pos, 1.0);
}