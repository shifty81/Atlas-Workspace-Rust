#pragma once

#include "NF/UI/AtlasUI/Widgets/Tooltip.h"
#include <string>

namespace NF::UI::AtlasUI {

class TooltipService {
public:
    static TooltipService& Get();

    void show(const std::string& text, NF::Vec2 anchor);
    void hide();
    [[nodiscard]] bool isShowing() const { return m_tooltip.isShowing(); }

    void paint(IPaintContext& context);

private:
    TooltipService() = default;
    Tooltip m_tooltip;
};

} // namespace NF::UI::AtlasUI
