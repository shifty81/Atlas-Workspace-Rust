#version 450

// Scene geometry fragment shader (M6)
// Compile with: glslc scene.frag -o scene.frag.spv

layout(location = 0) in vec3 v_world_pos;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;

layout(location = 0) out vec4 out_color;

// Simple Blinn-Phong until a PBR pass is added in M6
void main() {
    vec3 light_dir = normalize(vec3(0.5, 1.0, 0.3));
    vec3 n         = normalize(v_normal);
    float diffuse  = max(dot(n, light_dir), 0.0);
    float ambient  = 0.15;
    float c        = clamp(ambient + diffuse, 0.0, 1.0);
    out_color = vec4(vec3(c), 1.0);
}
