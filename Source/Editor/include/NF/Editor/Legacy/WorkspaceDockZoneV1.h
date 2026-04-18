#pragma once
// NF::Editor — Workspace dock zone v1: dock region and tab group authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Wdzv1DockSide   : uint8_t { Left, Right, Top, Bottom, Center, Floating };
enum class Wdzv1ZoneState  : uint8_t { Empty, Populated, Pinned, Hidden, Collapsed };
enum class Wdzv1ResizeAxis : uint8_t { None, Horizontal, Vertical, Both };

inline const char* wdzv1DockSideName(Wdzv1DockSide s) {
    switch (s) {
        case Wdzv1DockSide::Left:     return "Left";
        case Wdzv1DockSide::Right:    return "Right";
        case Wdzv1DockSide::Top:      return "Top";
        case Wdzv1DockSide::Bottom:   return "Bottom";
        case Wdzv1DockSide::Center:   return "Center";
        case Wdzv1DockSide::Floating: return "Floating";
    }
    return "Unknown";
}

inline const char* wdzv1ZoneStateName(Wdzv1ZoneState s) {
    switch (s) {
        case Wdzv1ZoneState::Empty:     return "Empty";
        case Wdzv1ZoneState::Populated: return "Populated";
        case Wdzv1ZoneState::Pinned:    return "Pinned";
        case Wdzv1ZoneState::Hidden:    return "Hidden";
        case Wdzv1ZoneState::Collapsed: return "Collapsed";
    }
    return "Unknown";
}

struct Wdzv1TabEntry {
    std::string panelId;
    std::string title;
    bool        isActive  = false;
    bool        isDirty   = false;
    bool        isPinned  = false;

    [[nodiscard]] bool isValid() const { return !panelId.empty() && !title.empty(); }
};

struct Wdzv1DockZone {
    uint64_t          id          = 0;
    std::string       name;
    Wdzv1DockSide     side        = Wdzv1DockSide::Center;
    Wdzv1ZoneState    state       = Wdzv1ZoneState::Empty;
    Wdzv1ResizeAxis   resizeAxis  = Wdzv1ResizeAxis::Both;
    float             sizeRatio   = 0.25f;
    std::vector<Wdzv1TabEntry> tabs;

    [[nodiscard]] bool isValid()     const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isVisible()   const { return state != Wdzv1ZoneState::Hidden && state != Wdzv1ZoneState::Collapsed; }
    [[nodiscard]] bool isPinned()    const { return state == Wdzv1ZoneState::Pinned; }
    [[nodiscard]] bool isCollapsed() const { return state == Wdzv1ZoneState::Collapsed; }

    bool addTab(const Wdzv1TabEntry& tab) {
        if (!tab.isValid()) return false;
        for (const auto& t : tabs) if (t.panelId == tab.panelId) return false;
        tabs.push_back(tab);
        return true;
    }

    bool removeTab(const std::string& panelId) {
        for (auto it = tabs.begin(); it != tabs.end(); ++it) {
            if (it->panelId == panelId) { tabs.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t dirtyTabCount() const {
        size_t c = 0; for (const auto& t : tabs) if (t.isDirty) ++c; return c;
    }
};

using Wdzv1ChangeCallback = std::function<void(uint64_t)>;

class WorkspaceDockZoneV1 {
public:
    static constexpr size_t MAX_ZONES = 32;

    bool addZone(const Wdzv1DockZone& zone) {
        if (!zone.isValid()) return false;
        for (const auto& z : m_zones) if (z.id == zone.id) return false;
        if (m_zones.size() >= MAX_ZONES) return false;
        m_zones.push_back(zone);
        return true;
    }

    bool removeZone(uint64_t id) {
        for (auto it = m_zones.begin(); it != m_zones.end(); ++it) {
            if (it->id == id) { m_zones.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Wdzv1DockZone* findZone(uint64_t id) {
        for (auto& z : m_zones) if (z.id == id) return &z;
        return nullptr;
    }

    bool setState(uint64_t id, Wdzv1ZoneState state) {
        auto* z = findZone(id);
        if (!z) return false;
        z->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setSizeRatio(uint64_t id, float ratio) {
        if (ratio <= 0.f || ratio > 1.f) return false;
        auto* z = findZone(id);
        if (!z) return false;
        z->sizeRatio = ratio;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool addTab(uint64_t zoneId, const Wdzv1TabEntry& tab) {
        auto* z = findZone(zoneId);
        return z && z->addTab(tab);
    }

    bool removeTab(uint64_t zoneId, const std::string& panelId) {
        auto* z = findZone(zoneId);
        return z && z->removeTab(panelId);
    }

    [[nodiscard]] size_t zoneCount()     const { return m_zones.size(); }
    [[nodiscard]] size_t visibleCount()  const {
        size_t c = 0; for (const auto& z : m_zones) if (z.isVisible())   ++c; return c;
    }
    [[nodiscard]] size_t pinnedCount()   const {
        size_t c = 0; for (const auto& z : m_zones) if (z.isPinned())    ++c; return c;
    }
    [[nodiscard]] size_t collapsedCount() const {
        size_t c = 0; for (const auto& z : m_zones) if (z.isCollapsed()) ++c; return c;
    }
    [[nodiscard]] size_t countBySide(Wdzv1DockSide side) const {
        size_t c = 0; for (const auto& z : m_zones) if (z.side == side) ++c; return c;
    }

    void setOnChange(Wdzv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Wdzv1DockZone> m_zones;
    Wdzv1ChangeCallback        m_onChange;
};

} // namespace NF
