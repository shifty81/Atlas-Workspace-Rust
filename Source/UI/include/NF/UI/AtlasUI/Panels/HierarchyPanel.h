#pragma once

#include "NF/UI/AtlasUI/PanelBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include <functional>
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

/// AtlasUI HierarchyPanel — displays entity tree with search filter.
/// Replaces the legacy NF::Editor::HierarchyPanel for the AtlasUI framework.
class HierarchyPanel final : public PanelBase {
public:
    HierarchyPanel()
        : PanelBase("atlas.hierarchy", "Hierarchy") {}

    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

    void setSearchFilter(const std::string& filter) { m_searchFilter = filter; }
    [[nodiscard]] const std::string& searchFilter() const { return m_searchFilter; }

    struct EntityEntry {
        int id = 0;
        std::string name;
        bool selected = false;
        int depth = 0;
    };

    void clearEntities() { m_entities.clear(); }
    void addEntity(int id, const std::string& name, bool selected = false, int depth = 0) {
        m_entities.push_back({id, name, selected, depth});
    }
    [[nodiscard]] const std::vector<EntityEntry>& entities() const { return m_entities; }
    [[nodiscard]] size_t entityCount() const { return m_entities.size(); }

    /// Called when the user clicks an entity row.  id is the entity's 1-based ID.
    using SelectCallback = std::function<void(int entityId)>;
    void setOnSelect(SelectCallback cb) { m_onSelect = std::move(cb); }

private:
    static constexpr float kScrollBarW = 10.f;
    static constexpr float kRowH       = 18.f;
    static constexpr float kHeaderH    = 22.f;

    std::string m_searchFilter;
    std::vector<EntityEntry> m_entities;
    SelectCallback m_onSelect;

    float m_scrollOffset   = 0.f;
    bool  m_thumbDragging  = false;
    float m_dragStartY     = 0.f;
    float m_dragStartOff   = 0.f;

    [[nodiscard]] float contentHeight() const;
    [[nodiscard]] float viewportHeight() const { return std::max(0.f, m_bounds.h - kHeaderH); }
    [[nodiscard]] float maxScroll() const { return std::max(0.f, contentHeight() - viewportHeight()); }
    [[nodiscard]] float thumbHeight() const;
    [[nodiscard]] float thumbY() const;
    void clampScroll() { m_scrollOffset = std::max(0.f, std::min(m_scrollOffset, maxScroll())); }
};

} // namespace NF::UI::AtlasUI
