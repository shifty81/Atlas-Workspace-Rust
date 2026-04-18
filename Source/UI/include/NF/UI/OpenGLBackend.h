#pragma once
// NF::OpenGLBackend — GPU-accelerated COMPATIBILITY rendering backend.
//
// ⚠️  COMPAT ONLY: This backend is not the canonical render path.
//     Primary target: D3D11 + DirectWrite (Windows, GPU-first).
//     Fallback:       GDI   (Windows bootstrap/recovery only).
//
// OpenGLBackend exists for cross-platform bring-up and compatibility
// testing. It is not included in the default Atlas build and is not
// a supported production target.
//
// Include via: NF/UI/Compat/CompatBackends.h
// Requires:    OpenGL headers + a valid GL context (GLFW or platform).
#include "NF/UI/UIBackend.h"
#include "NF/UI/IUIBackendInterfaces.h"

namespace NF {

// ── OpenGL shader sources (embedded) ─────────────────────────────

namespace GLShaders {

inline constexpr const char* kUI_VertexShader = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec4 aColor;

uniform mat4 uProjection;

out vec2 vUV;
out vec4 vColor;

void main() {
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
    vUV    = aUV;
    vColor = aColor;
}
)";

inline constexpr const char* kUI_FragmentShader = R"(
#version 330 core
in vec2 vUV;
in vec4 vColor;

uniform sampler2D uFontAtlas;
uniform bool uUseTexture;

out vec4 FragColor;

void main() {
    if (uUseTexture) {
        float alpha = texture(uFontAtlas, vUV).r;
        FragColor = vec4(vColor.rgb, vColor.a * alpha);
    } else {
        FragColor = vColor;
    }
}
)";

inline constexpr const char* kGrid_VertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 uViewProjection;

out vec3 vWorldPos;

void main() {
    gl_Position = uViewProjection * vec4(aPos, 1.0);
    vWorldPos = aPos;
}
)";

inline constexpr const char* kGrid_FragmentShader = R"(
#version 330 core
in vec3 vWorldPos;

uniform vec3 uGridColor;
uniform float uGridSpacing;

out vec4 FragColor;

void main() {
    vec2 grid = abs(fract(vWorldPos.xz / uGridSpacing - 0.5) - 0.5) / fwidth(vWorldPos.xz / uGridSpacing);
    float line = min(grid.x, grid.y);
    float alpha = 1.0 - min(line, 1.0);
    FragColor = vec4(uGridColor, alpha * 0.5);
}
)";

} // namespace GLShaders

// ── OpenGLBackend ────────────────────────────────────────────────
// Compat stub implementation.  Implements IFrameBackend and IGeometryBackend.
// When a real OpenGL context is available, this will compile shaders,
// create VAO/VBO/EBO, bake a font atlas, and render UI quads via glDrawElements.

class OpenGLBackend final : public UIBackend,
                            public IFrameBackend,
                            public IGeometryBackend {
public:
    bool init(int width, int height) override {
        m_width  = width;
        m_height = height;
        m_initialized = true;
        NF_LOG_INFO("UI", "OpenGLBackend initialized (stub) " +
                    std::to_string(width) + "x" + std::to_string(height));
        // Real implementation would:
        //   1. Compile kUI_VertexShader + kUI_FragmentShader
        //   2. Create VAO, VBO (dynamic), EBO (dynamic)
        //   3. Bake a font atlas (stb_truetype → GL texture)
        //   4. Set up orthographic projection
        return true;
    }

    void shutdown() override {
        m_initialized = false;
        // Real implementation: delete GL resources (VAO, VBO, EBO, textures, shaders)
        NF_LOG_INFO("UI", "OpenGLBackend shutdown");
    }

    void beginFrame(int width, int height) override {
        m_width  = width;
        m_height = height;
        // Real implementation:
        //   glViewport(0, 0, width, height);
        //   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //   Update ortho projection uniform
    }

    void flush(const UIVertex* vertices, size_t vertexCount,
               const uint32_t* indices, size_t indexCount) override {
        if (!m_initialized || vertexCount == 0) return;
        m_lastVertexCount = vertexCount;
        m_lastIndexCount  = indexCount;
        // Real implementation:
        //   glBindVertexArray(m_vao);
        //   glBufferSubData(GL_ARRAY_BUFFER, 0, vertexCount * sizeof(UIVertex), vertices);
        //   glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indexCount * sizeof(uint32_t), indices);
        //   glEnable(GL_BLEND);
        //   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //   glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
        (void)vertices; (void)indices;
    }

    void endFrame() override {
        // Real implementation: SwapBuffers(hdc) or glfwSwapBuffers(window)
    }

    void loadFont(const std::string& fontName, float fontSize) override {
        m_fontName = fontName;
        m_fontSize = fontSize;
        // Real implementation:
        //   Load TTF from disk or embedded data
        //   stbtt_BakeFontBitmap → m_fontAtlasBitmap
        //   glGenTextures(1, &m_fontAtlasTexture)
        //   glTexImage2D(GL_TEXTURE_2D, ..., m_fontAtlasBitmap, ...)
    }

    [[nodiscard]] const char* backendName() const override { return "OpenGL"; }
    [[nodiscard]] bool isGPUAccelerated() const override { return true; }

    // Stats
    [[nodiscard]] size_t lastVertexCount() const { return m_lastVertexCount; }
    [[nodiscard]] size_t lastIndexCount()  const { return m_lastIndexCount; }

    // Viewport 3D rendering stub
    struct Viewport3DParams {
        Rect bounds;
        Mat4 viewMatrix;
        Mat4 projectionMatrix;
        uint32_t gridColor = 0x333333FF;
        bool showGrid = true;
    };

    void render3DViewport(const Viewport3DParams& params) {
        (void)params;
        // Real implementation:
        //   glViewport(params.bounds.x, params.bounds.y, params.bounds.w, params.bounds.h);
        //   glClear(GL_DEPTH_BUFFER_BIT);
        //   Bind grid shader, set uViewProjection, draw grid mesh
        //   Render queued meshes through forward pipeline
    }

private:
    int m_width  = 0;
    int m_height = 0;
    bool m_initialized = false;
    std::string m_fontName = "Consolas";
    float m_fontSize = 14.f;
    size_t m_lastVertexCount = 0;
    size_t m_lastIndexCount  = 0;

    // GPU handles (populated when real GL context available):
    // uint32_t m_vao = 0, m_vbo = 0, m_ebo = 0;
    // uint32_t m_shaderProgram = 0;
    // uint32_t m_fontAtlasTexture = 0;
    // uint32_t m_gridShaderProgram = 0;
    // uint32_t m_gridVao = 0;
};

} // namespace NF
