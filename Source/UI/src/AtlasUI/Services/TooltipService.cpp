#include "NF/UI/AtlasUI/Services/TooltipService.h"

namespace NF::UI::AtlasUI {

TooltipService& TooltipService::Get() {
    static TooltipService instance;
    return instance;
}

void TooltipService::show(const std::string& text, NF::Vec2 anchor) {
    m_tooltip.show(text, anchor);
}

void TooltipService::hide() {
    m_tooltip.hide();
}

void TooltipService::paint(IPaintContext& context) {
    if (m_tooltip.isShowing()) {
        m_tooltip.paint(context);
    }
}

} // namespace NF::UI::AtlasUI
