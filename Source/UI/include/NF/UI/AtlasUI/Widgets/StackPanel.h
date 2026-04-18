#pragma once

#include "NF/UI/AtlasUI/WidgetTheme.h"
#include "NF/UI/AtlasUI/Widgets/Container.h"

namespace NF::UI::AtlasUI {

enum class StackOrientation {
    Horizontal,
    Vertical
};

class StackPanel final : public Container {
public:
    explicit StackPanel(StackOrientation orientation, float spacing = Theme::Spacing::Small)
        : m_orientation(orientation), m_spacing(spacing) {}

    void measure(ILayoutContext& context) override;
    void arrange(const NF::Rect& bounds) override;
    void paint(IPaintContext& context) override;

    [[nodiscard]] NF::Vec2 desiredSize() const { return m_desiredSize; }

private:
    StackOrientation m_orientation = StackOrientation::Vertical;
    float m_spacing = Theme::Spacing::Small;
    NF::Vec2 m_desiredSize{};
};

} // namespace NF::UI::AtlasUI
