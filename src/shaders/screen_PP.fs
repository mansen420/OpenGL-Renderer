#version 440 core
in vec2 texture_coordinates;
in vec2 pos_coords;
out vec4 frag_output;
uniform sampler2D screen_texture;

void main()
{
    frag_output = texture(screen_texture, texture_coordinates);
}