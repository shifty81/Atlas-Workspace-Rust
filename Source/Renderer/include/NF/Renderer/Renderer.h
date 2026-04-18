#pragma once
// NF::Renderer — RHI (OpenGL), forward pipeline, mesh, materials
#include "NF/Core/Core.h"

namespace NF {

// ── Scene Render Mode ────────────────────────────────────────────
// Per-slot render mode propagated from ViewportRenderMode down to the
// Renderer so shader permutations / rasterizer state can be selected.

enum class SceneRenderMode : uint8_t {
    Lit,       // PBR / standard lighting (default)
    Unlit,     // full albedo, no lighting
    Wireframe, // geometry outline only
    Normals,   // visualise vertex normals as colour
    Overdraw,  // visualise overdraw cost (additive blend)
};

inline const char* sceneRenderModeName(SceneRenderMode m) {
    switch (m) {
    case SceneRenderMode::Lit:       return "Lit";
    case SceneRenderMode::Unlit:     return "Unlit";
    case SceneRenderMode::Wireframe: return "Wireframe";
    case SceneRenderMode::Normals:   return "Normals";
    case SceneRenderMode::Overdraw:  return "Overdraw";
    }
    return "Unknown";
}

// ── Vertex Data ──────────────────────────────────────────────────

struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec2 texCoord;
    Vec4 color{1.f, 1.f, 1.f, 1.f};
};

// ── Mesh ─────────────────────────────────────────────────────────

class Mesh {
public:
    Mesh() = default;
    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
        : m_vertices(std::move(vertices)), m_indices(std::move(indices)) {}

    void setVertices(std::vector<Vertex> verts) { m_vertices = std::move(verts); m_dirty = true; }
    void setIndices(std::vector<uint32_t> idx) { m_indices = std::move(idx); m_dirty = true; }

    [[nodiscard]] const std::vector<Vertex>& vertices() const { return m_vertices; }
    [[nodiscard]] const std::vector<uint32_t>& indices() const { return m_indices; }
    [[nodiscard]] size_t vertexCount() const { return m_vertices.size(); }
    [[nodiscard]] size_t indexCount() const { return m_indices.size(); }
    [[nodiscard]] size_t triangleCount() const { return m_indices.size() / 3; }
    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void clearDirty() { m_dirty = false; }

    // Generate AABB from vertices
    [[nodiscard]] std::pair<Vec3, Vec3> bounds() const {
        if (m_vertices.empty()) return {{}, {}};
        Vec3 lo = m_vertices[0].position;
        Vec3 hi = m_vertices[0].position;
        for (auto& v : m_vertices) {
            lo.x = std::min(lo.x, v.position.x);
            lo.y = std::min(lo.y, v.position.y);
            lo.z = std::min(lo.z, v.position.z);
            hi.x = std::max(hi.x, v.position.x);
            hi.y = std::max(hi.y, v.position.y);
            hi.z = std::max(hi.z, v.position.z);
        }
        return {lo, hi};
    }

private:
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    bool m_dirty = true;
};

// ── Shader ───────────────────────────────────────────────────────

enum class ShaderStage : uint8_t {
    Vertex,
    Fragment,
    Geometry,
    Compute
};

class Shader {
public:
    Shader() = default;
    Shader(StringID name, const std::string& vertSrc, const std::string& fragSrc)
        : m_name(name), m_vertexSource(vertSrc), m_fragmentSource(fragSrc) {}

    [[nodiscard]] StringID name() const { return m_name; }
    [[nodiscard]] const std::string& vertexSource() const { return m_vertexSource; }
    [[nodiscard]] const std::string& fragmentSource() const { return m_fragmentSource; }
    [[nodiscard]] bool isCompiled() const { return m_compiled; }

    bool compile() {
        // Placeholder: real implementation would compile via OpenGL API
        m_compiled = !m_vertexSource.empty() && !m_fragmentSource.empty();
        return m_compiled;
    }

    void setUniformFloat(const std::string& name, float value) { m_floatUniforms[name] = value; }
    void setUniformVec3(const std::string& name, const Vec3& value) { m_vec3Uniforms[name] = value; }
    void setUniformMat4(const std::string& name, const Mat4& value) { m_mat4Uniforms[name] = value; }

