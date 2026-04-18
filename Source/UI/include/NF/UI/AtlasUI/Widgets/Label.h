#pragma once

#include "NF/UI/AtlasUI/WidgetTheme.h"
#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include <string>
#include <utility>

namespace NF::UI::AtlasUI {

class Label final : public WidgetBase {
public:
    explicit Label(std::string text) : m_text(std::move(text)) {}

    void setText(std::string text) { m_text = std::move(text); }
    [[nodiscard]] const std::string& text() const { return m_text; }

    void measure(ILayoutContext& context) override {
        m_desiredSize = context.measureText(m_text, m_fontSize);
        m_desiredSize.x += Theme::Spacing::Medium;
        m_desiredSize.y += Theme::Spacing::Small;
    }

    void paint(IPaintContext& context) override {
        if (!m_visible) return;
        context.drawText(insetRect(m_bounds, Theme::Spacing::Small, Theme::Spacing::Tiny), m_text, 0, Theme::ColorToken::Text);
    }

    [[nodiscard]] NF::Vec2 desiredSize() const { return m_desiredSize; }

private:
    std::string m_text;
    float m_fontSize = 14.f;
    NF::Vec2 m_desiredSize{};
};

} // namespace NF::UI::AtlasUI
