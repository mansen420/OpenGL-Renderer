in vec4 lightspace_frag_pos;
uniform sampler2D shadow_map;
float calculate_shadow(vec4 lightspace_pos);