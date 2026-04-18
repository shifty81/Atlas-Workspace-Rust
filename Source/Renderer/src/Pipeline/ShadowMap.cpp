#include "ShadowMap.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace atlas::render {

static constexpr float PI = 3.14159265358979323846f;

ShadowMapConfig ShadowMapConfig::Default() {
    ShadowMapConfig c;
    c.cascadeCount = 3;
    c.baseResolution = 1024;
    c.bias = 0.005f;
    c.normalBias = 0.02f;
    c.softShadows = true;
    c.pcfSamples = 4;
    return c;
}

void ShadowMap::Init(const ShadowMapConfig& config) {
    if (m_initialized) {
        Shutdown();
    }

    m_config = config;
    m_config.cascadeCount = std::max(1u, std::min(config.cascadeCount, 4u));

    m_cascades.resize(m_config.cascadeCount);
    m_nextId = 1;
    for (auto& c : m_cascades) {
        std::memset(c.viewMatrix, 0, sizeof(c.viewMatrix));
        std::memset(c.projMatrix, 0, sizeof(c.projMatrix));
        // Identity diagonal
        c.viewMatrix[0] = c.viewMatrix[5] = c.viewMatrix[10] = c.viewMatrix[15] = 1.0f;
        c.projMatrix[0] = c.projMatrix[5] = c.projMatrix[10] = c.projMatrix[15] = 1.0f;
        c.resolution = m_config.baseResolution;
        c.id = m_nextId++;
    }

    m_lightDir = {0.0f, -1.0f, 0.0f};
    m_initialized = true;
    m_bound = false;
}

void ShadowMap::Shutdown() {
    m_cascades.clear();
    m_initialized = false;
    m_bound = false;
}

void ShadowMap::SetLightDirection(float x, float y, float z) {
    float len = std::sqrt(x * x + y * y + z * z);
    if (len > 1e-6f) {
        m_lightDir.x = x / len;
        m_lightDir.y = y / len;
        m_lightDir.z = z / len;
    }
}

void ShadowMap::UpdateCascades(float cameraNear, float cameraFar,
                               float cameraFovDeg, float aspectRatio) {
    if (!m_initialized || m_cascades.empty()) return;

    const float lambda = 0.5f;
    uint32_t n = static_cast<uint32_t>(m_cascades.size());

    // Calculate cascade split distances using practical split scheme
    std::vector<float> splits(n + 1);
    splits[0] = cameraNear;
    splits[n] = cameraFar;

    for (uint32_t i = 1; i < n; ++i) {
        float p = static_cast<float>(i) / static_cast<float>(n);
        float logSplit = cameraNear * std::pow(cameraFar / cameraNear, p);
        float uniformSplit = cameraNear + (cameraFar - cameraNear) * p;
        splits[i] = lambda * logSplit + (1.0f - lambda) * uniformSplit;
    }

    // Compute half-size of the frustum at the far plane of each cascade
    float tanHalfFov = std::tan((cameraFovDeg * PI / 180.0f) * 0.5f);

    for (uint32_t i = 0; i < n; ++i) {
        m_cascades[i].nearPlane = splits[i];
        m_cascades[i].farPlane = splits[i + 1];

        // Frustum bounding sphere radius for this cascade
        float cascadeFar = splits[i + 1];
        float halfH = cascadeFar * tanHalfFov;
        float halfW = halfH * aspectRatio;
        float radius = std::sqrt(halfW * halfW + halfH * halfH + cascadeFar * cascadeFar);

        // Decrease resolution for farther cascades
        m_cascades[i].resolution = std::max(256u, m_config.baseResolution >> i);

        // Build view matrix looking along light direction
        float eyeX = -m_lightDir.x * radius;
        float eyeY = -m_lightDir.y * radius;
        float eyeZ = -m_lightDir.z * radius;
        BuildLookAtMatrix(m_cascades[i].viewMatrix,
                          eyeX, eyeY, eyeZ,
                          0.0f, 0.0f, 0.0f,
                          0.0f, 1.0f, 0.0f);

        // Build orthographic projection matrix
        BuildOrthoProjMatrix(m_cascades[i].projMatrix,
                             -radius, radius, -radius, radius,
                             0.0f, radius * 2.0f);
    }
}

const ShadowCascade* ShadowMap::GetCascade(uint32_t index) const {
    if (index < m_cascades.size()) {
        return &m_cascades[index];
    }
    return nullptr;
}

uint32_t ShadowMap::CascadeCount() const {
    return static_cast<uint32_t>(m_cascades.size());
}

void ShadowMap::BindForWriting(uint32_t cascadeIndex) {
    if (m_initialized && cascadeIndex < m_cascades.size()) {
        m_bound = true;
        m_boundCascade = cascadeIndex;
    }
}

void ShadowMap::BindForReading() {
    if (m_initialized) {
        m_bound = true;
    }
}

void ShadowMap::Unbind() {
    m_bound = false;
}

void ShadowMap::BuildOrthoProjMatrix(float* out, float left, float right,
                                     float bottom, float top,
                                     float nearVal, float farVal) const {
    std::memset(out, 0, 16 * sizeof(float));
    out[0]  =  2.0f / (right - left);
    out[5]  =  2.0f / (top - bottom);
    out[10] = -2.0f / (farVal - nearVal);
    out[12] = -(right + left) / (right - left);
    out[13] = -(top + bottom) / (top - bottom);
    out[14] = -(farVal + nearVal) / (farVal - nearVal);
    out[15] =  1.0f;
}

void ShadowMap::BuildLookAtMatrix(float* out,
                                  float eyeX, float eyeY, float eyeZ,
                                  float centerX, float centerY, float centerZ,
                                  float upX, float upY, float upZ) const {
    // Forward direction (from eye to center)
    float fX = centerX - eyeX;
    float fY = centerY - eyeY;
    float fZ = centerZ - eyeZ;
    float fLen = std::sqrt(fX * fX + fY * fY + fZ * fZ);
    if (fLen > 1e-6f) { fX /= fLen; fY /= fLen; fZ /= fLen; }

    // Right = forward × up
    float rX = fY * upZ - fZ * upY;
    float rY = fZ * upX - fX * upZ;
    float rZ = fX * upY - fY * upX;
    float rLen = std::sqrt(rX * rX + rY * rY + rZ * rZ);
    if (rLen > 1e-6f) { rX /= rLen; rY /= rLen; rZ /= rLen; }

    // Recomputed up = right × forward
    float uX = rY * fZ - rZ * fY;
    float uY = rZ * fX - rX * fZ;
    float uZ = rX * fY - rY * fX;

    // Column-major 4×4 matrix
    std::memset(out, 0, 16 * sizeof(float));
    out[0]  =  rX;  out[1]  =  uX;  out[2]  = -fX;
    out[4]  =  rY;  out[5]  =  uY;  out[6]  = -fY;
    out[8]  =  rZ;  out[9]  =  uZ;  out[10] = -fZ;
    out[12] = -(rX * eyeX + rY * eyeY + rZ * eyeZ);
    out[13] = -(uX * eyeX + uY * eyeY + uZ * eyeZ);
    out[14] =  (fX * eyeX + fY * eyeY + fZ * eyeZ);
    out[15] =  1.0f;
}

} // namespace atlas::render
