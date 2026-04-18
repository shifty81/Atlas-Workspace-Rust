#pragma once
// NF::Editor — Render settings panel
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

enum class RenderPipeline : uint8_t {
    Forward, Deferred, ForwardPlus, TiledDeferred, Custom
};

inline const char* renderPipelineName(RenderPipeline p) {
    switch (p) {
        case RenderPipeline::Forward:       return "Forward";
        case RenderPipeline::Deferred:      return "Deferred";
        case RenderPipeline::ForwardPlus:   return "ForwardPlus";
        case RenderPipeline::TiledDeferred: return "TiledDeferred";
        case RenderPipeline::Custom:        return "Custom";
    }
    return "Unknown";
}

enum class AntiAliasingMode : uint8_t {
    None, MSAA2x, MSAA4x, MSAA8x, FXAA, TAA, DLSS, FSR
};

inline const char* antiAliasingModeName(AntiAliasingMode m) {
    switch (m) {
        case AntiAliasingMode::None:   return "None";
        case AntiAliasingMode::MSAA2x: return "MSAA2x";
        case AntiAliasingMode::MSAA4x: return "MSAA4x";
        case AntiAliasingMode::MSAA8x: return "MSAA8x";
        case AntiAliasingMode::FXAA:   return "FXAA";
        case AntiAliasingMode::TAA:    return "TAA";
        case AntiAliasingMode::DLSS:   return "DLSS";
        case AntiAliasingMode::FSR:    return "FSR";
    }
    return "Unknown";
}

enum class ShadowQuality : uint8_t {
    Disabled, Low, Medium, High, Ultra
};

inline const char* shadowQualityName(ShadowQuality q) {
    switch (q) {
        case ShadowQuality::Disabled: return "Disabled";
        case ShadowQuality::Low:      return "Low";
        case ShadowQuality::Medium:   return "Medium";
        case ShadowQuality::High:     return "High";
        case ShadowQuality::Ultra:    return "Ultra";
    }
    return "Unknown";
}

enum class TextureQualityLevel : uint8_t {
    Low, Medium, High, Ultra, Streaming
};

inline const char* textureQualityLevelName(TextureQualityLevel l) {
    switch (l) {
        case TextureQualityLevel::Low:       return "Low";
        case TextureQualityLevel::Medium:    return "Medium";
        case TextureQualityLevel::High:      return "High";
        case TextureQualityLevel::Ultra:     return "Ultra";
        case TextureQualityLevel::Streaming: return "Streaming";
    }
    return "Unknown";
}

class RenderSettingsPanel {
public:
    void setPipeline(RenderPipeline p)         { m_pipeline        = p; }
    void setAntiAliasing(AntiAliasingMode a)   { m_antiAliasing    = a; }
    void setShadowQuality(ShadowQuality q)     { m_shadowQuality   = q; }
    void setTextureQuality(TextureQualityLevel l) { m_textureQuality = l; }

    void setVSyncEnabled(bool v)               { m_vsync           = v; }
    void setHDREnabled(bool v)                 { m_hdr             = v; }
    void setRayTracingEnabled(bool v)          { m_rayTracing      = v; }
    void setTargetFPS(uint16_t fps)            { m_targetFPS       = fps; }
    void setRenderScale(float s)               { m_renderScale     = s; }
    void setMaxDrawDistance(float d)           { m_maxDrawDistance = d; }

    [[nodiscard]] RenderPipeline      pipeline()        const { return m_pipeline;        }
    [[nodiscard]] AntiAliasingMode    antiAliasing()    const { return m_antiAliasing;    }
    [[nodiscard]] ShadowQuality       shadowQuality()   const { return m_shadowQuality;   }
    [[nodiscard]] TextureQualityLevel textureQuality()  const { return m_textureQuality;  }

    [[nodiscard]] bool     vsyncEnabled()      const { return m_vsync;           }
    [[nodiscard]] bool     hdrEnabled()        const { return m_hdr;             }
    [[nodiscard]] bool     rayTracingEnabled() const { return m_rayTracing;      }
    [[nodiscard]] uint16_t targetFPS()         const { return m_targetFPS;       }
    [[nodiscard]] float    renderScale()       const { return m_renderScale;     }
    [[nodiscard]] float    maxDrawDistance()   const { return m_maxDrawDistance; }

    [[nodiscard]] bool isHighFidelity() const {
        return m_shadowQuality   >= ShadowQuality::High &&
               m_textureQuality  >= TextureQualityLevel::High;
    }

private:
    RenderPipeline      m_pipeline        = RenderPipeline::Deferred;
    AntiAliasingMode    m_antiAliasing    = AntiAliasingMode::TAA;
    ShadowQuality       m_shadowQuality   = ShadowQuality::High;
    TextureQualityLevel m_textureQuality  = TextureQualityLevel::High;
    float               m_renderScale     = 1.0f;
    float               m_maxDrawDistance = 1000.0f;
    uint16_t            m_targetFPS       = 60;
    bool                m_vsync           = true;
    bool                m_hdr             = false;
    bool                m_rayTracing      = false;
};

} // namespace NF
