#pragma once
// NF::Editor — Wind field editor v1: volumetric wind force authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Wfv1WindType  : uint8_t { Directional, Point, Vortex, Turbulence };
enum class Wfv1WindState : uint8_t { Active, Disabled, Preview, Paused };

inline const char* wfv1WindTypeName(Wfv1WindType t) {
    switch (t) {
        case Wfv1WindType::Directional: return "Directional";
        case Wfv1WindType::Point:       return "Point";
        case Wfv1WindType::Vortex:      return "Vortex";
        case Wfv1WindType::Turbulence:  return "Turbulence";
    }
    return "Unknown";
}

inline const char* wfv1WindStateName(Wfv1WindState s) {
    switch (s) {
        case Wfv1WindState::Active:   return "Active";
        case Wfv1WindState::Disabled: return "Disabled";
        case Wfv1WindState::Preview:  return "Preview";
        case Wfv1WindState::Paused:   return "Paused";
    }
    return "Unknown";
}

struct Wfv1Source {
    uint64_t       id        = 0;
    std::string    name;
    Wfv1WindType   type      = Wfv1WindType::Directional;
    Wfv1WindState  state     = Wfv1WindState::Active;
    float          strength  = 10.f;
    float          radius    = 50.f;
    float          dirX      = 1.f;
    float          dirY      = 0.f;
    float          dirZ      = 0.f;
    float          frequency = 1.f;  // oscillation Hz for turbulence

    [[nodiscard]] bool isValid()  const { return id != 0 && !name.empty() && strength > 0.f; }
    [[nodiscard]] bool isActive() const { return state == Wfv1WindState::Active; }
};

using Wfv1ChangeCallback = std::function<void(uint64_t)>;

class WindFieldEditorV1 {
public:
    static constexpr size_t MAX_SOURCES = 128;

    bool addSource(const Wfv1Source& source) {
        if (!source.isValid()) return false;
        for (const auto& s : m_sources) if (s.id == source.id) return false;
        if (m_sources.size() >= MAX_SOURCES) return false;
        m_sources.push_back(source);
        if (m_onChange) m_onChange(source.id);
        return true;
    }

    bool removeSource(uint64_t id) {
        for (auto it = m_sources.begin(); it != m_sources.end(); ++it) {
            if (it->id == id) { m_sources.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Wfv1Source* findSource(uint64_t id) {
        for (auto& s : m_sources) if (s.id == id) return &s;
        return nullptr;
    }

    bool setState(uint64_t id, Wfv1WindState state) {
        auto* s = findSource(id);
        if (!s) return false;
        s->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setStrength(uint64_t id, float strength) {
        auto* s = findSource(id);
        if (!s) return false;
        s->strength = std::max(0.f, strength);
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t sourceCount() const { return m_sources.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0;
        for (const auto& s : m_sources) if (s.isActive()) ++c;
        return c;
    }

    [[nodiscard]] size_t countByType(Wfv1WindType type) const {
        size_t c = 0;
        for (const auto& s : m_sources) if (s.type == type) ++c;
        return c;
    }

    [[nodiscard]] float totalStrength() const {
        float total = 0.f;
        for (const auto& s : m_sources) if (s.isActive()) total += s.strength;
        return total;
    }

    void setOnChange(Wfv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Wfv1Source> m_sources;
    Wfv1ChangeCallback     m_onChange;
};

} // namespace NF
