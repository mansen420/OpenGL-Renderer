#version 450 core
layout (location = 0) in vec3 scene_pos;

uniform mat4 projection_transform;
uniform mat4 view_transform;
uniform mat4 model_transform;

out vec4 frag_pos;
void main()
{
    frag_pos = view_transform*model_transform*vec4(scene_pos, 1.0);
    gl_Position = projection_transform*frag_pos;
}