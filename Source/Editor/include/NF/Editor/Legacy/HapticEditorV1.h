#pragma once
// NF::Editor — Haptic editor v1: click, pulse, rumble, vibrate and impact pattern authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Hev1HapticType : uint8_t { Click, Pulse, Rumble, Vibrate, Impact };

inline const char* hev1HapticTypeName(Hev1HapticType t) {
    switch (t) {
        case Hev1HapticType::Click:   return "Click";
        case Hev1HapticType::Pulse:   return "Pulse";
        case Hev1HapticType::Rumble:  return "Rumble";
        case Hev1HapticType::Vibrate: return "Vibrate";
        case Hev1HapticType::Impact:  return "Impact";
    }
    return "Unknown";
}

struct Hev1Pattern {
    uint64_t      id        = 0;
    std::string   name;
    Hev1HapticType type     = Hev1HapticType::Click;
    float         intensity = 0.5f;
    float         duration  = 0.1f;
    float         frequency = 0.f;
    bool          looping   = false;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Hev1ChangeCallback = std::function<void(uint64_t)>;

class HapticEditorV1 {
public:
    static constexpr size_t MAX_PATTERNS = 256;

    bool addPattern(const Hev1Pattern& pattern) {
        if (!pattern.isValid()) return false;
        for (const auto& p : m_patterns) if (p.id == pattern.id) return false;
        if (m_patterns.size() >= MAX_PATTERNS) return false;
        m_patterns.push_back(pattern);
        if (m_onChange) m_onChange(pattern.id);
        return true;
    }

    bool removePattern(uint64_t id) {
        for (auto it = m_patterns.begin(); it != m_patterns.end(); ++it) {
            if (it->id == id) { m_patterns.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Hev1Pattern* findPattern(uint64_t id) {
        for (auto& p : m_patterns) if (p.id == id) return &p;
        return nullptr;
    }

    bool setIntensity(uint64_t id, float intensity) {
        auto* p = findPattern(id);
        if (!p) return false;
        p->intensity = std::clamp(intensity, 0.f, 1.f);
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setDuration(uint64_t id, float duration) {
        auto* p = findPattern(id);
        if (!p) return false;
        p->duration = std::clamp(duration, 0.f, 10.f);
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setFrequency(uint64_t id, float frequency) {
        auto* p = findPattern(id);
        if (!p) return false;
        p->frequency = std::clamp(frequency, 0.f, 1000.f);
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool toggleLooping(uint64_t id) {
        auto* p = findPattern(id);
        if (!p) return false;
        p->looping = !p->looping;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t patternCount() const { return m_patterns.size(); }

    [[nodiscard]] size_t loopingCount() const {
        size_t c = 0;
        for (const auto& p : m_patterns) if (p.looping) ++c;
        return c;
    }

    [[nodiscard]] size_t countByType(Hev1HapticType type) const {
        size_t c = 0;
        for (const auto& p : m_patterns) if (p.type == type) ++c;
        return c;
    }

    void setOnChange(Hev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Hev1Pattern> m_patterns;
    Hev1ChangeCallback       m_onChange;
};

} // namespace NF
