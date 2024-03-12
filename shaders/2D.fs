#version 450 core
#include "object_shader_h.fs"
#include "shadow_h.fs"

void main()
{
    light foo_light = light(vec4(vec3(light_pos), 1.0), vec3(1.0));

    float t;
    float s;
    {
    vec3  l = normalize(foo_light.pos.xyz - frag_pos);
    vec3  n = normalize(normal);
          t = max(dot( l, n)/2.0 + 0.5, 0.0);
    vec3  r = reflect(-l, n);
          s = clamp(pow(max(dot(normalize(r), normalize(view_vector)), 0.0), 0), 
          0.0, 1.0);
    }
    fragment_output = 
    0*normalize(vec4(vec3(pow(t, 1)), 1.0)) 

    + 0.7*(vec4(vec3(s), 1.0))
    * (vec4(1.0, 1.0, 1.0, 1.0));
}

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

