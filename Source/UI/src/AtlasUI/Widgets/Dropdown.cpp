#include "NF/UI/AtlasUI/Widgets/Dropdown.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"

namespace NF::UI::AtlasUI {

static const std::string kEmptyString;

Dropdown::Dropdown(std::vector<std::string> items)
    : m_items(std::move(items)) {}

void Dropdown::setItems(std::vector<std::string> items) {
    m_items = std::move(items);
    m_selectedIndex = 0;
    m_open = false;
}

void Dropdown::setSelectedIndex(size_t index) {
    if (index < m_items.size()) m_selectedIndex = index;
}

const std::string& Dropdown::selectedText() const {
    if (m_selectedIndex < m_items.size()) return m_items[m_selectedIndex];
    return kEmptyString;
}

void Dropdown::measure(ILayoutContext&) {
    m_bounds.h = 28.f;
}

void Dropdown::paint(IPaintContext& context) {
    if (!m_visible) return;

    context.fillRect(m_bounds, Theme::ColorToken::Surface);
    context.drawRect(m_bounds, Theme::ColorToken::Border);

    NF::Rect textArea = insetRect(m_bounds, Theme::Spacing::Small, Theme::Spacing::Tiny);
    if (m_selectedIndex < m_items.size()) {
        context.drawText(textArea, m_items[m_selectedIndex], 0, Theme::ColorToken::Text);
    }

    if (m_open) {
        float dropY = m_bounds.y + m_bounds.h;
        for (size_t i = 0; i < m_items.size(); ++i) {
            NF::Rect itemRect = {m_bounds.x, dropY, m_bounds.w, m_itemHeight};
            Color bg = (static_cast<int>(i) == m_hoveredItem)
                           ? Theme::ColorToken::AccentHover
                           : Theme::ColorToken::Surface;
            context.fillRect(itemRect, bg);
            context.drawRect(itemRect, Theme::ColorToken::Border);
            context.drawText(insetRect(itemRect, Theme::Spacing::Small, 2.f),
                             m_items[i], 0, Theme::ColorToken::Text);
            dropY += m_itemHeight;
        }
    }
}

bool Dropdown::handleInput(IInputContext& context) {
    if (!m_visible) return false;

    bool insideHeader = rectContains(m_bounds, context.mousePosition());
    if (insideHeader && context.primaryDown()) {
        m_open = !m_open;
        return true;
    }

    if (m_open) {
        m_hoveredItem = -1;
        float dropY = m_bounds.y + m_bounds.h;
        for (size_t i = 0; i < m_items.size(); ++i) {
            NF::Rect itemRect = {m_bounds.x, dropY, m_bounds.w, m_itemHeight};
            if (rectContains(itemRect, context.mousePosition())) {
                m_hoveredItem = static_cast<int>(i);
                if (context.primaryDown()) {
                    m_selectedIndex = i;
                    m_open = false;
                    if (m_onSelect) m_onSelect(i, m_items[i]);
                    return true;
                }
            }
            dropY += m_itemHeight;
        }

        if (context.primaryDown()) {
            m_open = false;
        }
        return true;
    }

    return insideHeader;
}

} // namespace NF::UI::AtlasUI
