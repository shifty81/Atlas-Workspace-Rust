#pragma once
// NF::Editor — Atmosphere editor v1: sky, fog, and atmospheric scattering authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Aev1AtmoType  : uint8_t { Rayleigh, Mie, Combined, Custom };
enum class Aev1FogType   : uint8_t { Linear, Exponential, ExponentialSquared, Height };
enum class Aev1TimeOfDay : uint8_t { Dawn, Morning, Noon, Afternoon, Dusk, Night };

inline const char* aev1AtmoTypeName(Aev1AtmoType t) {
    switch (t) {
        case Aev1AtmoType::Rayleigh: return "Rayleigh";
        case Aev1AtmoType::Mie:      return "Mie";
        case Aev1AtmoType::Combined: return "Combined";
        case Aev1AtmoType::Custom:   return "Custom";
    }
    return "Unknown";
}

inline const char* aev1FogTypeName(Aev1FogType t) {
    switch (t) {
        case Aev1FogType::Linear:             return "Linear";
        case Aev1FogType::Exponential:        return "Exponential";
        case Aev1FogType::ExponentialSquared: return "ExponentialSquared";
        case Aev1FogType::Height:             return "Height";
    }
    return "Unknown";
}

inline const char* aev1TimeOfDayName(Aev1TimeOfDay t) {
    switch (t) {
        case Aev1TimeOfDay::Dawn:      return "Dawn";
        case Aev1TimeOfDay::Morning:   return "Morning";
        case Aev1TimeOfDay::Noon:      return "Noon";
        case Aev1TimeOfDay::Afternoon: return "Afternoon";
        case Aev1TimeOfDay::Dusk:      return "Dusk";
        case Aev1TimeOfDay::Night:     return "Night";
    }
    return "Unknown";
}

struct Aev1AtmoPreset {
    uint64_t       id            = 0;
    std::string    name;
    Aev1AtmoType   scatterType   = Aev1AtmoType::Combined;
    Aev1FogType    fogType       = Aev1FogType::Exponential;
    Aev1TimeOfDay  timeOfDay     = Aev1TimeOfDay::Noon;
    float          sunIntensity  = 1.f;
    float          fogDensity    = 0.02f;
    float          fogStart      = 10.f;
    float          fogEnd        = 500.f;
    float          rayleighScale = 1.f;
    float          mieScale      = 1.f;
    uint32_t       skyColor      = 0x87CEEBFF;  // light blue
    uint32_t       fogColor      = 0xC0C0C0FF;  // grey

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Aev1ChangeCallback = std::function<void(uint64_t)>;

class AtmosphereEditorV1 {
public:
    static constexpr size_t MAX_PRESETS = 64;

    bool addPreset(const Aev1AtmoPreset& preset) {
        if (!preset.isValid()) return false;
        for (const auto& p : m_presets) if (p.id == preset.id) return false;
        if (m_presets.size() >= MAX_PRESETS) return false;
        m_presets.push_back(preset);
        if (m_onChange) m_onChange(preset.id);
        return true;
    }

    bool removePreset(uint64_t id) {
        for (auto it = m_presets.begin(); it != m_presets.end(); ++it) {
            if (it->id == id) { m_presets.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Aev1AtmoPreset* findPreset(uint64_t id) {
        for (auto& p : m_presets) if (p.id == id) return &p;
        return nullptr;
    }

    bool selectPreset(uint64_t id) {
        if (!findPreset(id)) return false;
        m_activePresetId = id;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] uint64_t activePresetId() const { return m_activePresetId; }

    bool setFogDensity(uint64_t id, float density) {
        auto* p = findPreset(id);
        if (!p) return false;
        p->fogDensity = std::max(0.f, density);
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setSunIntensity(uint64_t id, float intensity) {
        auto* p = findPreset(id);
        if (!p) return false;
        p->sunIntensity = std::max(0.f, intensity);
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setTimeOfDay(uint64_t id, Aev1TimeOfDay tod) {
        auto* p = findPreset(id);
        if (!p) return false;
        p->timeOfDay = tod;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t presetCount() const { return m_presets.size(); }

    [[nodiscard]] size_t countByTimeOfDay(Aev1TimeOfDay tod) const {
        size_t c = 0;
        for (const auto& p : m_presets) if (p.timeOfDay == tod) ++c;
        return c;
    }

    [[nodiscard]] size_t countByAtmoType(Aev1AtmoType type) const {
        size_t c = 0;
        for (const auto& p : m_presets) if (p.scatterType == type) ++c;
        return c;
    }

    void setOnChange(Aev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Aev1AtmoPreset> m_presets;
    uint64_t                    m_activePresetId = 0;
    Aev1ChangeCallback          m_onChange;
};

} // namespace NF
