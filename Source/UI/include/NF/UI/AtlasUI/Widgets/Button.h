#pragma once

#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include <functional>
#include <string>
#include <utility>

namespace NF::UI::AtlasUI {

class Button final : public WidgetBase {
public:
    using ClickHandler = std::function<void()>;

    explicit Button(std::string text, ClickHandler onClick = {})
        : m_text(std::move(text)), m_onClick(std::move(onClick)) {}

    void setOnClick(ClickHandler onClick) { m_onClick = std::move(onClick); }
    void setText(std::string text) { m_text = std::move(text); }

    void measure(ILayoutContext& context) override;
    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

    [[nodiscard]] NF::Vec2 desiredSize() const { return m_desiredSize; }

private:
    std::string m_text;
    ClickHandler m_onClick;
    NF::Vec2 m_desiredSize{};
    bool m_hovered = false;
    bool m_pressed = false;
};

} // namespace NF::UI::AtlasUI
