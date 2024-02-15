#version 450 core

layout (location = 0) in vec3     vertexPos;
layout (location = 1) in vec3 vertexNormals;
layout (location = 2) in vec2     vertexTex;

uniform mat4 view_transform, projection_transform, model_transform;

out vec3  frag_pos;
out vec3    normal;
out vec2 TexCoords;

void main()
{
    normal         = normalize(vec3(transpose(inverse(model_transform))*vec4(vertexNormals, 1.0))); 
    vec4 frag_pos4 = model_transform*vec4(vertexPos, 1.0f);

    gl_Position =      projection_transform*view_transform*frag_pos4;

    frag_pos  = frag_pos4.xyz;
    TexCoords = vertexTex;
}