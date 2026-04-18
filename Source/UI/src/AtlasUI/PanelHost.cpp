#include "NF/UI/AtlasUI/PanelHost.h"

namespace NF::UI::AtlasUI {

void PanelHost::attachPanel(std::shared_ptr<IPanel> panel) {
    if (!panel) {
        return;
    }

    auto existing = findPanel(panel->panelId());
    if (existing) {
        return;
    }

    m_panels.push_back(std::move(panel));
}

void PanelHost::detachPanel(std::string_view panelId) {
    auto it = std::remove_if(m_panels.begin(), m_panels.end(),
        [panelId](const std::shared_ptr<IPanel>& panel) {
            return panel && panelId == panel->panelId();
        });
    m_panels.erase(it, m_panels.end());
}

std::shared_ptr<IPanel> PanelHost::findPanel(std::string_view panelId) const {
    auto it = std::find_if(m_panels.begin(), m_panels.end(),
        [panelId](const std::shared_ptr<IPanel>& panel) {
            return panel && panelId == panel->panelId();
        });
    return it != m_panels.end() ? *it : nullptr;
}

} // namespace NF::UI::AtlasUI