    [[nodiscard]] const std::unordered_map<std::string, float>& floatUniforms() const { return m_floatUniforms; }
    [[nodiscard]] const std::unordered_map<std::string, Vec3>& vec3Uniforms() const { return m_vec3Uniforms; }

private:
    StringID m_name;
    std::string m_vertexSource;
    std::string m_fragmentSource;
    bool m_compiled = false;
    std::unordered_map<std::string, float> m_floatUniforms;
    std::unordered_map<std::string, Vec3> m_vec3Uniforms;
    std::unordered_map<std::string, Mat4> m_mat4Uniforms;
};

// ── Material ─────────────────────────────────────────────────────

class Material {
public:
    Material() = default;
    explicit Material(StringID name) : m_name(name) {}

    void setShader(Shader* shader) { m_shader = shader; }
    void setColor(const Vec4& color) { m_color = color; }
    void setMetallic(float v) { m_metallic = v; }
    void setRoughness(float v) { m_roughness = v; }

    [[nodiscard]] StringID name() const { return m_name; }
    [[nodiscard]] Shader* shader() const { return m_shader; }
    [[nodiscard]] const Vec4& color() const { return m_color; }
    [[nodiscard]] float metallic() const { return m_metallic; }
    [[nodiscard]] float roughness() const { return m_roughness; }

private:
    StringID m_name;
    Shader* m_shader = nullptr;
    Vec4 m_color{1.f, 1.f, 1.f, 1.f};
    float m_metallic = 0.f;
    float m_roughness = 0.5f;
};

// ── Render Command ───────────────────────────────────────────────

struct RenderCommand {
    const Mesh* mesh = nullptr;
    const Material* material = nullptr;
    Mat4 transform = Mat4::identity();
    float sortKey = 0.f;
    SceneRenderMode renderMode = SceneRenderMode::Lit; // per-command render mode
};

// ── Render Queue ─────────────────────────────────────────────────

class RenderQueue {
public:
    void submit(RenderCommand cmd) {
        m_commands.push_back(std::move(cmd));
    }

    void sort() {
        std::sort(m_commands.begin(), m_commands.end(),
                  [](const RenderCommand& a, const RenderCommand& b) {
                      return a.sortKey < b.sortKey;
                  });
    }

    void clear() { m_commands.clear(); }

    [[nodiscard]] const std::vector<RenderCommand>& commands() const { return m_commands; }
    [[nodiscard]] size_t size() const { return m_commands.size(); }
    [[nodiscard]] bool empty() const { return m_commands.empty(); }

private:
    std::vector<RenderCommand> m_commands;
};

// ── Camera ───────────────────────────────────────────────────────

struct Camera {
    Vec3 position{0.f, 0.f, 5.f};
    Vec3 target{0.f, 0.f, 0.f};
    Vec3 up{0.f, 1.f, 0.f};
    float fov = 60.f;
    float nearPlane = 0.1f;
    float farPlane = 1000.f;

    [[nodiscard]] Mat4 viewMatrix() const {
        return Mat4::lookAt(position, target, up);
    }

    [[nodiscard]] Mat4 projectionMatrix(float aspect) const {
        constexpr float DEG2RAD = 3.14159265358979f / 180.f;
        return Mat4::perspective(fov * DEG2RAD, aspect, nearPlane, farPlane);
    }

    [[nodiscard]] Vec3 forward() const { return (target - position).normalized(); }
};

// ── Renderer ─────────────────────────────────────────────────────

class Renderer {
public:
    bool init(int width, int height) {
        m_width = width;
        m_height = height;
        NF_LOG_INFO("Renderer", "Renderer initialized");
        return true;
    }

    void shutdown() {
        m_queue.clear();
        NF_LOG_INFO("Renderer", "Renderer shutdown");
    }

    void beginFrame() {
        m_queue.clear();
        m_drawCallCount = 0;
    }

    void submit(RenderCommand cmd) { m_queue.submit(std::move(cmd)); }

    void endFrame() {
        m_queue.sort();
        m_drawCallCount = m_queue.size();
        // Placeholder: real implementation would issue OpenGL draw calls
    }

