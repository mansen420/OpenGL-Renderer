layout (location = 0) in vec3     vertexPos;
layout (location = 1) in vec3 vertexNormals;
layout (location = 2) in vec2     vertexTex;

uniform mat4 view_transform, projection_transform, model_transform, lightspace_transform;

out vec3  frag_pos;
out vec3    normal;
out vec2 tex_coords;
out vec4 lightspace_frag_pos;