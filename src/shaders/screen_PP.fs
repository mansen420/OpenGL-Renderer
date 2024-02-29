#version 440 core
in vec2 texture_coordinates;
in vec2 pos_coords;
out vec4 frag_output;

uniform sampler2D screen_texture;
uniform bool rendering_depth;
uniform vec3 depth_view_color;
void main()
{
    if (rendering_depth)
    {
        float intensity = texture(screen_texture, texture_coordinates).r;
        frag_output = intensity*vec4(depth_view_color, 1.0);
        return;
    }
    frag_output = texture(screen_texture, texture_coordinates);
}