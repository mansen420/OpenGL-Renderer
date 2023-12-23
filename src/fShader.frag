#version 460 core
out vec4 fragment_output;

in vec3 vertex_color;
in vec3 surface_normal;
in vec2 tex_coord;
in vec3 frag_pos;

uniform sampler2D tex_sampler0;
uniform sampler2D tex_sampler1;
uniform sampler2D tex_sampler2;

struct light 
{
    vec4 pos;   //pos.w == 0 for directional light, pos == vec4(0) for ambient light,
    vec3 color; //otherwise, point light assumed.
};
struct spotlight
{
    light core;
    vec3 direction;
    float cosine_angle;
};
uniform bool emissive;
uniform vec3 eye_pos;
const int NR_LIGHTS = 5;
uniform light lights[NR_LIGHTS];
int valid_size = 1;
//uniform spotlight test_light;

vec3 shade_directional(light dir_light);
vec3 shade_point(light point_light);
vec3 shade_spot(spotlight s_light);
//statics
vec4 diffuse_map = texture(tex_sampler0, tex_coord);
vec4 spec_map = texture(tex_sampler1, tex_coord);
vec3 object_color = vertex_color;
const float SHININESS = 64;
const float KL = 0.22;    //linear attenuation factor
const float KQ = 0.20;    //quadratic attenuation factor
void main()
{
    if(emissive)
    {
        fragment_output = vec4(1.0);
        return;
    }
    vec3 light_output = vec3(0,0,0);
    for (int i = 0; i<valid_size; i++)
    {
        if (lights[i].pos.w==1)
            light_output += shade_point(lights[i]);
        else if (lights[i].pos.w==0)
        {
            if(lights[i].pos == vec4(0.0))
                light_output += lights[i].color;
            else
                   light_output += shade_directional(lights[i]);
        }
    }    
    fragment_output = vec4(light_output, 1);
}
float spec(vec3 light_dir)
{   //expects light_dir TOWARDS the surface
    vec3 view_direction = normalize(eye_pos-frag_pos);
    vec3 reflect_direction = reflect(vec3(light_dir), normalize(surface_normal));
    float angular_intensity = max(dot(view_direction, normalize(reflect_direction)), 0);
    float spec_intensity = pow(angular_intensity, SHININESS);
    return spec_intensity;
}
vec3 shade_spot(spotlight s_light)
{
    vec3 differential = vec3(s_light.core.pos)-frag_pos;    //change these names
    float cos_phi = dot(normalize(differential), normalize(-s_light.direction)); //equal to cos(phi)
    if (cos_phi>=s_light.cosine_angle)
    {
        float d = length(frag_pos-vec3(s_light.core.pos));
        float distance_attenuation = 1.0/(1.0+0*d+(0.1)*d*d);
        //the higher this is, the closer we are to the center of the spotlight
        float attenuation_factor = (cos_phi-s_light.cosine_angle);  //[0, 1-cosine_angle]
        attenuation_factor /= (1-s_light.cosine_angle); //[0, 1]
        return distance_attenuation*(0.5*attenuation_factor + 1.5*pow(attenuation_factor, 2))*   //sharpness factor * attenuation factor 
        (vec3(diffuse_map)*s_light.core.color + vec3(spec_map)*spec(s_light.direction));
    }
    return vec3(0, 0, 0);
}
vec3 shade_point(light point_light)
{
    vec3 light_dir = normalize(vec3(point_light.pos)-frag_pos);
    float diffuse_intensity = max(dot(light_dir,  normalize(surface_normal)), 0.0);
    float d = length(frag_pos-vec3(point_light.pos));
    float attenuation = 1.0/(1.0+KL*d+KQ*d*d);
    return attenuation*((vec3(diffuse_intensity*diffuse_map)*point_light.color)+vec3(spec(-light_dir)*spec_map));
}
vec3 shade_directional(light dir_light)
{
    float diffuse_intensity = max(dot(normalize(-vec3(dir_light.pos)),  normalize(surface_normal)), 0.0);
    return vec3(spec(vec3(dir_light.pos))*spec_map + diffuse_intensity*diffuse_map)*dir_light.color;
}