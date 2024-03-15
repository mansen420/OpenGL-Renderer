#version 450 core
out vec4 frag_output;
in vec2 texture_coordinates;
in vec2 pos_coords;


struct sphere
{
    vec3 center;
    float radius;
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
vec3 trace(in ray r, in sphere[2] s, in int depth)
{
    float t_min = 0, t_max = 10.0;

    vec3 result = vec3(1.0);
    int count = 0;
    ray r_in = r;
    while(count++ < depth)
    {
        for(int i = 0; i < 2; ++i)
        {
            bool intersect;
            float root = ray_sphere_intersection_root(r_in, s[i], t_min, t_max, intersect);
            if(intersect)
            {
                vec3 P = extend_ray(r_in, root);
                vec3 view_vec = cam_space.origin - P;
                vec3 normal = P - s[i].center;
                vec3 reflected_vec = normalize(reflect(view_vec, normal));
                r_in = ray(r_in.origin, normalize( vec3(1.0, 1.0, 0.0) - P));
                result = 0.5*normalize(normal)+0.5;
            }
        }
    }
    return result;
}
void main()
{
    ray r = project_ray_perspective(cam_space, 1.0);
    sphere my_s1 = sphere(vec3(0.5, 0, -1.5), 0.8);
    sphere my_s2 = sphere(vec3(0, 0, -2), 0.8);
    sphere s[2] = {my_s2, my_s1};
    frag_output = vec4(trace(r, s, 10), 1.0);
}