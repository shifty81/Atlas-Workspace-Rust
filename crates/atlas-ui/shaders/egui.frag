#version 450

// Egui fragment shader (M4)
// Compile with: glslc egui.frag -o egui.frag.spv

layout(location = 0) in vec2 v_uv;
layout(location = 1) in vec4 v_color;

layout(set = 0, binding = 0) uniform sampler2D u_texture;

layout(location = 0) out vec4 out_color;

void main() {
    out_color = v_color * texture(u_texture, v_uv);
}
