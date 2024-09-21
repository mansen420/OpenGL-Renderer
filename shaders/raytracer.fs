#version 450 core
uniform float rand;
uniform float time;

out vec4 frag_output;
in vec2 texture_coordinates;
in vec2 pos_coords;


struct sphere
{
    vec3   center;
    float  radius;
    vec3    color;
    bool emissive;
};
struct light
{
    vec3 pos;
    vec3 color;
};
struct ray
{
    vec3 origin;
    vec3 direction;
};
vec3 extend_ray(ray r, float t)
{
    return r.origin + t*normalize(r.direction);
}

bool ray_sphere_intersection(ray r, sphere s)
{
    vec3 d = normalize(r.direction), e = r.origin, c = s.center;
    float R = s.radius;
    float discriminant = pow(dot(d, e - c), 2.0) - dot(d, d)*(dot(e - c, e - c) - R*R);
    return discriminant > 0.0;
}
float ray_sphere_intersection_root(in ray r, in sphere s, in float t_min, in float t_max, out bool intersect)
{
    vec3 d = normalize(r.direction), e = r.origin, c = s.center;
    float R = s.radius;
    float discriminant = pow(dot(d, e - c), 2.0) - dot(d, d)*(dot(e - c, e - c) - R*R);
    if(discriminant<0)
    {
        intersect = false;
        return t_min - 1.0; //overflow?
    }

    float close_root = (dot(-d, e - c) - discriminant)/(dot(d, d));
    if (close_root > t_min && close_root < t_max)
    {
        intersect = true;
        return close_root;
    }
    float far_root = (dot(-d, e - c) + discriminant)/(dot(d, d));
    if (far_root < t_max && far_root > t_min)
    {
        intersect = true;
        return far_root;
    }

    intersect = false;
    return t_max + 1.0; //watch out for  overflow 
}
struct camera_space
{
    vec3 origin;
    vec3 w;
    vec3 v;
    vec3 u;
};
camera_space prepare_cam_space(vec3 up, vec3 view_dir, vec3 origin)
{
    return camera_space(origin, normalize(view_dir), normalize(up), normalize(cross(up, view_dir)));
}
vec3 eye_pos = vec3(0.0, 0.0, 0.0);
vec3 up      = normalize(vec3(0.0, 1.0, 0.0));
vec3 w       = normalize(vec3(0.0, 0.0, 1.0));
camera_space cam_space = prepare_cam_space(up, w, eye_pos);

vec4 background_color = vec4(1.0, 0.0, 0.0, 1.0);

ray project_ray_ortho(camera_space cam_space){return ray(cam_space.origin + cam_space.u*pos_coords.x + cam_space.v*pos_coords.y, normalize(-cam_space.w));}
ray project_ray_perspective(camera_space cam_space, float focal_length)
{
    return ray(cam_space.origin, 
    normalize(cam_space.origin - cam_space.w*focal_length + cam_space.u*pos_coords.x + cam_space.v*pos_coords.y));
}
light LIGHT = light(vec3(1), vec3(1));
vec3 trace(in ray r, in sphere[2] s, in int depth)
{
    float t_min = 0, t_max = 10.0;

    vec3 result = vec3(1.0);
    int count = 0;
    ray r_in = r;
    while(count++ < depth)
    {
        bool hit_object = false;
        float closest_intersection = t_max;
        sphere obj;
        for(int i = 0; i < 2; ++i)
        {
            bool intersect;
            float root = ray_sphere_intersection_root(r_in, s[i], t_min, closest_intersection, intersect);
            if(intersect)
            {
                closest_intersection = root;
                hit_object = true;
                obj = s[i];
            }
        }
        if(hit_object)
        {
            if(obj.emissive)
            {
                result *= 1.0;
                break;
            }
            else
            {
                vec3 P = extend_ray(r_in, closest_intersection);
                vec3 n = normalize(P - obj.center);
                result *= obj.color;
                float intensity = dot(n, normalize(LIGHT.pos - P));
                result *= intensity;
                break;
            }
            //vec3 view_vec = P - cam_space.origin;
            //vec3 r = normalize(reflect(-view_vec, n));
            //r_in.direction = normalize(r); 
        }
        else
        {
            result = vec3(0.5, 0.8, 1.0);
            //result *= mix(vec3(1.0), vec3(0.2, 0.5, 1.0), 0.5*normalize(r_in.direction).y+0.5);
            //result *= 0;
            break;
        }
    }
    return result;
}
void main()
{
    const float time_arg = 2*degrees(time);
    const float time_var = (0.5*sin(2*degrees(time))+0.5);
    const float time_neg = sin(time_arg);
    ray r = project_ray_perspective(cam_space, 1.0);

    sphere my_s1 = sphere(vec3(1, 0.2, -4), 0.8, vec3(1), false);
    sphere my_s2 = sphere(vec3(-0.5, 0.0, -2.0), 0.8, vec3(1,0,0), false);
    my_s1.center += 2*vec3(0, time_neg,0);
    sphere s[2] = {my_s2, my_s1};

    frag_output =  vec4(trace(r, s, 1), 1.0);
}