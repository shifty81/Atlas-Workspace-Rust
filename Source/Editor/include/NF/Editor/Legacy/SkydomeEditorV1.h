#pragma once
// NF::Editor — Skydome editor v1: procedural, HDRI, gradient and cubemap sky presets
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Sdv1SkyType : uint8_t { Procedural, HDRI, Gradient, Cubemap };

inline const char* sdv1SkyTypeName(Sdv1SkyType t) {
    switch (t) {
        case Sdv1SkyType::Procedural: return "Procedural";
        case Sdv1SkyType::HDRI:       return "HDRI";
        case Sdv1SkyType::Gradient:   return "Gradient";
        case Sdv1SkyType::Cubemap:    return "Cubemap";
    }
    return "Unknown";
}

struct Sdv1SkyPreset {
    uint64_t    id          = 0;
    std::string name;
    Sdv1SkyType type        = Sdv1SkyType::Procedural;
    float       rotationDeg = 0.f;
    float       exposure    = 0.f;
    bool        selected    = false;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Sdv1ChangeCallback = std::function<void(uint64_t)>;

class SkydomeEditorV1 {
public:
    static constexpr size_t MAX_PRESETS = 64;

    bool addPreset(const Sdv1SkyPreset& preset) {
        if (!preset.isValid()) return false;
        for (const auto& p : m_presets) if (p.id == preset.id) return false;
        if (m_presets.size() >= MAX_PRESETS) return false;
        m_presets.push_back(preset);
        if (m_onChange) m_onChange(preset.id);
        return true;
    }

    bool removePreset(uint64_t id) {
        for (auto it = m_presets.begin(); it != m_presets.end(); ++it) {
            if (it->id == id) {
                if (m_activePresetId == id) m_activePresetId = 0;
                m_presets.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] Sdv1SkyPreset* findPreset(uint64_t id) {
        for (auto& p : m_presets) if (p.id == id) return &p;
        return nullptr;
    }

    bool selectPreset(uint64_t id) {
        auto* p = findPreset(id);
        if (!p) return false;
        for (auto& q : m_presets) q.selected = false;
        p->selected = true;
        m_activePresetId = id;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setRotation(uint64_t id, float deg) {
        auto* p = findPreset(id);
        if (!p) return false;
        p->rotationDeg = std::clamp(deg, 0.f, 360.f);
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setExposure(uint64_t id, float exposure) {
        auto* p = findPreset(id);
        if (!p) return false;
        p->exposure = std::clamp(exposure, -5.f, 5.f);
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] uint64_t activePresetId() const { return m_activePresetId; }
    [[nodiscard]] size_t   presetCount()    const { return m_presets.size(); }

    [[nodiscard]] size_t countByType(Sdv1SkyType type) const {
        size_t c = 0;
        for (const auto& p : m_presets) if (p.type == type) ++c;
        return c;
    }

    void setOnChange(Sdv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Sdv1SkyPreset> m_presets;
    Sdv1ChangeCallback         m_onChange;
    uint64_t                   m_activePresetId = 0;
};

} // namespace NF
