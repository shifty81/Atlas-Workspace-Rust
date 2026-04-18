#pragma once
// NF::UI::AtlasUI::StatusBar — single-row status bar widget.
// Displays a list of text sections separated by thin dividers.

#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

class StatusBar final : public WidgetBase {
public:
    void setText(const std::string& text);
    void addSection(const std::string& text);
    void clearSections();

    [[nodiscard]] const std::vector<std::string>& sections() const { return m_sections; }
    [[nodiscard]] size_t sectionCount() const { return m_sections.size(); }

    void measure(ILayoutContext& context) override;
    void arrange(const NF::Rect& bounds) override;
    void paint(IPaintContext& context) override;

    static constexpr float kBarHeight = 24.f;
    static constexpr float kSectionPadX = 12.f;

private:
    std::vector<std::string> m_sections;
};

} // namespace NF::UI::AtlasUI
