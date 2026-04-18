#pragma once
// NF::UI::AtlasUI::RadioButton — single option in a radio group.
// Multiple RadioButtons sharing the same group ID act as a mutually exclusive set.

#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include <functional>
#include <string>
#include <utility>

namespace NF::UI::AtlasUI {

class RadioButton final : public WidgetBase {
public:
    using SelectHandler = std::function<void(int value)>;

    explicit RadioButton(std::string label = {}, int value = 0)
        : m_label(std::move(label)), m_value(value) {}

    void setLabel(std::string label) { m_label = std::move(label); }
    void setValue(int v) { m_value = v; }
    [[nodiscard]] int value() const { return m_value; }

    void setSelected(bool v) { m_selected = v; }
    [[nodiscard]] bool isSelected() const { return m_selected; }

    void setOnSelect(SelectHandler cb) { m_onSelect = std::move(cb); }

    void measure(ILayoutContext& context) override;
    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

    static constexpr float kDotSize = 16.f;

private:
    std::string m_label;
    int m_value = 0;
    bool m_selected = false;
    bool m_hovered = false;
    SelectHandler m_onSelect;
    NF::Vec2 m_desiredSize{};
};

} // namespace NF::UI::AtlasUI
