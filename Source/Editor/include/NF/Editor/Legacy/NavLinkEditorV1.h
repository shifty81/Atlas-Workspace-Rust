#pragma once
// NF::Editor — Nav link editor v1: navigation link and area management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Nlev1LinkType  : uint8_t { Jump, Climb, Swim, Teleport, Door, Ladder };
enum class Nlev1LinkState : uint8_t { Inactive, Active, Blocked, Disabled };

inline const char* nlev1LinkTypeName(Nlev1LinkType t) {
    switch (t) {
        case Nlev1LinkType::Jump:      return "Jump";
        case Nlev1LinkType::Climb:     return "Climb";
        case Nlev1LinkType::Swim:      return "Swim";
        case Nlev1LinkType::Teleport:  return "Teleport";
        case Nlev1LinkType::Door:      return "Door";
        case Nlev1LinkType::Ladder:    return "Ladder";
    }
    return "Unknown";
}

inline const char* nlev1LinkStateName(Nlev1LinkState s) {
    switch (s) {
        case Nlev1LinkState::Inactive: return "Inactive";
        case Nlev1LinkState::Active:   return "Active";
        case Nlev1LinkState::Blocked:  return "Blocked";
        case Nlev1LinkState::Disabled: return "Disabled";
    }
    return "Unknown";
}

struct Nlev1NavLink {
    uint64_t       id        = 0;
    std::string    name;
    Nlev1LinkType  linkType  = Nlev1LinkType::Jump;
    Nlev1LinkState state     = Nlev1LinkState::Inactive;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive()   const { return state == Nlev1LinkState::Active; }
    [[nodiscard]] bool isBlocked()  const { return state == Nlev1LinkState::Blocked; }
    [[nodiscard]] bool isDisabled() const { return state == Nlev1LinkState::Disabled; }
};

struct Nlev1NavArea {
    uint64_t    id   = 0;
    std::string name;
    float       cost = 1.0f;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Nlev1ChangeCallback = std::function<void(uint64_t)>;

class NavLinkEditorV1 {
public:
    static constexpr size_t MAX_LINKS = 2048;
    static constexpr size_t MAX_AREAS = 256;

    bool addLink(const Nlev1NavLink& link) {
        if (!link.isValid()) return false;
        for (const auto& l : m_links) if (l.id == link.id) return false;
        if (m_links.size() >= MAX_LINKS) return false;
        m_links.push_back(link);
        if (m_onChange) m_onChange(link.id);
        return true;
    }

    bool removeLink(uint64_t id) {
        for (auto it = m_links.begin(); it != m_links.end(); ++it) {
            if (it->id == id) { m_links.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Nlev1NavLink* findLink(uint64_t id) {
        for (auto& l : m_links) if (l.id == id) return &l;
        return nullptr;
    }

    bool addArea(const Nlev1NavArea& area) {
        if (!area.isValid()) return false;
        for (const auto& a : m_areas) if (a.id == area.id) return false;
        if (m_areas.size() >= MAX_AREAS) return false;
        m_areas.push_back(area);
        if (m_onChange) m_onChange(area.id);
        return true;
    }

    bool removeArea(uint64_t id) {
        for (auto it = m_areas.begin(); it != m_areas.end(); ++it) {
            if (it->id == id) { m_areas.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t linkCount() const { return m_links.size(); }
    [[nodiscard]] size_t areaCount() const { return m_areas.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0; for (const auto& l : m_links) if (l.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t blockedCount() const {
        size_t c = 0; for (const auto& l : m_links) if (l.isBlocked()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(Nlev1LinkType type) const {
        size_t c = 0; for (const auto& l : m_links) if (l.linkType == type) ++c; return c;
    }

    void setOnChange(Nlev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Nlev1NavLink> m_links;
    std::vector<Nlev1NavArea> m_areas;
    Nlev1ChangeCallback       m_onChange;
};

} // namespace NF
