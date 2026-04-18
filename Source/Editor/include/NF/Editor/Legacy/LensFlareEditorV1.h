#pragma once
// NF::Editor — Lens flare editor v1: lens flare element and profile management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Lfev1ElementType  : uint8_t { Streak, Halo, Glow, Ring, Shimmer, Ghost };
enum class Lfev1ElementState : uint8_t { Hidden, Visible, Previewing };

inline const char* lfev1ElementTypeName(Lfev1ElementType t) {
    switch (t) {
        case Lfev1ElementType::Streak:    return "Streak";
        case Lfev1ElementType::Halo:      return "Halo";
        case Lfev1ElementType::Glow:      return "Glow";
        case Lfev1ElementType::Ring:      return "Ring";
        case Lfev1ElementType::Shimmer:   return "Shimmer";
        case Lfev1ElementType::Ghost:     return "Ghost";
    }
    return "Unknown";
}

inline const char* lfev1ElementStateName(Lfev1ElementState s) {
    switch (s) {
        case Lfev1ElementState::Hidden:     return "Hidden";
        case Lfev1ElementState::Visible:    return "Visible";
        case Lfev1ElementState::Previewing: return "Previewing";
    }
    return "Unknown";
}

struct Lfev1FlareElement {
    uint64_t           id          = 0;
    std::string        name;
    Lfev1ElementType   elementType = Lfev1ElementType::Glow;
    Lfev1ElementState  state       = Lfev1ElementState::Hidden;
    float              intensity   = 1.0f;

    [[nodiscard]] bool isValid()      const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isVisible()    const { return state == Lfev1ElementState::Visible; }
    [[nodiscard]] bool isPreviewing() const { return state == Lfev1ElementState::Previewing; }
};

struct Lfev1FlareProfile {
    uint64_t    id   = 0;
    std::string name;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Lfev1ChangeCallback = std::function<void(uint64_t)>;

class LensFlareEditorV1 {
public:
    static constexpr size_t MAX_ELEMENTS = 256;
    static constexpr size_t MAX_PROFILES = 64;

    bool addElement(const Lfev1FlareElement& elem) {
        if (!elem.isValid()) return false;
        for (const auto& e : m_elements) if (e.id == elem.id) return false;
        if (m_elements.size() >= MAX_ELEMENTS) return false;
        m_elements.push_back(elem);
        if (m_onChange) m_onChange(elem.id);
        return true;
    }

    bool removeElement(uint64_t id) {
        for (auto it = m_elements.begin(); it != m_elements.end(); ++it) {
            if (it->id == id) { m_elements.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Lfev1FlareElement* findElement(uint64_t id) {
        for (auto& e : m_elements) if (e.id == id) return &e;
        return nullptr;
    }

    bool addProfile(const Lfev1FlareProfile& profile) {
        if (!profile.isValid()) return false;
        for (const auto& p : m_profiles) if (p.id == profile.id) return false;
        if (m_profiles.size() >= MAX_PROFILES) return false;
        m_profiles.push_back(profile);
        if (m_onChange) m_onChange(profile.id);
        return true;
    }

    bool removeProfile(uint64_t id) {
        for (auto it = m_profiles.begin(); it != m_profiles.end(); ++it) {
            if (it->id == id) { m_profiles.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t elementCount() const { return m_elements.size(); }
    [[nodiscard]] size_t profileCount() const { return m_profiles.size(); }

    [[nodiscard]] size_t visibleCount() const {
        size_t c = 0; for (const auto& e : m_elements) if (e.isVisible()) ++c; return c;
    }
    [[nodiscard]] size_t previewingCount() const {
        size_t c = 0; for (const auto& e : m_elements) if (e.isPreviewing()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(Lfev1ElementType type) const {
        size_t c = 0; for (const auto& e : m_elements) if (e.elementType == type) ++c; return c;
    }

    void setOnChange(Lfev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Lfev1FlareElement> m_elements;
    std::vector<Lfev1FlareProfile> m_profiles;
    Lfev1ChangeCallback            m_onChange;
};

} // namespace NF
