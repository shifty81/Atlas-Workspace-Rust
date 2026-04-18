#pragma once

#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include <memory>
#include <string>

namespace NF::UI::AtlasUI {

class PropertyRow final : public WidgetBase {
public:
    PropertyRow(std::string label, std::shared_ptr<IWidget> valueWidget);

    void setLabel(const std::string& label) { m_label = label; }
    [[nodiscard]] const std::string& label() const { return m_label; }

    void measure(ILayoutContext& context) override;
    void arrange(const NF::Rect& bounds) override;
    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

private:
    std::string m_label;
    std::shared_ptr<IWidget> m_valueWidget;
    float m_labelWidth = 120.f;
};

} // namespace NF::UI::AtlasUI
