#version 450 core

out vec4 fragment_output;

in vec3            frag_pos;
in vec3              normal;
in vec2           TexCoords;
in vec4 lightspace_frag_pos;

struct light
{
    vec4 pos;
    vec3 color;
};

uniform vec3 view_vector;
uniform vec3 light_pos;
uniform sampler2D shadow_map;

float calculate_shadow(vec4 lightspace_pos)
{
    vec3 projected_coordinates = lightspace_pos.xyz / lightspace_pos.w;
    projected_coordinates *= 0.5;
    projected_coordinates += 0.5;
    float closest_depth = texture(shadow_map, projected_coordinates.xy).r;
    float current_depth = projected_coordinates.z;
    float n = dot(normalize(normal), normalize(light_pos - frag_pos));
    float bias =  -0.01*(n)+(n-1)*0.01;
    return current_depth + bias> closest_depth ? 0.0 : 1.0;
}