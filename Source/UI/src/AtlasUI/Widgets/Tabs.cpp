#include "NF/UI/AtlasUI/Widgets/Tabs.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"

namespace NF::UI::AtlasUI {

void Tabs::addTab(std::string title, std::shared_ptr<IWidget> content) {
    m_tabs.push_back({std::move(title), std::move(content)});
    m_children.clear();
    for (auto& tab : m_tabs) {
        if (tab.content) m_children.push_back(tab.content);
    }
}

void Tabs::measure(ILayoutContext& context) {
    for (auto& tab : m_tabs) {
        if (tab.content) tab.content->measure(context);
    }
}

void Tabs::arrange(const NF::Rect& bounds) {
    m_bounds = bounds;
    if (m_tabs.empty()) return;
    NF::Rect contentRect = bounds;
    contentRect.y += m_tabHeight;
    contentRect.h = std::max(0.f, contentRect.h - m_tabHeight);
    for (size_t i = 0; i < m_tabs.size(); ++i) {
        if (m_tabs[i].content) {
            m_tabs[i].content->setVisible(i == m_activeIndex);
            m_tabs[i].content->arrange(contentRect);
        }
    }
}

void Tabs::paint(IPaintContext& context) {
    if (!m_visible) return;
    context.fillRect({m_bounds.x, m_bounds.y, m_bounds.w, m_tabHeight}, Theme::ColorToken::Surface);
    context.drawRect(m_bounds, Theme::ColorToken::Border);
    for (size_t i = 0; i < m_tabs.size(); ++i) {
        const NF::Rect rect = tabRect(i);
        context.fillRect(rect, i == m_activeIndex ? Theme::ColorToken::Accent : Theme::ColorToken::SurfaceAlt);
        context.drawRect(rect, Theme::ColorToken::Border);
        context.drawText({rect.x + 8.f, rect.y + 6.f, rect.w - 16.f, rect.h - 12.f}, m_tabs[i].title, 0, Theme::ColorToken::Text);
    }
    if (m_activeIndex < m_tabs.size() && m_tabs[m_activeIndex].content && m_tabs[m_activeIndex].content->isVisible()) {
        m_tabs[m_activeIndex].content->paint(context);
    }
}

bool Tabs::handleInput(IInputContext& context) {
    if (!m_visible) return false;
    for (size_t i = 0; i < m_tabs.size(); ++i) {
        if (rectContains(tabRect(i), context.mousePosition()) && context.primaryDown()) {
            setActiveIndex(i);
            return true;
        }
    }
    if (m_activeIndex < m_tabs.size() && m_tabs[m_activeIndex].content) {
        return m_tabs[m_activeIndex].content->handleInput(context);
    }
    return false;
}

void Tabs::setActiveIndex(size_t index) {
    if (index < m_tabs.size()) m_activeIndex = index;
}

NF::Rect Tabs::tabRect(size_t index) const {
    const float width = m_tabs.empty() ? m_bounds.w : m_bounds.w / static_cast<float>(m_tabs.size());
    return {m_bounds.x + width * static_cast<float>(index), m_bounds.y, width, m_tabHeight};
}

} // namespace NF::UI::AtlasUI
