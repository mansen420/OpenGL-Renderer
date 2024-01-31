#version 440 core
in vec2 texture_coordinates;
in vec2 pos_coords;
out vec4 frag_output;
uniform sampler2D screen_texture;

void main()
{
    //frag_output = vec4(texture_coordinates, 0.0, 1.0);
    //frag_output = vec4(0.5*pos_coords + vec2(0.5), 0.0, 1.0);
    frag_output = texture(screen_texture, texture_coordinates);
}