    void setCamera(const Camera& cam) { m_camera = cam; }
    [[nodiscard]] const Camera& camera() const { return m_camera; }

    // Per-frame render mode — applied to all submitted commands that do not
    // specify their own mode (i.e. commands that retain the default Lit mode).
    void setRenderMode(SceneRenderMode mode) { m_renderMode = mode; }
    [[nodiscard]] SceneRenderMode renderMode() const { return m_renderMode; }

    [[nodiscard]] int width() const { return m_width; }
    [[nodiscard]] int height() const { return m_height; }
    [[nodiscard]] float aspect() const {
        return m_height > 0 ? static_cast<float>(m_width) / static_cast<float>(m_height) : 1.f;
    }
    [[nodiscard]] size_t drawCallCount() const { return m_drawCallCount; }

private:
    int m_width = 0;
    int m_height = 0;
    Camera m_camera;
    RenderQueue m_queue;
    size_t m_drawCallCount = 0;
    SceneRenderMode m_renderMode = SceneRenderMode::Lit;
};

// ── Light Types (G2) ────────────────────────────────────────────

enum class LightType : uint8_t { Directional, Point, Spot };

struct LightSource {
    LightType type = LightType::Directional;
    Vec3 position{0, 10, 0};
    Vec3 direction{0, -1, 0};
    Vec3 color{1, 1, 1};
    float intensity = 1.f;
    float range = 50.f;
    float spotAngle = 45.f;
    bool enabled = true;
};

// ── Lighting State (Phong model) ────────────────────────────────

class LightingState {
public:
    static constexpr int MAX_LIGHTS = 8;

    void addLight(const LightSource& light) {
        if (static_cast<int>(m_lights.size()) < MAX_LIGHTS)
            m_lights.push_back(light);
    }

    void removeLight(int index) {
        if (index >= 0 && index < static_cast<int>(m_lights.size()))
            m_lights.erase(m_lights.begin() + index);
    }

    void clear() { m_lights.clear(); }

    LightSource& light(int index) { return m_lights[index]; }
    const LightSource& light(int index) const { return m_lights[index]; }

    int lightCount() const { return static_cast<int>(m_lights.size()); }

    void setAmbientColor(const Vec3& color) { m_ambientColor = color; }
    void setAmbientIntensity(float intensity) { m_ambientIntensity = intensity; }
    Vec3 ambientColor() const { return m_ambientColor; }
    float ambientIntensity() const { return m_ambientIntensity; }

    Vec3 computeLighting(const Vec3& surfacePos, const Vec3& surfaceNormal,
                         const Vec3& viewPos) const {
        Vec3 result = m_ambientColor * m_ambientIntensity;
        constexpr float shininess = 32.f;

        for (const auto& lt : m_lights) {
            if (!lt.enabled) continue;

            Vec3 L;
            float atten = 1.f;

            if (lt.type == LightType::Directional) {
                L = (lt.direction * -1.f).normalized();
            } else {
                Vec3 toLight = lt.position - surfacePos;
                float dist = toLight.length();
                if (dist < 1e-6f) continue;
                L = toLight * (1.f / dist);
                float dNorm = dist / lt.range;
                atten = std::max(0.f, 1.f - dNorm * dNorm);
            }

            Vec3 N = surfaceNormal.normalized();
            float NdotL = std::max(0.f, N.dot(L));
            Vec3 diffuse = lt.color * (NdotL * lt.intensity * atten);

            Vec3 specular{0.f, 0.f, 0.f};
            if (NdotL > 0.f) {
                Vec3 V = (viewPos - surfacePos).normalized();
                Vec3 R = N * (2.f * N.dot(L)) - L;
                float spec = std::pow(std::max(0.f, R.dot(V)), shininess);
                specular = lt.color * (spec * lt.intensity * atten);
            }

            result = result + diffuse + specular;
        }

        result.x = std::min(1.f, std::max(0.f, result.x));
        result.y = std::min(1.f, std::max(0.f, result.y));
        result.z = std::min(1.f, std::max(0.f, result.z));
        return result;
    }

private:
    std::vector<LightSource> m_lights;
    Vec3 m_ambientColor{0.1f, 0.1f, 0.15f};
    float m_ambientIntensity = 0.3f;
};

