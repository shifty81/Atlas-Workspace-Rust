#pragma once
// NF::Editor — Workspace layout manager v1: layout preset registration and panel slot authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Wlmv1LayoutMode    : uint8_t { Single, Split2H, Split2V, Split3, Grid4, Custom };
enum class Wlmv1PanelSlotEdge : uint8_t { Left, Right, Top, Bottom, Center, Float };
enum class Wlmv1LayoutState   : uint8_t { Draft, Active, Saved, Locked };

inline const char* wlmv1LayoutModeName(Wlmv1LayoutMode m) {
    switch (m) {
        case Wlmv1LayoutMode::Single:  return "Single";
        case Wlmv1LayoutMode::Split2H: return "Split2H";
        case Wlmv1LayoutMode::Split2V: return "Split2V";
        case Wlmv1LayoutMode::Split3:  return "Split3";
        case Wlmv1LayoutMode::Grid4:   return "Grid4";
        case Wlmv1LayoutMode::Custom:  return "Custom";
    }
    return "Unknown";
}

inline const char* wlmv1PanelSlotEdgeName(Wlmv1PanelSlotEdge e) {
    switch (e) {
        case Wlmv1PanelSlotEdge::Left:   return "Left";
        case Wlmv1PanelSlotEdge::Right:  return "Right";
        case Wlmv1PanelSlotEdge::Top:    return "Top";
        case Wlmv1PanelSlotEdge::Bottom: return "Bottom";
        case Wlmv1PanelSlotEdge::Center: return "Center";
        case Wlmv1PanelSlotEdge::Float:  return "Float";
    }
    return "Unknown";
}

inline const char* wlmv1LayoutStateName(Wlmv1LayoutState s) {
    switch (s) {
        case Wlmv1LayoutState::Draft:  return "Draft";
        case Wlmv1LayoutState::Active: return "Active";
        case Wlmv1LayoutState::Saved:  return "Saved";
        case Wlmv1LayoutState::Locked: return "Locked";
    }
    return "Unknown";
}

struct Wlmv1PanelSlot {
    std::string         panelId;
    Wlmv1PanelSlotEdge  edge      = Wlmv1PanelSlotEdge::Center;
    float               weight    = 1.f;
    bool                visible   = true;

    [[nodiscard]] bool isValid() const { return !panelId.empty() && weight > 0.f; }
};

struct Wlmv1LayoutPreset {
    uint64_t              id      = 0;
    std::string           name;
    Wlmv1LayoutMode       mode    = Wlmv1LayoutMode::Single;
    Wlmv1LayoutState      state   = Wlmv1LayoutState::Draft;
    std::vector<Wlmv1PanelSlot> slots;
    bool                  isDefault = false;

    [[nodiscard]] bool isValid()  const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive() const { return state == Wlmv1LayoutState::Active; }
    [[nodiscard]] bool isLocked() const { return state == Wlmv1LayoutState::Locked; }

    bool addSlot(const Wlmv1PanelSlot& slot) {
        if (!slot.isValid()) return false;
        for (const auto& s : slots) if (s.panelId == slot.panelId) return false;
        slots.push_back(slot);
        return true;
    }

    bool removeSlot(const std::string& panelId) {
        for (auto it = slots.begin(); it != slots.end(); ++it) {
            if (it->panelId == panelId) { slots.erase(it); return true; }
        }
        return false;
    }
};

using Wlmv1ChangeCallback = std::function<void(uint64_t)>;

class WorkspaceLayoutManagerV1 {
public:
    static constexpr size_t MAX_PRESETS = 64;

    bool addPreset(const Wlmv1LayoutPreset& preset) {
        if (!preset.isValid()) return false;
        for (const auto& p : m_presets) if (p.id == preset.id) return false;
        if (m_presets.size() >= MAX_PRESETS) return false;
        m_presets.push_back(preset);
        return true;
    }

    bool removePreset(uint64_t id) {
        for (auto it = m_presets.begin(); it != m_presets.end(); ++it) {
            if (it->id == id) {
                if (m_activeId == id) m_activeId = 0;
                m_presets.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] Wlmv1LayoutPreset* findPreset(uint64_t id) {
        for (auto& p : m_presets) if (p.id == id) return &p;
        return nullptr;
    }

    bool activate(uint64_t id) {
        auto* p = findPreset(id);
        if (!p) return false;
        if (p->isLocked()) return false;
        // deactivate old
        for (auto& pr : m_presets)
            if (pr.state == Wlmv1LayoutState::Active) pr.state = Wlmv1LayoutState::Saved;
        p->state  = Wlmv1LayoutState::Active;
        m_activeId = id;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setState(uint64_t id, Wlmv1LayoutState state) {
        auto* p = findPreset(id);
        if (!p) return false;
        p->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setDefault(uint64_t id, bool isDefault) {
        auto* p = findPreset(id);
        if (!p) return false;
        p->isDefault = isDefault;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool addSlot(uint64_t presetId, const Wlmv1PanelSlot& slot) {
        auto* p = findPreset(presetId);
        return p && p->addSlot(slot);
    }

    bool removeSlot(uint64_t presetId, const std::string& panelId) {
        auto* p = findPreset(presetId);
        return p && p->removeSlot(panelId);
    }

    [[nodiscard]] uint64_t activeId()      const { return m_activeId; }
    [[nodiscard]] size_t   presetCount()   const { return m_presets.size(); }
    [[nodiscard]] size_t   defaultCount()  const {
        size_t c = 0; for (const auto& p : m_presets) if (p.isDefault)  ++c; return c;
    }
    [[nodiscard]] size_t   lockedCount()   const {
        size_t c = 0; for (const auto& p : m_presets) if (p.isLocked()) ++c; return c;
    }
    [[nodiscard]] size_t   countByMode(Wlmv1LayoutMode mode) const {
        size_t c = 0; for (const auto& p : m_presets) if (p.mode == mode) ++c; return c;
    }

    void setOnChange(Wlmv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Wlmv1LayoutPreset> m_presets;
    uint64_t                       m_activeId = 0;
    Wlmv1ChangeCallback            m_onChange;
};

} // namespace NF
