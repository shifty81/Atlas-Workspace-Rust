#include "NF/UI/AtlasUI/Widgets/Toolbar.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"

namespace NF::UI::AtlasUI {

void Toolbar::addItem(std::string label, std::function<void()> action, std::string tooltip) {
    m_items.push_back({std::move(label), std::move(tooltip), std::move(action), false});
}

void Toolbar::addSeparator() {
    m_items.push_back({"", "", nullptr, true});
}

void Toolbar::measure(ILayoutContext& context) {
    m_itemWidths.resize(m_items.size());
    float totalW = 0.f;
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (m_items[i].separator) {
            m_itemWidths[i] = Theme::Spacing::Small;
        } else {
            auto sz = context.measureText(m_items[i].label, 13.f);
            m_itemWidths[i] = sz.x + Theme::Spacing::Large * 2.f;
        }
        totalW += m_itemWidths[i];
    }
    m_bounds.w = totalW;
    m_bounds.h = m_itemHeight;
}

void Toolbar::arrange(const NF::Rect& bounds) {
    m_bounds = bounds;
    m_bounds.h = m_itemHeight;
}

void Toolbar::paint(IPaintContext& context) {
    if (!m_visible) return;
    context.fillRect(m_bounds, Theme::ColorToken::Surface);

    float x = m_bounds.x;
    for (size_t i = 0; i < m_items.size(); ++i) {
        float w = i < m_itemWidths.size() ? m_itemWidths[i] : 80.f;
        const auto& item = m_items[i];
        if (item.separator) {
            NF::Rect sep = {x + 2.f, m_bounds.y + 4.f, 1.f, m_bounds.h - 8.f};
            context.fillRect(sep, Theme::ColorToken::Border);
            x += w;
            continue;
        }
        NF::Rect itemRect = {x, m_bounds.y, w, m_itemHeight};
        if (static_cast<int>(i) == m_hoveredIndex) {
            context.fillRect(itemRect, Theme::ColorToken::AccentHover);
        }
        context.drawText(insetRect(itemRect, Theme::Spacing::Small, Theme::Spacing::Tiny),
                         item.label, 0, Theme::ColorToken::Text);
        x += w;
    }
}

bool Toolbar::handleInput(IInputContext& context) {
    if (!m_visible) return false;
    m_hoveredIndex = -1;
    float x = m_bounds.x;
    for (size_t i = 0; i < m_items.size(); ++i) {
        float w = i < m_itemWidths.size() ? m_itemWidths[i] : 80.f;
        if (m_items[i].separator) {
            x += w;
            continue;
        }
        NF::Rect itemRect = {x, m_bounds.y, w, m_itemHeight};
        if (rectContains(itemRect, context.mousePosition())) {
            m_hoveredIndex = static_cast<int>(i);
            if (context.primaryDown() && m_items[i].action) {
                m_items[i].action();
                return true;
            }
            return true;
        }
        x += w;
    }
    return false;
}

} // namespace NF::UI::AtlasUI
