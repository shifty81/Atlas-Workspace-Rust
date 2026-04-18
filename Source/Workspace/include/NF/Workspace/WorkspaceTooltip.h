#pragma once
// NF::Workspace — Phase 24: Workspace Tooltip and Help System
//
// Workspace-level tooltip lifecycle and content management:
//   TooltipTrigger  — how a tooltip is activated (Hover/Focus/Manual)
//   TooltipPosition — preferred display anchor (Auto/Top/Bottom/Left/Right)
//   TooltipEntry    — id + title + body + trigger + position + target; isValid()
//   TooltipState    — tracks which tooltip is currently visible and when
//   TooltipManager  — registry with show/hide lifecycle and observers

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// TooltipTrigger — activation mode
// ═════════════════════════════════════════════════════════════════

enum class TooltipTrigger : uint8_t {
    Hover,   // shown on pointer hover over target element
    Focus,   // shown when target element gains keyboard focus
    Manual,  // shown programmatically via show()
};

inline const char* tooltipTriggerName(TooltipTrigger t) {
    switch (t) {
        case TooltipTrigger::Hover:  return "Hover";
        case TooltipTrigger::Focus:  return "Focus";
        case TooltipTrigger::Manual: return "Manual";
        default:                     return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// TooltipPosition — preferred display anchor
// ═════════════════════════════════════════════════════════════════

enum class TooltipPosition : uint8_t {
    Auto,
    Top,
    Bottom,
    Left,
    Right,
};

inline const char* tooltipPositionName(TooltipPosition p) {
    switch (p) {
        case TooltipPosition::Auto:   return "Auto";
        case TooltipPosition::Top:    return "Top";
        case TooltipPosition::Bottom: return "Bottom";
        case TooltipPosition::Left:   return "Left";
        case TooltipPosition::Right:  return "Right";
        default:                      return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// TooltipEntry — tooltip content and display metadata
// ═════════════════════════════════════════════════════════════════

struct TooltipEntry {
    std::string     id;
    std::string     title;
    std::string     body;
    std::string     targetElementId;  // UI element this tooltip is attached to
    TooltipTrigger  trigger  = TooltipTrigger::Hover;
    TooltipPosition position = TooltipPosition::Auto;
    bool            enabled  = true;

    bool isValid() const { return !id.empty() && !body.empty(); }

    bool operator==(const TooltipEntry& o) const { return id == o.id; }
    bool operator!=(const TooltipEntry& o) const { return id != o.id; }
};

// ═════════════════════════════════════════════════════════════════
// TooltipState — current visibility record
// ═════════════════════════════════════════════════════════════════

struct TooltipState {
    std::string entryId;
    bool        visible        = false;
    uint64_t    showTimestamp  = 0;

    bool isValid() const { return !entryId.empty(); }
};

// ═════════════════════════════════════════════════════════════════
// TooltipManager — registry with show/hide lifecycle and observers
// ═════════════════════════════════════════════════════════════════

class TooltipManager {
public:
    using Observer = std::function<void(const TooltipEntry&, bool visible)>;

    static constexpr int MAX_TOOLTIPS  = 256;
    static constexpr int MAX_OBSERVERS = 16;

    // Registry ─────────────────────────────────────────────────

    bool registerTooltip(const TooltipEntry& entry) {
        if (!entry.isValid()) return false;
        if (findTooltip(entry.id)) return false;
        if ((int)m_entries.size() >= MAX_TOOLTIPS) return false;
        m_entries.push_back(entry);
        return true;
    }

    bool unregisterTooltip(const std::string& id) {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
            [&](const TooltipEntry& e) { return e.id == id; });
        if (it == m_entries.end()) return false;
        // Hide if currently shown
        if (m_state.entryId == id && m_state.visible)
            hideInternal(id);
        m_entries.erase(it);
        return true;
    }

    bool isRegistered(const std::string& id) const {
        return findTooltip(id) != nullptr;
    }

    const TooltipEntry* findTooltip(const std::string& id) const {
        for (auto& e : m_entries)
            if (e.id == id) return &e;
        return nullptr;
    }

    // Show / hide ──────────────────────────────────────────────

    bool show(const std::string& id) {
        const TooltipEntry* entry = findTooltip(id);
        if (!entry || !entry->enabled) return false;
        // Hide any currently visible tooltip first
        if (m_state.visible && m_state.entryId != id)
            hideInternal(m_state.entryId);
        m_state.entryId       = id;
        m_state.visible       = true;
        m_state.showTimestamp = ++m_clock;
        notifyObservers(*entry, true);
        return true;
    }

    bool hide(const std::string& id) {
        if (!m_state.visible || m_state.entryId != id) return false;
        return hideInternal(id);
    }

    void hideAll() {
        if (m_state.visible)
            hideInternal(m_state.entryId);
    }

    bool isVisible(const std::string& id) const {
        return m_state.visible && m_state.entryId == id;
    }

    const TooltipState& currentVisible() const { return m_state; }

    // Enable / disable individual entries ──────────────────────

    bool enableTooltip(const std::string& id) {
        for (auto& e : m_entries)
            if (e.id == id) { e.enabled = true; return true; }
        return false;
    }

    bool disableTooltip(const std::string& id) {
        for (auto& e : m_entries) {
            if (e.id == id) {
                e.enabled = false;
                if (m_state.visible && m_state.entryId == id)
                    hideInternal(id);
                return true;
            }
        }
        return false;
    }

    // Bulk access ──────────────────────────────────────────────

    std::vector<std::string> allTooltipIds() const {
        std::vector<std::string> ids;
        ids.reserve(m_entries.size());
        for (auto& e : m_entries) ids.push_back(e.id);
        return ids;
    }

    int tooltipCount() const { return (int)m_entries.size(); }

    void clear() {
        if (m_state.visible) hideInternal(m_state.entryId);
        m_entries.clear();
        m_state = {};
    }

    // Observers ────────────────────────────────────────────────

    uint32_t addObserver(Observer cb) {
        if (!cb || (int)m_observers.size() >= MAX_OBSERVERS) return 0;
        uint32_t id = ++m_nextObserverId;
        m_observers.push_back({id, std::move(cb)});
        return id;
    }

    void removeObserver(uint32_t id) {
        m_observers.erase(
            std::remove_if(m_observers.begin(), m_observers.end(),
                [id](const ObserverEntry& e) { return e.id == id; }),
            m_observers.end());
    }

    void clearObservers() { m_observers.clear(); }

private:
    struct ObserverEntry { uint32_t id; Observer cb; };

    bool hideInternal(const std::string& id) {
        const TooltipEntry* entry = findTooltip(id);
        m_state.visible = false;
        if (entry) notifyObservers(*entry, false);
        return true;
    }

    void notifyObservers(const TooltipEntry& entry, bool visible) {
        for (auto& e : m_observers) e.cb(entry, visible);
    }

    std::vector<TooltipEntry>  m_entries;
    TooltipState               m_state;
    uint64_t                   m_clock          = 0;
    uint32_t                   m_nextObserverId = 0;
    std::vector<ObserverEntry> m_observers;
};

} // namespace NF
