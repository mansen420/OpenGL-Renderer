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
    3*normalize(vec4(vec3(t), 1.0)) 
    * normalize(vec4(0.9, 0.8, 0.75, 1.0)) 
    + 0.5*normalize(vec4(vec3(s), 1.0))
    * normalize(vec4(1.0, 1.0, 1.0, 1.0));

    fragment_output *= clamp(calculate_shadow(lightspace_frag_pos), 0.25, 1.0);
}