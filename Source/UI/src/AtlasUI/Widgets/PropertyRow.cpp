#include "NF/UI/AtlasUI/Widgets/PropertyRow.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"

namespace NF::UI::AtlasUI {

PropertyRow::PropertyRow(std::string label, std::shared_ptr<IWidget> valueWidget)
    : m_label(std::move(label)), m_valueWidget(std::move(valueWidget)) {}

void PropertyRow::measure(ILayoutContext& context) {
    m_bounds.h = 24.f;
    if (m_valueWidget) m_valueWidget->measure(context);
}

void PropertyRow::arrange(const NF::Rect& bounds) {
    m_bounds = bounds;
    m_bounds.h = 24.f;

    if (m_valueWidget) {
        NF::Rect valueRect = {
            bounds.x + m_labelWidth,
            bounds.y,
            std::max(0.f, bounds.w - m_labelWidth),
            m_bounds.h
        };
        m_valueWidget->arrange(valueRect);
    }
}

void PropertyRow::paint(IPaintContext& context) {
    if (!m_visible) return;

    NF::Rect labelRect = {m_bounds.x, m_bounds.y, m_labelWidth, m_bounds.h};
    context.drawText(insetRect(labelRect, Theme::Spacing::Small, Theme::Spacing::Tiny),
                     m_label, 0, Theme::ColorToken::TextMuted);

    if (m_valueWidget && m_valueWidget->isVisible()) {
        m_valueWidget->paint(context);
    }
}

bool PropertyRow::handleInput(IInputContext& context) {
    if (!m_visible) return false;
    if (m_valueWidget && m_valueWidget->isVisible()) {
        return m_valueWidget->handleInput(context);
    }
    return false;
}

} // namespace NF::UI::AtlasUI
