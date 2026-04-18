#pragma once

#include "NF/UI/AtlasUI/PanelBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include "NF/UI/AtlasUI/Widgets/PropertyGrid.h"
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

/// AtlasUI InspectorPanel — displays and edits selected entity properties.
/// Hosts a PropertyGrid child so property rows are interactively editable.
class InspectorPanel final : public PanelBase {
public:
    using ChangeCallback = std::function<void(const std::string& name, const PropertyValue& value)>;

    InspectorPanel()
        : PanelBase("atlas.inspector", "Inspector")
        , m_grid(std::make_unique<PropertyGrid>()) {
        m_grid->setOnChange([this](const std::string& name, const PropertyValue& val) {
            if (m_onChange) m_onChange(name, val);
        });
    }

    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

    void setSelectedEntityId(int id) {
        if (m_selectedEntityId == id) return;
        m_selectedEntityId = id;
        rebuildGrid();
    }
    [[nodiscard]] int selectedEntityId() const { return m_selectedEntityId; }

    void setTransform(float x, float y, float z) {
        m_transformX = x;
        m_transformY = y;
        m_transformZ = z;
        rebuildGrid();
    }

    [[nodiscard]] float transformX() const { return m_transformX; }
    [[nodiscard]] float transformY() const { return m_transformY; }
    [[nodiscard]] float transformZ() const { return m_transformZ; }

    struct PropertyEntry {
        std::string label;
        std::string value;
    };

    void clearProperties() { m_properties.clear(); rebuildGrid(); }
    void addProperty(const std::string& label, const std::string& value) {
        m_properties.push_back({label, value});
        rebuildGrid();
    }
    [[nodiscard]] const std::vector<PropertyEntry>& properties() const { return m_properties; }

    /// Register a callback that is invoked whenever the user edits a property.
    void setOnChange(ChangeCallback cb) { m_onChange = std::move(cb); }

    void arrange(const NF::Rect& bounds) override {
        PanelBase::arrange(bounds);
        // Title bar (22 px) + entity-ID row (20 px) + separator/header (26 px) = 68 px.
        // The grid fills the remaining height with a 4 px horizontal inset each side.
        static constexpr float kGridTopOffset  = 68.f;
        static constexpr float kGridSideInset  =  4.f;
        NF::Rect gridBounds = {bounds.x + kGridSideInset,
                               bounds.y + kGridTopOffset,
                               std::max(0.f, bounds.w - kGridSideInset * 2.f - kScrollW),
                               std::max(0.f, bounds.h - kGridTopOffset - kGridSideInset)};
        m_grid->arrange(gridBounds);
    }

private:
    int m_selectedEntityId = -1;
    float m_transformX = 0.f;
    float m_transformY = 0.f;
    float m_transformZ = 0.f;
    std::vector<PropertyEntry> m_properties;
    std::unique_ptr<PropertyGrid> m_grid;
    ChangeCallback m_onChange;

    float m_scrollOffset  = 0.f;
    bool  m_thumbDragging = false;
    float m_dragStartY    = 0.f;
    float m_dragStartOff  = 0.f;

    static constexpr float kScrollW   = 10.f;
    static constexpr float kHeaderOff = 68.f; // content starts below this many px

    [[nodiscard]] float contentHeight() const {
        // PropertyGrid measures its own height; mirror the calculation.
        return static_cast<float>(m_grid->visibleRowCount()) * 20.f;
    }
    [[nodiscard]] float viewportHeight() const {
        return std::max(0.f, m_bounds.h - kHeaderOff);
    }
    [[nodiscard]] float maxScroll() const {
        return std::max(0.f, contentHeight() - viewportHeight());
    }
    [[nodiscard]] float thumbHeight() const {
        float vpH = viewportHeight();
        float ctH = contentHeight();
        if (ctH <= vpH) return vpH;
        return std::max(20.f, vpH * std::min(1.f, vpH / ctH));
    }
    [[nodiscard]] float thumbY() const {
        float trackH = viewportHeight() - thumbHeight();
        float frac   = (maxScroll() > 0.f) ? (m_scrollOffset / maxScroll()) : 0.f;
        return (m_bounds.y + kHeaderOff) + trackH * frac;
    }
    void clampScroll() {
        m_scrollOffset = std::max(0.f, std::min(m_scrollOffset, maxScroll()));
    }

    /// Rebuild the PropertyGrid items from the current transform and properties.
    void rebuildGrid();
};

} // namespace NF::UI::AtlasUI
