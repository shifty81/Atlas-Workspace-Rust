#pragma once

#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include <string>

namespace NF::UI::AtlasUI {

class Tooltip final : public WidgetBase {
public:
    void show(const std::string& text, NF::Vec2 anchor);
    void hide();
    [[nodiscard]] bool isShowing() const { return m_visible; }

    void paint(IPaintContext& context) override;

private:
    std::string m_text;
    NF::Vec2 m_anchor{};
};

} // namespace NF::UI::AtlasUI