// ── Voxel Shader (G2) ──────────────────────────────────────────

class VoxelShader {
public:
    void init() {
        m_shader = Shader(StringID("voxel_shader"),
            "// voxel vertex shader\n"
            "uniform mat4 u_viewProjection;\n"
            "uniform mat4 u_model;\n",
            "// voxel fragment shader\n"
            "uniform vec3 u_lightDir;\n"
            "uniform vec3 u_lightColor;\n"
            "uniform vec3 u_ambient;\n"
            "uniform vec3 u_viewPos;\n");
        m_shader.compile();
        m_ready = true;
    }

    bool isReady() const { return m_ready; }

    Shader& shader() { return m_shader; }
    const Shader& shader() const { return m_shader; }

    void setViewProjection(const Mat4& vp) { m_shader.setUniformMat4("u_viewProjection", vp); }
    void setModel(const Mat4& model) { m_shader.setUniformMat4("u_model", model); }
    void setLightDir(const Vec3& dir) { m_shader.setUniformVec3("u_lightDir", dir); }
    void setLightColor(const Vec3& color) { m_shader.setUniformVec3("u_lightColor", color); }
    void setAmbient(const Vec3& ambient) { m_shader.setUniformVec3("u_ambient", ambient); }
    void setViewPos(const Vec3& pos) { m_shader.setUniformVec3("u_viewPos", pos); }

private:
    Shader m_shader;
    bool m_ready = false;
};

// ── Frustum Culling (G2) ────────────────────────────────────────

struct FrustumPlane {
    Vec3 normal;
    float distance = 0.f;

    float distanceToPoint(const Vec3& point) const {
        return normal.dot(point) + distance;
    }
};

class Frustum {
public:
    void extractFromVP(const Mat4& vp) {
        // Griggs/Hartmann method: extract planes from VP matrix rows
        // Column-major: row i of the matrix is m[i], m[4+i], m[8+i], m[12+i]
        auto row = [&](int i) -> Vec4 {
            return {vp.m[i], vp.m[4 + i], vp.m[8 + i], vp.m[12 + i]};
        };

        Vec4 r0 = row(0), r1 = row(1), r2 = row(2), r3 = row(3);

        auto setPlane = [](FrustumPlane& p, Vec4 v) {
            float len = Vec3{v.x, v.y, v.z}.length();
            if (len > 1e-7f) {
                float inv = 1.f / len;
                p.normal = {v.x * inv, v.y * inv, v.z * inv};
                p.distance = v.w * inv;
            }
        };

        setPlane(m_planes[0], {r3.x + r0.x, r3.y + r0.y, r3.z + r0.z, r3.w + r0.w}); // left
        setPlane(m_planes[1], {r3.x - r0.x, r3.y - r0.y, r3.z - r0.z, r3.w - r0.w}); // right
        setPlane(m_planes[2], {r3.x + r1.x, r3.y + r1.y, r3.z + r1.z, r3.w + r1.w}); // bottom
        setPlane(m_planes[3], {r3.x - r1.x, r3.y - r1.y, r3.z - r1.z, r3.w - r1.w}); // top
        setPlane(m_planes[4], {r3.x + r2.x, r3.y + r2.y, r3.z + r2.z, r3.w + r2.w}); // near
        setPlane(m_planes[5], {r3.x - r2.x, r3.y - r2.y, r3.z - r2.z, r3.w - r2.w}); // far
    }

    bool testAABB(const Vec3& min, const Vec3& max) const {
        for (int i = 0; i < 6; ++i) {
            // Find the positive vertex (most in the direction of the normal)
            Vec3 pv;
            pv.x = (m_planes[i].normal.x >= 0.f) ? max.x : min.x;
            pv.y = (m_planes[i].normal.y >= 0.f) ? max.y : min.y;
            pv.z = (m_planes[i].normal.z >= 0.f) ? max.z : min.z;

            if (m_planes[i].distanceToPoint(pv) < 0.f)
                return false;
        }
        return true;
    }

    const FrustumPlane& plane(int index) const { return m_planes[index]; }

private:
    FrustumPlane m_planes[6];
};

} // namespace NF
