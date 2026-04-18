#pragma once

#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include <functional>
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

class Dropdown final : public WidgetBase {
public:
    using SelectHandler = std::function<void(size_t index, const std::string& value)>;

    explicit Dropdown(std::vector<std::string> items = {});

    void setItems(std::vector<std::string> items);
    void setSelectedIndex(size_t index);
    void setOnSelect(SelectHandler handler) { m_onSelect = std::move(handler); }

    [[nodiscard]] size_t selectedIndex() const { return m_selectedIndex; }
    [[nodiscard]] const std::string& selectedText() const;

    void measure(ILayoutContext& context) override;
    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

private:
    std::vector<std::string> m_items;
    size_t m_selectedIndex = 0;
    SelectHandler m_onSelect;
    bool m_open = false;
    int m_hoveredItem = -1;
    float m_itemHeight = 24.f;
};

} // namespace NF::UI::AtlasUI
