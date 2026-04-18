#pragma once
// NF::UI::AtlasUI::ProgressBar — horizontal fill progress widget (0–1).

#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include <algorithm>
#include <string>
#include <utility>

namespace NF::UI::AtlasUI {

class ProgressBar final : public WidgetBase {
public:
    explicit ProgressBar(float value = 0.f) : m_value(std::clamp(value, 0.f, 1.f)) {}

    /// Set fill fraction in [0, 1].
    void setValue(float v) { m_value = std::clamp(v, 0.f, 1.f); }
    [[nodiscard]] float value() const { return m_value; }

    void setLabel(std::string label) { m_label = std::move(label); }
    [[nodiscard]] const std::string& label() const { return m_label; }

    void measure(ILayoutContext& context) override;
    void paint(IPaintContext& context) override;

    static constexpr float kBarHeight = 8.f;
    static constexpr float kLabelHeight = 16.f;

private:
    float m_value = 0.f;
    std::string m_label;
    NF::Vec2 m_desiredSize{};
};

} // namespace NF::UI::AtlasUI
