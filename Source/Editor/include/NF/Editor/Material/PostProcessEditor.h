#pragma once
// NF::Editor — Post-process effect editor
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

enum class PostProcessEffect : uint8_t {
    Bloom, DepthOfField, MotionBlur, AmbientOcclusion, ChromaticAberration,
    Vignette, LensFlare, ColorGrading, ToneMapping, FilmGrain
};

inline const char* postProcessEffectName(PostProcessEffect e) {
    switch (e) {
        case PostProcessEffect::Bloom:               return "Bloom";
        case PostProcessEffect::DepthOfField:        return "DepthOfField";
        case PostProcessEffect::MotionBlur:          return "MotionBlur";
        case PostProcessEffect::AmbientOcclusion:    return "AmbientOcclusion";
        case PostProcessEffect::ChromaticAberration: return "ChromaticAberration";
        case PostProcessEffect::Vignette:            return "Vignette";
        case PostProcessEffect::LensFlare:           return "LensFlare";
        case PostProcessEffect::ColorGrading:        return "ColorGrading";
        case PostProcessEffect::ToneMapping:         return "ToneMapping";
        case PostProcessEffect::FilmGrain:           return "FilmGrain";
    }
    return "Unknown";
}

enum class ToneMappingOperator : uint8_t {
    Linear, Reinhard, ACES, Filmic, Uncharted2, Custom
};

inline const char* toneMappingOperatorName(ToneMappingOperator t) {
    switch (t) {
        case ToneMappingOperator::Linear:     return "Linear";
        case ToneMappingOperator::Reinhard:   return "Reinhard";
        case ToneMappingOperator::ACES:       return "ACES";
        case ToneMappingOperator::Filmic:     return "Filmic";
        case ToneMappingOperator::Uncharted2: return "Uncharted2";
        case ToneMappingOperator::Custom:     return "Custom";
    }
    return "Unknown";
}

enum class PostProcessPassOrder : uint8_t {
    BeforeTransparency, AfterTransparency, BeforeUI, AfterUI, Custom
};

inline const char* postProcessPassOrderName(PostProcessPassOrder p) {
    switch (p) {
        case PostProcessPassOrder::BeforeTransparency: return "BeforeTransparency";
        case PostProcessPassOrder::AfterTransparency:  return "AfterTransparency";
        case PostProcessPassOrder::BeforeUI:           return "BeforeUI";
        case PostProcessPassOrder::AfterUI:            return "AfterUI";
        case PostProcessPassOrder::Custom:             return "Custom";
    }
    return "Unknown";
}

class PostProcessLayer {
public:
    explicit PostProcessLayer(const std::string& name, PostProcessEffect effect)
        : m_name(name), m_effect(effect) {}

    void setEnabled(bool v)               { m_enabled   = v; }
    void setIntensity(float i)            { m_intensity = i; }
    void setPassOrder(PostProcessPassOrder o) { m_passOrder = o; }
    void setPriority(int p)               { m_priority  = p; }

    [[nodiscard]] const std::string&    name()      const { return m_name;      }
    [[nodiscard]] PostProcessEffect     effect()    const { return m_effect;    }
    [[nodiscard]] bool                  isEnabled() const { return m_enabled;   }
    [[nodiscard]] float                 intensity() const { return m_intensity; }
    [[nodiscard]] PostProcessPassOrder  passOrder() const { return m_passOrder; }
    [[nodiscard]] int                   priority()  const { return m_priority;  }

private:
    std::string           m_name;
    PostProcessEffect     m_effect;
    PostProcessPassOrder  m_passOrder = PostProcessPassOrder::AfterTransparency;
    float                 m_intensity = 1.0f;
    int                   m_priority  = 0;
    bool                  m_enabled   = true;
};

class PostProcessEditor {
public:
    static constexpr size_t MAX_LAYERS = 32;

    [[nodiscard]] bool addLayer(const PostProcessLayer& layer) {
        for (auto& l : m_layers) if (l.name() == layer.name()) return false;
        if (m_layers.size() >= MAX_LAYERS) return false;
        m_layers.push_back(layer);
        return true;
    }

    [[nodiscard]] bool removeLayer(const std::string& name) {
        for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
            if (it->name() == name) {
                if (m_activeLayer == name) m_activeLayer.clear();
                m_layers.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] PostProcessLayer* findLayer(const std::string& name) {
        for (auto& l : m_layers) if (l.name() == name) return &l;
        return nullptr;
    }

    [[nodiscard]] bool setActiveLayer(const std::string& name) {
        for (auto& l : m_layers) if (l.name() == name) { m_activeLayer = name; return true; }
        return false;
    }

    void setToneMapping(ToneMappingOperator t) { m_toneMapping = t; }

    [[nodiscard]] const std::string&  activeLayer()  const { return m_activeLayer; }
    [[nodiscard]] ToneMappingOperator toneMapping()  const { return m_toneMapping; }
    [[nodiscard]] size_t              layerCount()   const { return m_layers.size(); }

    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (auto& l : m_layers) if (l.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t countByEffect(PostProcessEffect e) const {
        size_t c = 0; for (auto& l : m_layers) if (l.effect() == e) ++c; return c;
    }

private:
    std::vector<PostProcessLayer> m_layers;
    std::string                   m_activeLayer;
    ToneMappingOperator           m_toneMapping = ToneMappingOperator::ACES;
};

} // namespace NF
