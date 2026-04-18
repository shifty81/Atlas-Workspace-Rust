#pragma once
// NF::UI::AtlasUI::Checkbox — two-state checkbox widget.

#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include <functional>
#include <string>
#include <utility>

namespace NF::UI::AtlasUI {

class Checkbox final : public WidgetBase {
public:
    using ChangeHandler = std::function<void(bool checked)>;

    explicit Checkbox(std::string label = {}, bool checked = false)
        : m_label(std::move(label)), m_checked(checked) {}

    void setLabel(std::string label) { m_label = std::move(label); }
    void setChecked(bool v) { m_checked = v; }
    [[nodiscard]] bool isChecked() const { return m_checked; }
    void setOnChange(ChangeHandler cb) { m_onChange = std::move(cb); }

    void measure(ILayoutContext& context) override;
    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

    static constexpr float kBoxSize = 16.f;

private:
    std::string m_label;
    bool m_checked = false;
    bool m_hovered = false;
    ChangeHandler m_onChange;
    NF::Vec2 m_desiredSize{};
};

} // namespace NF::UI::AtlasUI
