#version 450

// Scene geometry vertex shader (M6)
// Compile with: glslc scene.vert -o scene.vert.spv

layout(push_constant) uniform PushConstants {
    mat4 view_proj;
} pc;

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

// Per-instance transform (mat4 as 4 × vec4)
layout(location = 3) in vec4 i_model_0;
layout(location = 4) in vec4 i_model_1;
layout(location = 5) in vec4 i_model_2;
layout(location = 6) in vec4 i_model_3;

layout(location = 0) out vec3 v_world_pos;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec2 v_uv;

void main() {
    mat4 model = mat4(i_model_0, i_model_1, i_model_2, i_model_3);
    vec4 world = model * vec4(a_position, 1.0);
    v_world_pos = world.xyz;
    v_normal    = mat3(model) * a_normal;
    v_uv        = a_uv;
    gl_Position = pc.view_proj * world;
}
