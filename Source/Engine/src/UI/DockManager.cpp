#include "DockManager.h"
#include "UIConstants.h"

namespace atlas::ui {

void DockManager::Init(UIScreen* screen) {
    m_screen = screen;
    m_dockAreas.clear();
    m_panelToDock.clear();
}

void DockManager::RegisterDockArea(uint32_t dockAreaId) {
    if (m_dockAreas.find(dockAreaId) == m_dockAreas.end()) {
        m_dockAreas[dockAreaId] = DockAreaState{};
    }
}

bool DockManager::DockPanel(uint32_t dockAreaId, uint32_t panelId, DockSide side, float ratio) {
    if (!m_screen) return false;
    if (ratio <= 0.0f || ratio > 1.0f) return false;

    auto it = m_dockAreas.find(dockAreaId);
    if (it == m_dockAreas.end()) return false;

    const UIWidget* dockWidget = m_screen->GetWidget(dockAreaId);
    if (!dockWidget) return false;

    // Remove from previous dock area if any
    UndockPanel(panelId);

    // Set the panel as child of the dock area
    m_screen->SetParent(panelId, dockAreaId);
    it->second.panels.push_back(panelId);
    m_panelToDock[panelId] = dockAreaId;

    // Position the panel within the dock area based on side
    UIWidget* panel = m_screen->GetWidgetMutable(panelId);
    if (!panel) return false;

    float dockX = dockWidget->x;
    float dockY = dockWidget->y;
    float dockW = dockWidget->width;
    float dockH = dockWidget->height;

    switch (side) {
        case DockSide::Left:
            panel->x = dockX;
            panel->y = dockY;
            panel->width = dockW * ratio;
            panel->height = dockH;
            break;
        case DockSide::Right:
            panel->x = dockX + dockW * (1.0f - ratio);
            panel->y = dockY;
            panel->width = dockW * ratio;
            panel->height = dockH;
            break;
        case DockSide::Top:
            panel->x = dockX;
            panel->y = dockY;
            panel->width = dockW;
            panel->height = dockH * ratio;
            break;
        case DockSide::Bottom:
            panel->x = dockX;
            panel->y = dockY + dockH * (1.0f - ratio);
            panel->width = dockW;
            panel->height = dockH * ratio;
            break;
        case DockSide::Center:
            panel->x = dockX;
            panel->y = dockY;
            panel->width = dockW;
            panel->height = dockH;
            break;
    }

    panel->visible = true;

    if (m_callback) {
        m_callback(dockAreaId, panelId, side);
    }

    return true;
}

bool DockManager::UndockPanel(uint32_t panelId) {
    auto it = m_panelToDock.find(panelId);
    if (it == m_panelToDock.end()) return false;

    uint32_t dockAreaId = it->second;
    auto dockIt = m_dockAreas.find(dockAreaId);
    if (dockIt != m_dockAreas.end()) {
        auto& panels = dockIt->second.panels;
        panels.erase(std::remove(panels.begin(), panels.end(), panelId), panels.end());
    }
    m_panelToDock.erase(it);

    // Clear parent
    if (m_screen) {
        m_screen->SetParent(panelId, 0);
    }

    return true;
}

uint32_t DockManager::GetPanelDockArea(uint32_t panelId) const {
    auto it = m_panelToDock.find(panelId);
    return (it != m_panelToDock.end()) ? it->second : 0;
}

std::vector<uint32_t> DockManager::GetDockedPanels(uint32_t dockAreaId) const {
    auto it = m_dockAreas.find(dockAreaId);
    if (it != m_dockAreas.end()) {
        return it->second.panels;
    }
    return {};
}

bool DockManager::IsDockArea(uint32_t widgetId) const {
    return m_dockAreas.find(widgetId) != m_dockAreas.end();
}

void DockManager::SetDockCallback(DockCallback callback) {
    m_callback = std::move(callback);
}

void DockManager::SetSplitRatio(uint32_t dockAreaId, float ratio) {
    if (ratio <= 0.0f || ratio > 1.0f) return;
    auto it = m_dockAreas.find(dockAreaId);
    if (it != m_dockAreas.end()) {
        it->second.splitRatio = ratio;
    }
}

float DockManager::GetSplitRatio(uint32_t dockAreaId) const {
    auto it = m_dockAreas.find(dockAreaId);
    return (it != m_dockAreas.end()) ? it->second.splitRatio : 0.5f;
}

} // namespace atlas::ui
