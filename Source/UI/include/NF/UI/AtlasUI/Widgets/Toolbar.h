#pragma once

#include "NF/UI/AtlasUI/Widgets/Container.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace NF::UI::AtlasUI {

struct ToolbarItem {
    std::string label;
    std::string tooltip;
    std::function<void()> action;
    bool separator = false;
};

class Toolbar final : public WidgetBase {
public:
    void addItem(std::string label, std::function<void()> action, std::string tooltip = {});
    void addSeparator();

    void measure(ILayoutContext& context) override;
    void arrange(const NF::Rect& bounds) override;
    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

private:
    std::vector<ToolbarItem> m_items;
    std::vector<float> m_itemWidths;
    int m_hoveredIndex = -1;
    float m_itemHeight = 28.f;
};

} // namespace NF::UI::AtlasUI
