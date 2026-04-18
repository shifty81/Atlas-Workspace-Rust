#pragma once

#include "NF/UI/AtlasUI/Interfaces.h"

namespace NF::UI::AtlasUI {

class WidgetBase : public IWidget {
public:
    void setVisible(bool visible) override { m_visible = visible; }
    [[nodiscard]] bool isVisible() const override { return m_visible; }
    [[nodiscard]] NF::Rect bounds() const override { return m_bounds; }

    void measure(ILayoutContext&) override {}
    void arrange(const NF::Rect& bounds) override { m_bounds = bounds; }
    void paint(IPaintContext&) override {}
    bool handleInput(IInputContext&) override { return false; }

protected:
    NF::Rect m_bounds{};
    bool m_visible = true;
};

} // namespace NF::UI::AtlasUI
