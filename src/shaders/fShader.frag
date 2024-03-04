#version 460 core
out vec4 fragment_output;

in vec3 vertex_color;
in vec3 surface_normal;
in vec2 tex_coord;
in vec3 frag_pos;

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
uniform vec3 view_vector;
const int NR_LIGHTS = 5;
uniform light lights[NR_LIGHTS];
int valid_size = 1;

vec4 shade_directional(light dir_light);
vec4 shade_point(light point_light);
vec3 shade_spot(spotlight s_light);
//statics
vec3 obj_color = vec3(0.5, 0.5, 0.75);
vec4 diffuse_map = vec4(obj_color, 1.0);
vec4 spec_map = vec4(1.0);
//vec3 object_color = vertex_color;

const float SHININESS = 24.0;
const float KL = 0.00;    //linear distance attenuation factor
const float KQ = 1.00;    //quadratic distance attenuation factor
void main()
{
    float gamma = 2.2;
    spec_map = vec4(pow(spec_map.rgb, vec3(1/gamma)), spec_map.a);
    vec4 light_output = vec4(0, 0, 0, 1);
    for (int i = 0; i < valid_size; i++)
    {
        if (lights[i].pos.w==1)
            light_output += shade_point(lights[i]);
        else if (lights[i].pos.w==0)
        {
            if(lights[i].pos == vec4(0.0))
                light_output += vec4(lights[i].color, 1.0);
            else
                   light_output += shade_directional(lights[i]);
        }
    }
    spotlight s;
    s.core.color = vec3(1.0);
    s.core.pos = vec4(view_vector, 1);
    s.direction = vec3(0, 0, -1);
    s.cosine_angle = cos(radians(12.5));
    light_output += vec4(shade_spot(s), 1);
    light sunlight;
    sunlight.pos = vec4(0.0, -1.0, 0.0, 0.0);
    sunlight.color = vec3(1.0, 0.85, 0.65);
    light_output += shade_directional(sunlight);
    fragment_output = vec4(pow(light_output.rgb, vec3(1/gamma)), 1.0);
}
float spec(vec3 light_dir)
{   //expects light_dir TOWARDS the surface
    vec3 view_direction = normalize(view_vector-frag_pos);
    //vec3 reflect_direction = reflect(vec3(light_dir), normalize(surface_normal));
    vec3 halfway_vector;
    if (dot(normalize(surface_normal), normalize(light_dir))>0)
        return 0;
    else 
        halfway_vector = normalize(view_direction+light_dir);
    float angular_intensity = max(dot(halfway_vector, normalize(surface_normal)), 0);
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
        float distance_attenuation = 1.0/(d*d);
        //the higher this is, the closer we are to the center of the spotlight
        float attenuation_factor = (cos_phi-s_light.cosine_angle);  //[0, 1-cosine_angle]
        attenuation_factor /= (1-s_light.cosine_angle); //[0, 1]
        return distance_attenuation*(0.5*attenuation_factor + 0.5*pow(attenuation_factor, 2))*   //sharpness factor * attenuation factor 
        (vec3(diffuse_map)*s_light.core.color + vec3(spec_map)*spec(s_light.direction));
    }
    return vec3(0, 0, 0);
}
vec4 shade_point(light point_light)
{
    vec3 light_dir = normalize(vec3(point_light.pos)-frag_pos);
    float diffuse_intensity = max(dot(light_dir,  normalize(surface_normal)), 0.0);
    float d = length(frag_pos-vec3(point_light.pos));
    float attenuation = 1.0/(1.0+KL*d+KQ*d*d);
    return attenuation*((vec4(diffuse_intensity*diffuse_map)*vec4(point_light.color, 1))+(spec(-light_dir)*spec_map));
}
vec4 shade_directional(light dir_light)
{
    float diffuse_intensity = max(dot(normalize(-vec3(dir_light.pos)),  normalize(surface_normal)), 0.0);
    return (spec(vec3(dir_light.pos))*spec_map + diffuse_intensity*diffuse_map)*vec4(dir_light.color, 1.0);
}