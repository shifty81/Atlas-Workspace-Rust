#include "NF/UI/AtlasUI/Widgets/ContextMenu.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"

namespace NF::UI::AtlasUI {

ContextMenu::ContextMenu(std::vector<MenuItem> items)
    : m_items(std::move(items)) {}

void ContextMenu::open(NF::Vec2 position) {
    m_position = position;
    m_open = true;
    m_hoveredIndex = -1;
}

void ContextMenu::close() {
    m_open = false;
    m_hoveredIndex = -1;
}

void ContextMenu::measure(ILayoutContext& context) {
    m_menuWidth = m_minWidth;
    for (const auto& item : m_items) {
        if (item.isSeparator()) continue;
        auto textSize = context.measureText(item.label, 14.f);
        float needed = textSize.x + Theme::Spacing::Large * 2.f;
        if (!item.shortcutHint.empty()) {
            auto hintSize = context.measureText(item.shortcutHint, 12.f);
            needed += hintSize.x + Theme::Spacing::Large;
        }
        if (needed > m_menuWidth) m_menuWidth = needed;
    }

    float totalHeight = 0.f;
    for (const auto& item : m_items) {
        totalHeight += item.isSeparator() ? m_separatorHeight : m_itemHeight;
    }
    m_bounds = {m_position.x, m_position.y, m_menuWidth, totalHeight};
}

void ContextMenu::paint(IPaintContext& context) {
    if (!m_open || !m_visible) return;

    context.fillRect(m_bounds, Theme::ColorToken::Surface);
    context.drawRect(m_bounds, Theme::ColorToken::Border);

    float y = m_bounds.y;
    for (size_t i = 0; i < m_items.size(); ++i) {
        const auto& item = m_items[i];
        if (item.isSeparator()) {
            NF::Rect sepRect = {m_bounds.x + Theme::Spacing::Small, y + m_separatorHeight * 0.5f - 0.5f,
                                m_bounds.w - Theme::Spacing::Small * 2.f, 1.f};
            context.fillRect(sepRect, Theme::ColorToken::Border);
            y += m_separatorHeight;
            continue;
        }

        NF::Rect itemRect = {m_bounds.x, y, m_bounds.w, m_itemHeight};
        if (static_cast<int>(i) == m_hoveredIndex && item.enabled) {
            context.fillRect(itemRect, Theme::ColorToken::AccentHover);
        }

        Color textColor = item.enabled ? Theme::ColorToken::Text : Theme::ColorToken::TextMuted;
        context.drawText(insetRect(itemRect, Theme::Spacing::Medium, Theme::Spacing::Tiny),
                         item.label, 0, textColor);

        if (!item.shortcutHint.empty()) {
            NF::Rect hintRect = itemRect;
            hintRect.x = itemRect.x + itemRect.w - 80.f;
            hintRect.w = 80.f - Theme::Spacing::Medium;
            context.drawText(insetRect(hintRect, 0.f, Theme::Spacing::Tiny),
                             item.shortcutHint, 0, Theme::ColorToken::TextMuted);
        }

        y += m_itemHeight;
    }
}

bool ContextMenu::handleInput(IInputContext& context) {
    if (!m_open || !m_visible) return false;

    m_hoveredIndex = -1;
    float y = m_bounds.y;
    for (size_t i = 0; i < m_items.size(); ++i) {
        const auto& item = m_items[i];
        if (item.isSeparator()) {
            y += m_separatorHeight;
            continue;
        }

        NF::Rect itemRect = {m_bounds.x, y, m_bounds.w, m_itemHeight};
        if (rectContains(itemRect, context.mousePosition())) {
            m_hoveredIndex = static_cast<int>(i);
            if (context.primaryDown() && item.enabled && item.action) {
                item.action();
                close();
                return true;
            }
        }
        y += m_itemHeight;
    }

    if (context.primaryDown() && !rectContains(m_bounds, context.mousePosition())) {
        close();
        return false;
    }

    return rectContains(m_bounds, context.mousePosition());
}

} // namespace NF::UI::AtlasUI
