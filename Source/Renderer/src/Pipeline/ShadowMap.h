#pragma once
#include <cstdint>
#include <vector>

namespace atlas::render {

/// A single cascade in a cascaded shadow map.
struct ShadowCascade {
    float nearPlane = 0.0f;
    float farPlane = 0.0f;
    uint32_t resolution = 0;
    float viewMatrix[16] = {};
    float projMatrix[16] = {};
    uint32_t id = 0;
};

/// Direction vector for a directional light.
struct LightDirection {
    float x = 0.0f;
    float y = -1.0f;
    float z = 0.0f;
};

/// Configuration for creating a shadow map.
struct ShadowMapConfig {
    uint32_t cascadeCount = 3;
    uint32_t baseResolution = 1024;
    float bias = 0.005f;
    float normalBias = 0.02f;
    bool softShadows = true;
    uint32_t pcfSamples = 4;

    /// Default configuration: 3 cascades, 1024 resolution, 0.005 bias.
    static ShadowMapConfig Default();
};

/// Cascaded shadow map for directional lights.
/// Manages cascade splits, view/projection matrices, and bind state.
class ShadowMap {
public:
    void Init(const ShadowMapConfig& config);
    void Shutdown();

    void SetLightDirection(float x, float y, float z);
    LightDirection GetLightDirection() const { return m_lightDir; }

    /// Recalculate cascade splits using practical split scheme (λ=0.5).
    void UpdateCascades(float cameraNear, float cameraFar,
                        float cameraFovDeg, float aspectRatio);

    const ShadowCascade* GetCascade(uint32_t index) const;
    uint32_t CascadeCount() const;
    const ShadowMapConfig& GetConfig() const { return m_config; }

    void BindForWriting(uint32_t cascadeIndex);
    void BindForReading();
    void Unbind();

    bool IsInitialized() const { return m_initialized; }
    bool IsBound() const { return m_bound; }

private:
    ShadowMapConfig m_config;
    LightDirection m_lightDir;
    std::vector<ShadowCascade> m_cascades;
    bool m_initialized = false;
    bool m_bound = false;
    uint32_t m_boundCascade = 0;
    uint32_t m_nextId = 1;

    void BuildOrthoProjMatrix(float* out, float left, float right,
                              float bottom, float top,
                              float nearVal, float farVal) const;
    void BuildLookAtMatrix(float* out,
                           float eyeX, float eyeY, float eyeZ,
                           float centerX, float centerY, float centerZ,
                           float upX, float upY, float upZ) const;
};

} // namespace atlas::render
