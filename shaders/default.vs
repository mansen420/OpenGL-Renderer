#version 450 core
layout (location = 0) in vec3 vertexPos;

uniform mat4 view_transform, projection_transform, model_transform;

void main()
{
    gl_Position = projection_transform*view_transform*model_transform*vec4(vertexPos, 1.0f);
}