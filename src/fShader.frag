#version 460 core
out vec4 fragment_output;

in vec3 vertex_color;
in vec3 surface_normal;
in vec3 frag_pos;

uniform sampler2D tex_sampler0;
uniform sampler2D tex_sampler1;

uniform bool emissive;
uniform vec3 light_ambient;
uniform vec3 light_diffuse;
uniform vec3 light_pos;
uniform vec3 eye_pos;

// vec3 object_color = vec3(0.45, 0.5, 0.65);
vec3 object_color = vertex_color;
void main()
{
    if(emissive)
    {
        fragment_output = vec4(1.0);
        return;
    }
    //calculate diffuse intensity 
    vec3 direction = normalize(light_pos-frag_pos);
    float cosine_theta = dot(direction, normalize(surface_normal));
    float light_intensity = max(0, cosine_theta);
    //calculate specular intensity 
    vec3 view_direction = normalize(eye_pos-frag_pos);
    vec3 reflect_direction = reflect(-direction, surface_normal);
    float angular_intensity = max(dot(view_direction, normalize(reflect_direction)), 0);
    float spec_intensity = pow(angular_intensity, 32);

    fragment_output = vec4(object_color*(light_intensity*light_diffuse + light_ambient + spec_intensity*light_diffuse), 1);   
}