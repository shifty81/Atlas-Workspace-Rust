#pragma once

#include "NF/UI/AtlasUI/PanelBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"

namespace NF::UI::AtlasUI {

class Panel final : public PanelBase {
public:
    Panel(std::string panelId, std::string title)
        : PanelBase(std::move(panelId), std::move(title)) {}

    void paint(IPaintContext& context) override;
};

} // namespace NF::UI::AtlasUI
