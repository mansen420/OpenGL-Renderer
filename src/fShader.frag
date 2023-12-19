#version 460 core
out vec4 fragment_output;
in vec3 vertex_color;
void main()
{
    fragment_output = vec4(vertex_color, 1.0);
}