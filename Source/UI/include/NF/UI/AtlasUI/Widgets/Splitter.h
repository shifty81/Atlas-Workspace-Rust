#pragma once

#include "NF/UI/AtlasUI/Widgets/Container.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"

namespace NF::UI::AtlasUI {

enum class SplitOrientation {
    Horizontal,
    Vertical
};

class Splitter final : public Container {
public:
    explicit Splitter(SplitOrientation orientation, float ratio = 0.5f)
        : m_orientation(orientation), m_ratio(std::clamp(ratio, 0.1f, 0.9f)) {}

    void setFirst(std::shared_ptr<IWidget> widget);
    void setSecond(std::shared_ptr<IWidget> widget);

    void measure(ILayoutContext& context) override;
    void arrange(const NF::Rect& bounds) override;
    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

    [[nodiscard]] float ratio() const { return m_ratio; }

private:
    [[nodiscard]] NF::Rect separatorRect() const;
    void updateRatioFromMouse(NF::Vec2 mousePosition);

    SplitOrientation m_orientation = SplitOrientation::Horizontal;
    float m_ratio = 0.5f;
    float m_separatorThickness = 6.f;
    bool m_dragging = false;
};

} // namespace NF::UI::AtlasUI
