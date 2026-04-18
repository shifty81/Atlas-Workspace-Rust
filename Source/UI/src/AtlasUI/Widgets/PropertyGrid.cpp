#include "NF/UI/AtlasUI/Widgets/PropertyGrid.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"

#include <string>

namespace NF::UI::AtlasUI {

std::string PropertyValue::asString() const {
    struct Visitor {
        std::string operator()(const std::string& v) const { return v; }
        std::string operator()(float v) const { return std::to_string(v); }
        std::string operator()(int v) const { return std::to_string(v); }
        std::string operator()(bool v) const { return v ? "true" : "false"; }
    };
    return std::visit(Visitor{}, value);
}

void PropertyGrid::rebuildFlat() {
    m_flatNodes.clear();
    flattenItems(m_items, 0);
    m_flatDirty = false;
}

void PropertyGrid::flattenItems(std::vector<PropertyItem>& items, int depth) {
    for (auto& item : items) {
        if (item.isGroup()) {
            m_flatNodes.push_back({&item, depth, true, 0.f});
            if (item.expanded) {
                flattenItems(item.children, depth + 1);
            }
        } else {
            m_flatNodes.push_back({&item, depth, false, 0.f});
        }
    }
}

bool PropertyGrid::toggleGroup(const std::string& name) {
    if (toggleInItems(m_items, name)) {
        m_flatDirty = true;
        return true;
    }
    return false;
}

bool PropertyGrid::toggleInItems(std::vector<PropertyItem>& items, const std::string& name) {
    for (auto& item : items) {
        if (item.name == name && item.isGroup()) {
            item.expanded = !item.expanded;
            return true;
        }
        if (toggleInItems(item.children, name)) return true;
    }
    return false;
}

size_t PropertyGrid::visibleRowCount() const {
    if (m_flatDirty) {
        const_cast<PropertyGrid*>(this)->rebuildFlat();
    }
    return m_flatNodes.size();
}

void PropertyGrid::measure(ILayoutContext&) {
    if (m_flatDirty) rebuildFlat();
    m_bounds.h = static_cast<float>(m_flatNodes.size()) * m_rowHeight;
}

void PropertyGrid::arrange(const NF::Rect& bounds) {
    m_bounds = bounds;
    if (m_flatDirty) rebuildFlat();
    float y = bounds.y;
    for (auto& flat : m_flatNodes) {
        flat.y = y;
        y += m_rowHeight;
    }
}

void PropertyGrid::paint(IPaintContext& context) {
    if (!m_visible) return;
    if (m_flatDirty) rebuildFlat();

    context.pushClip(m_bounds);

    for (size_t i = 0; i < m_flatNodes.size(); ++i) {
        const auto& flat = m_flatNodes[i];
        const auto* item = flat.item;
        float indent = static_cast<float>(flat.depth) * 16.f;

        NF::Rect rowRect = {m_bounds.x, flat.y, m_bounds.w, m_rowHeight};

        // Hover highlight
        if (static_cast<int>(i) == m_hoveredRow) {
            context.fillRect(rowRect, Theme::ColorToken::SurfaceAlt);
        }

        if (flat.isGroupHeader) {
            // Group header
            const char* arrow = item->expanded ? "v" : ">";
            NF::Rect arrowRect = {m_bounds.x + indent + 2.f, flat.y + 4.f, 16.f, 16.f};
            context.drawText(arrowRect, arrow, 0, Theme::ColorToken::Accent);

            NF::Rect labelRect = {m_bounds.x + indent + 20.f, flat.y, m_bounds.w - indent - 20.f, m_rowHeight};
            context.drawText(insetRect(labelRect, Theme::Spacing::Tiny, Theme::Spacing::Tiny),
                             item->name, 0, Theme::ColorToken::Text);
        } else {
            // Property row: label | value
            NF::Rect labelRect = {m_bounds.x + indent, flat.y, m_labelWidth - indent, m_rowHeight};
            context.drawText(insetRect(labelRect, Theme::Spacing::Small, Theme::Spacing::Tiny),
                             item->name, 0, Theme::ColorToken::TextMuted);

            NF::Rect valueRect = {m_bounds.x + m_labelWidth, flat.y,
                                  std::max(0.f, m_bounds.w - m_labelWidth), m_rowHeight};
            Color valueColor = item->readOnly ? Theme::ColorToken::TextMuted : Theme::ColorToken::Text;
            context.drawText(insetRect(valueRect, Theme::Spacing::Small, Theme::Spacing::Tiny),
                             item->value.asString(), 0, valueColor);

            // Divider line between label and value
            NF::Rect divider = {m_bounds.x + m_labelWidth, flat.y, 1.f, m_rowHeight};
            context.fillRect(divider, Theme::ColorToken::Border);
        }
    }

    context.popClip();
}

bool PropertyGrid::handleInput(IInputContext& context) {
    if (!m_visible) return false;
    if (!rectContains(m_bounds, context.mousePosition())) {
        m_hoveredRow = -1;
        return false;
    }
    if (m_flatDirty) rebuildFlat();

    m_hoveredRow = -1;
    for (size_t i = 0; i < m_flatNodes.size(); ++i) {
        NF::Rect rowRect = {m_bounds.x, m_flatNodes[i].y, m_bounds.w, m_rowHeight};
        if (rectContains(rowRect, context.mousePosition())) {
            m_hoveredRow = static_cast<int>(i);
            if (context.primaryPressed()) {
                if (m_flatNodes[i].isGroupHeader) {
                    toggleGroup(m_flatNodes[i].item->name);
                    return true;
                }
            }
            break;
        }
    }

    return m_hoveredRow >= 0;
}

} // namespace NF::UI::AtlasUI
