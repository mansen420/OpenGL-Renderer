out vec4 fragment_output;

//TODO standardize naming 
in vec3   frag_pos;
in vec3     normal;
in vec2 tex_coords;

struct light
{
    vec4 pos;
    vec3 color;
};

uniform vec3     view_vector;
uniform vec3       light_pos;