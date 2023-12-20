#version 460 core
out vec4 fragment_output;
in vec3 vertex_color;
in vec2 tex_coords;
uniform sampler2D tex_sampler0;
uniform sampler2D tex_sampler1;
void main()
{
    vec2 Atex_coords = 1*tex_coords;
    // fragment_output = vec4(vertex_color, 1);
    fragment_output = mix (texture(tex_sampler0, Atex_coords), texture(tex_sampler1, Atex_coords), 1.0);
}