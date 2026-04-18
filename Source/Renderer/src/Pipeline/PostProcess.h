#pragma once
#include <cstdint>

namespace atlas::render {

/// Available post-processing effects.
enum class PostProcessEffect : uint8_t {
    Bloom,
    ToneMapping,
    FXAA,
    Vignette,
    ChromaticAberration,
    FilmGrain
};

/// Tone-mapping operators.
enum class ToneMapOperator : uint8_t {
    Reinhard,
    ACES,
    Uncharted2,
    Filmic
};

/// Bloom effect configuration.
struct BloomSettings {
    float threshold = 1.0f;
    float intensity = 1.0f;
    float radius = 0.005f;
    uint32_t mipCount = 5;
    bool enabled = true;

    static BloomSettings Default();
};

/// Tone-mapping configuration.
struct ToneMappingSettings {
    ToneMapOperator op = ToneMapOperator::ACES;
    float exposure = 1.0f;
    float gamma = 2.2f;
    float whitePoint = 4.0f;
    bool enabled = true;

    static ToneMappingSettings Default();
};

/// Aggregate post-processing settings.
struct PostProcessSettings {
    BloomSettings bloom;
    ToneMappingSettings toneMapping;
    bool fxaaEnabled = true;
    float vignetteIntensity = 0.0f;
    float chromaticAberrationIntensity = 0.0f;
    float filmGrainIntensity = 0.0f;

    static PostProcessSettings Default();
};

/// Post-processing pipeline that executes a chain of screen-space effects.
class PostProcessPipeline {
public:
    void Init(uint32_t width, uint32_t height);
    void Shutdown();

    void Resize(uint32_t width, uint32_t height);

    void SetSettings(const PostProcessSettings& settings);
    const PostProcessSettings& GetSettings() const { return m_settings; }

    bool IsEffectEnabled(PostProcessEffect effect) const;
    void SetEffectEnabled(PostProcessEffect effect, bool enabled);

    /// Execute the full post-processing chain.
    void Execute();

    uint32_t GetOutputTextureId() const { return m_outputTextureId; }
    bool IsInitialized() const { return m_initialized; }

    /// Number of currently active effects.
    uint32_t EffectCount() const;

private:
    PostProcessSettings m_settings;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_outputTextureId = 0;
    bool m_initialized = false;
    uint32_t m_nextTextureId = 1;
};

} // namespace atlas::render
