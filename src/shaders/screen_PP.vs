#version 440 core
layout (location = 0) in vec2 screen_coords;
layout (location = 1) in vec2 tex_coords;

out vec2 texture_coordinates;
out vec2 pos_coords;

void main()
{
    pos_coords = screen_coords;
    texture_coordinates = tex_coords;
    gl_Position = vec4(screen_coords, 0.0 , 1.0);
}