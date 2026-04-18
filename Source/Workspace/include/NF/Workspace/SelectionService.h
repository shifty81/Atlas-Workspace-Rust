#pragma once
// NF::Workspace — Selection service and dock layout primitives
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include <cstdint>
#include <set>
#include <string>

namespace NF {

// ── Dock layout ──────────────────────────────────────────────────

enum class DockSlot : uint8_t {
    Left,
    Right,
    Top,
    Bottom,
    Center
};

struct DockPanel {
    std::string name;
    DockSlot slot = DockSlot::Center;
    Rect bounds;
    bool visible = true;
};

// ── Selection Service ────────────────────────────────────────────

class SelectionService {
public:
    void select(EntityID id) {
        m_selection.insert(id);
        m_primary = id;
        ++m_version;
    }

    void deselect(EntityID id) {
        m_selection.erase(id);
        if (m_primary == id) {
            m_primary = m_selection.empty() ? INVALID_ENTITY : *m_selection.begin();
        }
        ++m_version;
    }

    void toggleSelect(EntityID id) {
        if (isSelected(id)) deselect(id);
        else select(id);
    }

    void clearSelection() {
        m_selection.clear();
        m_primary = INVALID_ENTITY;
        ++m_version;
    }

    void selectExclusive(EntityID id) {
        m_selection.clear();
        m_selection.insert(id);
        m_primary = id;
        ++m_version;
    }

    [[nodiscard]] bool isSelected(EntityID id) const {
        return m_selection.count(id) > 0;
    }

    [[nodiscard]] EntityID primarySelection() const { return m_primary; }
    [[nodiscard]] const std::set<EntityID>& selection() const { return m_selection; }
    [[nodiscard]] size_t selectionCount() const { return m_selection.size(); }
    [[nodiscard]] bool hasSelection() const { return !m_selection.empty(); }
    [[nodiscard]] uint32_t version() const { return m_version; }

private:
    std::set<EntityID> m_selection;
    EntityID m_primary = INVALID_ENTITY;
    uint32_t m_version = 0;
};

// ── Content Browser ──────────────────────────────────────────────

} // namespace NF
