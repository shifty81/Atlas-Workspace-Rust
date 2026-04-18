#pragma once
// NF::Editor — Workspace selection manager v1: selection context and multi-type selection authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Wsmv1SelectionMode : uint8_t { Single, Multi, Box, Brush, Invert };
enum class Wsmv1ObjectType    : uint8_t { Entity, Asset, Panel, Component, Material, Node };
enum class Wsmv1SelectionFlag : uint8_t { None, Locked, Pinned, Highlighted };

inline const char* wsmv1SelectionModeName(Wsmv1SelectionMode m) {
    switch (m) {
        case Wsmv1SelectionMode::Single: return "Single";
        case Wsmv1SelectionMode::Multi:  return "Multi";
        case Wsmv1SelectionMode::Box:    return "Box";
        case Wsmv1SelectionMode::Brush:  return "Brush";
        case Wsmv1SelectionMode::Invert: return "Invert";
    }
    return "Unknown";
}

inline const char* wsmv1ObjectTypeName(Wsmv1ObjectType t) {
    switch (t) {
        case Wsmv1ObjectType::Entity:    return "Entity";
        case Wsmv1ObjectType::Asset:     return "Asset";
        case Wsmv1ObjectType::Panel:     return "Panel";
        case Wsmv1ObjectType::Component: return "Component";
        case Wsmv1ObjectType::Material:  return "Material";
        case Wsmv1ObjectType::Node:      return "Node";
    }
    return "Unknown";
}

struct Wsmv1SelectionItem {
    uint64_t            id         = 0;
    std::string         label;
    Wsmv1ObjectType     objectType = Wsmv1ObjectType::Entity;
    Wsmv1SelectionFlag  flag       = Wsmv1SelectionFlag::None;
    std::string         contextId;

    [[nodiscard]] bool isValid()       const { return id != 0; }
    [[nodiscard]] bool isLocked()      const { return flag == Wsmv1SelectionFlag::Locked; }
    [[nodiscard]] bool isPinned()      const { return flag == Wsmv1SelectionFlag::Pinned; }
    [[nodiscard]] bool isHighlighted() const { return flag == Wsmv1SelectionFlag::Highlighted; }
};

using Wsmv1ChangeCallback = std::function<void()>;

class WorkspaceSelectionManagerV1 {
public:
    static constexpr size_t MAX_SELECTION = 4096;

    bool select(const Wsmv1SelectionItem& item) {
        if (!item.isValid()) return false;
        for (const auto& s : m_selection) if (s.id == item.id) return false;
        if (m_selection.size() >= MAX_SELECTION) return false;
        m_selection.push_back(item);
        if (m_onChange) m_onChange();
        return true;
    }

    bool deselect(uint64_t id) {
        for (auto it = m_selection.begin(); it != m_selection.end(); ++it) {
            if (it->id == id) { m_selection.erase(it); if (m_onChange) m_onChange(); return true; }
        }
        return false;
    }

    void clearSelection() {
        if (!m_selection.empty()) { m_selection.clear(); if (m_onChange) m_onChange(); }
    }

    [[nodiscard]] Wsmv1SelectionItem* findItem(uint64_t id) {
        for (auto& s : m_selection) if (s.id == id) return &s;
        return nullptr;
    }

    bool setFlag(uint64_t id, Wsmv1SelectionFlag flag) {
        auto* s = findItem(id);
        if (!s) return false;
        s->flag = flag;
        if (m_onChange) m_onChange();
        return true;
    }

    void setMode(Wsmv1SelectionMode mode) { m_mode = mode; }
    [[nodiscard]] Wsmv1SelectionMode mode() const { return m_mode; }

    [[nodiscard]] size_t selectionCount()   const { return m_selection.size(); }
    [[nodiscard]] bool   hasSelection()     const { return !m_selection.empty(); }
    [[nodiscard]] size_t lockedCount()      const {
        size_t c = 0; for (const auto& s : m_selection) if (s.isLocked())      ++c; return c;
    }
    [[nodiscard]] size_t pinnedCount()      const {
        size_t c = 0; for (const auto& s : m_selection) if (s.isPinned())      ++c; return c;
    }
    [[nodiscard]] size_t highlightedCount() const {
        size_t c = 0; for (const auto& s : m_selection) if (s.isHighlighted()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(Wsmv1ObjectType type) const {
        size_t c = 0; for (const auto& s : m_selection) if (s.objectType == type) ++c; return c;
    }

    void setOnChange(Wsmv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Wsmv1SelectionItem> m_selection;
    Wsmv1SelectionMode              m_mode = Wsmv1SelectionMode::Single;
    Wsmv1ChangeCallback             m_onChange;
};

} // namespace NF
