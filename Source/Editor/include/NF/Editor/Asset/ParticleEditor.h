#pragma once
// NF::Editor — Particle effect layer + editor
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

enum class ParticleEmitterShape : uint8_t {
    Point, Circle, Rectangle, Cone, Sphere, Ring, Line, Custom
};

inline const char* particleEmitterShapeName(ParticleEmitterShape s) {
    switch (s) {
        case ParticleEmitterShape::Point:     return "Point";
        case ParticleEmitterShape::Circle:    return "Circle";
        case ParticleEmitterShape::Rectangle: return "Rectangle";
        case ParticleEmitterShape::Cone:      return "Cone";
        case ParticleEmitterShape::Sphere:    return "Sphere";
        case ParticleEmitterShape::Ring:      return "Ring";
        case ParticleEmitterShape::Line:      return "Line";
        case ParticleEmitterShape::Custom:    return "Custom";
    }
    return "Unknown";
}

enum class ParticleBlendMode : uint8_t {
    Additive, Alpha, Multiply, Screen
};

inline const char* particleBlendModeName(ParticleBlendMode b) {
    switch (b) {
        case ParticleBlendMode::Additive: return "Additive";
        case ParticleBlendMode::Alpha:    return "Alpha";
        case ParticleBlendMode::Multiply: return "Multiply";
        case ParticleBlendMode::Screen:   return "Screen";
    }
    return "Unknown";
}

struct ParticleEmitterConfig {
    std::string          id;
    ParticleEmitterShape shape       = ParticleEmitterShape::Point;
    ParticleBlendMode    blendMode   = ParticleBlendMode::Additive;
    float                emitRate    = 10.0f;
    float                lifetime    = 1.0f;
    float                speed       = 1.0f;
    float                size        = 1.0f;
    bool                 looping     = true;

    void setEmitRate(float r) { emitRate = r; }
    void setLifetime(float l) { lifetime = l; }
    void setSpeed(float s)    { speed = s; }
    void setSize(float s)     { size = s; }

    [[nodiscard]] bool isValid() const {
        return emitRate > 0.0f && lifetime > 0.0f && size > 0.0f;
    }
};

class ParticleEffectLayer {
public:
    static constexpr size_t MAX_EMITTERS = 64;

    explicit ParticleEffectLayer(const std::string& name) : m_name(name) {}

    [[nodiscard]] bool addEmitter(const ParticleEmitterConfig& cfg) {
        for (auto& e : m_emitters) if (e.id == cfg.id) return false;
        if (m_emitters.size() >= MAX_EMITTERS) return false;
        m_emitters.push_back(cfg);
        return true;
    }

    [[nodiscard]] bool removeEmitter(const std::string& id) {
        for (auto it = m_emitters.begin(); it != m_emitters.end(); ++it) {
            if (it->id == id) { m_emitters.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] ParticleEmitterConfig* findEmitter(const std::string& id) {
        for (auto& e : m_emitters) if (e.id == id) return &e;
        return nullptr;
    }

    [[nodiscard]] size_t emitterCount()   const { return m_emitters.size(); }
    [[nodiscard]] bool   visible()        const { return m_visible; }
    void                 setVisible(bool v)     { m_visible = v; }
    [[nodiscard]] const std::string& name() const { return m_name; }

private:
    std::string                      m_name;
    std::vector<ParticleEmitterConfig> m_emitters;
    bool                             m_visible = true;
};

class ParticleEffectEditor {
public:
    static constexpr size_t MAX_LAYERS = 32;

    [[nodiscard]] bool addLayer(const ParticleEffectLayer& layer) {
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

    [[nodiscard]] ParticleEffectLayer* findLayer(const std::string& name) {
        for (auto& l : m_layers) if (l.name() == name) return &l;
        return nullptr;
    }

    [[nodiscard]] bool setActiveLayer(const std::string& name) {
        for (auto& l : m_layers) {
            if (l.name() == name) { m_activeLayer = name; return true; }
        }
        return false;
    }

    void preview()  { m_previewing = true;  }
    void stopPreview() { m_previewing = false; }

    [[nodiscard]] bool               isPreviewing() const { return m_previewing; }
    [[nodiscard]] const std::string& activeLayer()  const { return m_activeLayer; }
    [[nodiscard]] size_t             layerCount()   const { return m_layers.size(); }

    [[nodiscard]] size_t totalEmitterCount() const {
        size_t c = 0;
        for (auto& l : m_layers) c += l.emitterCount();
        return c;
    }

private:
    std::vector<ParticleEffectLayer> m_layers;
    std::string                      m_activeLayer;
    bool                             m_previewing = false;
};

// ── S33 — Shader Graph Editor ─────────────────────────────────────


} // namespace NF
