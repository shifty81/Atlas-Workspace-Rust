#include "NF/UI/AtlasUI/Widgets/MenuBar.h"

namespace NF::UI::AtlasUI {

MenuBarCategory& MenuBar::addCategory(std::string label) {
    m_categories.push_back({std::move(label), {}});
    return m_categories.back();
}

MenuBarCategory* MenuBar::findCategory(const std::string& label) {
    for (auto& cat : m_categories) {
        if (cat.label == label) return &cat;
    }
    return nullptr;
}

void MenuBar::measure(ILayoutContext& context) {
    m_catWidths.resize(m_categories.size());
    for (size_t i = 0; i < m_categories.size(); ++i) {
        auto sz = context.measureText(m_categories[i].label, 13.f);
        m_catWidths[i] = sz.x + kCategoryPadX * 2.f;
    }
    m_bounds.h = kBarHeight;
}

void MenuBar::arrange(const NF::Rect& bounds) {
    m_bounds = bounds;
    m_bounds.h = kBarHeight;
}

void MenuBar::paint(IPaintContext& context) {
    if (!m_visible) return;

    // Bar background
    context.fillRect(m_bounds, Theme::ColorToken::SurfaceAlt);

    // Category labels
    for (size_t i = 0; i < m_categories.size(); ++i) {
        NF::Rect catR = categoryRect(i);
        bool open = (m_openIdx == static_cast<int>(i));
        bool hovered = (m_hoveredCat == static_cast<int>(i));
        if (open || hovered) {
            context.fillRect(catR, open ? Theme::ColorToken::Accent : Theme::ColorToken::AccentHover);
        }
        context.drawText({catR.x + kCategoryPadX, catR.y + 7.f, catR.w - kCategoryPadX * 2.f, 14.f},
                         m_categories[i].label, 0, Theme::ColorToken::Text);
    }

    // Bottom border
    context.fillRect({m_bounds.x, m_bounds.y + m_bounds.h - 1.f, m_bounds.w, 1.f},
                     Theme::ColorToken::Border);

    // Open drop-down
    if (m_openIdx >= 0 && m_openIdx < static_cast<int>(m_categories.size())) {
        size_t ci = static_cast<size_t>(m_openIdx);
        NF::Rect dd = dropDownRect(ci);
        context.fillRect(dd, Theme::ColorToken::Surface);
        context.drawRect(dd, Theme::ColorToken::Border);

        for (size_t ei = 0; ei < m_categories[ci].entries.size(); ++ei) {
            const auto& entry = m_categories[ci].entries[ei];
            NF::Rect er = dropEntryRect(ci, ei);
            if (entry.separator) {
                context.fillRect({er.x + 4.f, er.y + er.h * 0.5f, er.w - 8.f, 1.f},
                                 Theme::ColorToken::Border);
                continue;
            }
            bool entHov = (m_hoveredEntry == static_cast<int>(ei));
            if (entHov) {
                context.fillRect(er, Theme::ColorToken::AccentHover);
            }
            context.drawText({er.x + 8.f, er.y + 4.f, er.w - 16.f, 14.f},
                             entry.label, 0, Theme::ColorToken::Text);
        }
    }
}

bool MenuBar::handleInput(IInputContext& context) {
    if (!m_visible) return false;

    NF::Vec2 mp = context.mousePosition();

    // Hit-test open drop-down first
    if (m_openIdx >= 0 && m_openIdx < static_cast<int>(m_categories.size())) {
        size_t ci = static_cast<size_t>(m_openIdx);
        NF::Rect dd = dropDownRect(ci);
        if (rectContains(dd, mp)) {
            m_hoveredEntry = -1;
            for (size_t ei = 0; ei < m_categories[ci].entries.size(); ++ei) {
                NF::Rect er = dropEntryRect(ci, ei);
                if (rectContains(er, mp)) {
                    m_hoveredEntry = static_cast<int>(ei);
                    if (context.primaryDown()) {
                        const auto& entry = m_categories[ci].entries[ei];
                        if (!entry.separator && entry.action) {
                            entry.action();
                        }
                        m_openIdx = -1;
                        m_hoveredEntry = -1;
                        return true;
                    }
                    return true;
                }
            }
            return true;
        }
        // Click outside drop-down closes it
        if (context.primaryDown()) {
            m_openIdx = -1;
            m_hoveredEntry = -1;
        }
    }

    // Hit-test category bar
    m_hoveredCat = -1;
    for (size_t i = 0; i < m_categories.size(); ++i) {
        NF::Rect catR = categoryRect(i);
        if (rectContains(catR, mp)) {
            m_hoveredCat = static_cast<int>(i);
            if (context.primaryDown()) {
                m_openIdx = (m_openIdx == static_cast<int>(i)) ? -1 : static_cast<int>(i);
                return true;
            }
            return true;
        }
    }
    return false;
}

NF::Rect MenuBar::categoryRect(size_t idx) const {
    float x = m_bounds.x;
    for (size_t i = 0; i < idx && i < m_catWidths.size(); ++i) {
        x += m_catWidths[i];
    }
    float w = idx < m_catWidths.size() ? m_catWidths[idx] : 80.f;
    return {x, m_bounds.y, w, kBarHeight};
}

NF::Rect MenuBar::dropDownRect(size_t catIdx) const {
    NF::Rect catR = categoryRect(catIdx);
    float ddW = 200.f;
    // Minimum width to fit longest item
    if (catIdx < m_categories.size()) {
        for (const auto& e : m_categories[catIdx].entries) {
            float ew = static_cast<float>(e.label.size()) * 8.f + 24.f;
            if (ew > ddW) ddW = ew;
        }
    }
    float ddH = catIdx < m_categories.size()
                ? static_cast<float>(m_categories[catIdx].entries.size()) * kItemHeight
                : 0.f;
    return {catR.x, catR.y + kBarHeight, ddW, ddH};
}

NF::Rect MenuBar::dropEntryRect(size_t catIdx, size_t entryIdx) const {
    NF::Rect dd = dropDownRect(catIdx);
    return {dd.x, dd.y + static_cast<float>(entryIdx) * kItemHeight, dd.w, kItemHeight};
}

} // namespace NF::UI::AtlasUI
