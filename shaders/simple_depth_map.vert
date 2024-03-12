#version 460 core
layout (location = 0) in vec3 light_pos;
layout (std140, binding = 0) uniform matrices
{
    mat4 view_transform;    //0-->64
    mat4 projection_transform; //64--128
};
uniform mat4 model_transform;
void main()
{
    gl_Position = projection_transform*view_transform*model_transform*vec4(light_pos, 1.0);
}