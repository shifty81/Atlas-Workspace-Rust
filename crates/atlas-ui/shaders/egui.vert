#version 450

// Egui vertex shader (M4)
// Compile with: glslc egui.vert -o egui.vert.spv

layout(push_constant) uniform PushConstants {
    vec2 screen_size;
} pc;

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec4 a_color; // sRGB RGBA u8 normalised

layout(location = 0) out vec2 v_uv;
layout(location = 1) out vec4 v_color;

// Converts sRGB u8 normalised [0..1] to linear
vec4 srgb_to_linear(vec4 c) {
    vec3 low  = c.rgb / 12.92;
    vec3 high = pow((c.rgb + 0.055) / 1.055, vec3(2.4));
    vec3 lin  = mix(low, high, step(0.04045, c.rgb));
    return vec4(lin, c.a);
}

void main() {
    gl_Position = vec4(
        2.0 * a_pos.x / pc.screen_size.x - 1.0,
        2.0 * a_pos.y / pc.screen_size.y - 1.0,
        0.0,
        1.0
    );
    v_uv    = a_uv;
    v_color = srgb_to_linear(a_color);
}
