#pragma once
// NF::UI::AtlasUI::Slider — horizontal range slider widget.

#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include <algorithm>
#include <functional>
#include <string>
#include <utility>

namespace NF::UI::AtlasUI {

class Slider final : public WidgetBase {
public:
    using ChangeHandler = std::function<void(float value)>;

    Slider(float minVal = 0.f, float maxVal = 1.f, float value = 0.f)
        : m_min(minVal), m_max(maxVal),
          m_value(std::clamp(value, minVal, maxVal)) {}

    void setValue(float v) { m_value = std::clamp(v, m_min, m_max); }
    [[nodiscard]] float value() const { return m_value; }

    void setRange(float minVal, float maxVal) {
        m_min = minVal;
        m_max = maxVal;
        m_value = std::clamp(m_value, m_min, m_max);
    }
    [[nodiscard]] float minValue() const { return m_min; }
    [[nodiscard]] float maxValue() const { return m_max; }

    void setLabel(std::string label) { m_label = std::move(label); }
    void setOnChange(ChangeHandler cb) { m_onChange = std::move(cb); }

    /// Fraction of the slider travel (0=min, 1=max).
    [[nodiscard]] float fraction() const {
        float range = m_max - m_min;
        return range > 0.f ? (m_value - m_min) / range : 0.f;
    }

    void measure(ILayoutContext& context) override;
    void arrange(const NF::Rect& bounds) override;
    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

    static constexpr float kThumbSize = 14.f;
    static constexpr float kTrackHeight = 4.f;
    static constexpr float kLabelHeight = 16.f;

private:
    [[nodiscard]] NF::Rect trackRect() const;
    [[nodiscard]] NF::Rect thumbRect() const;
    void setValueFromMouseX(float mouseX);

    std::string m_label;
    float m_min = 0.f;
    float m_max = 1.f;
    float m_value = 0.f;
    bool m_dragging = false;
    ChangeHandler m_onChange;
    NF::Vec2 m_desiredSize{};
};

} // namespace NF::UI::AtlasUI
