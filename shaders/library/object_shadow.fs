#version 450 core 
#include "shadow_h.fs"
#include "object_shader_h.fs"

float calculate_shadow(vec4 lightspace_pos)
{
    vec3 projected_coordinates = lightspace_pos.xyz / lightspace_pos.w;
    projected_coordinates *= 0.5;
    projected_coordinates += 0.5;
    if(projected_coordinates.z  > 1.0)
    {
        return 0.0;
    }
    float closest_depth = texture(shadow_map, projected_coordinates.xy).r;
    float current_depth = projected_coordinates.z;
    float n = dot(normalize(normal), normalize(light_pos - frag_pos));
    float bias =  -0.01*(n)+(n-1)*0.01;

    vec2 texel_size = 1.0/textureSize(shadow_map, 0);
    float shadow = 0.0;
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float PCF_depth = texture(shadow_map, projected_coordinates.xy + vec2(x, y)*texel_size).r;
            shadow += current_depth + bias> PCF_depth ? 0.0 : 1.0;
        }
    }
    return shadow/9.0;
}