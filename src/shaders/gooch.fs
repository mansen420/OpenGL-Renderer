#version 450 core

out vec4 fragment_output;

in vec3  frag_pos;
in vec3    normal;
in vec2 TexCoords;

struct light
{
    vec4 pos;
    vec3 color;
};
vec3 warm_color, cool_color;

uniform vec3 view_vector;

void main()
{
    warm_color = vec3(0.65, 0.4, 0.0);
    cool_color = vec3(0.0, 0.2, 0.5);

    light foo_light = light(vec4(1.0), vec3(1.0));

    float t;
    float s;
    {
    vec3  l = normalize(foo_light.pos.xyz - frag_pos);
    vec3  n = normalize(normal);
          t = max(dot( l, n)/2.0 + 0.5, 0.0);
    vec3  r = reflect(-l, n);
          s = clamp(pow(max(dot(normalize(r), normalize(view_vector)), 0.0), 32), 0.0, 1.0);
    }
    vec3 res = warm_color*t+(1-t)*cool_color;
    res = s * vec3(1.0) + (1-s) * (res);
    fragment_output = vec4(res, 1.0);
}