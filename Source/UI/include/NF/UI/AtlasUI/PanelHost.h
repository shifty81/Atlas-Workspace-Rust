#pragma once

#include "NF/UI/AtlasUI/Interfaces.h"
#include <algorithm>
#include <memory>
#include <vector>

namespace NF::UI::AtlasUI {

class PanelHost final : public IWindowHost {
public:
    void attachPanel(std::shared_ptr<IPanel> panel) override;
    void detachPanel(std::string_view panelId) override;
    [[nodiscard]] std::shared_ptr<IPanel> findPanel(std::string_view panelId) const override;
    [[nodiscard]] const std::vector<std::shared_ptr<IPanel>>& panels() const { return m_panels; }

private:
    std::vector<std::shared_ptr<IPanel>> m_panels;
};

} // namespace NF::UI::AtlasUI
