#pragma once

#include "NF/UI/AtlasUI/PanelBase.h"

namespace NF::UI::AtlasUI {

class SimplePanel final : public PanelBase {
public:
    SimplePanel(std::string panelId, std::string title)
        : PanelBase(std::move(panelId), std::move(title)) {}

    void paint(IPaintContext& context) override {
        if (!m_visible) {
            return;
        }

        context.fillRect(m_bounds, 0xFF202020);
        context.drawRect(m_bounds, 0xFF4A4A4A);
        NF::Rect titleRect = m_bounds;
        titleRect.x += 8.f;
        titleRect.y += 8.f;
        titleRect.h = 20.f;
        context.drawText(titleRect, m_title, 0, 0xFFFFFFFF);
    }
};

} // namespace NF::UI::AtlasUI
