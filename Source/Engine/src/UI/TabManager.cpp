#include "TabManager.h"
#include "UIConstants.h"
#include <algorithm>

namespace atlas::ui {

void TabManager::Init(UIScreen* screen) {
    m_screen = screen;
    m_tabContent.clear();
}

bool TabManager::ActivateTab(uint32_t tabId) {
    if (!m_screen) return false;

    UIWidget* tab = m_screen->GetWidgetMutable(tabId);
    if (!tab || tab->type != UIWidgetType::Tab) return false;

    uint32_t groupId = tab->parentId;
    uint32_t previousActive = GetActiveTab(groupId);

    if (previousActive == tabId) return false; // Already active

    // Deactivate all tabs in the same group
    for (uint32_t i = 1; i < kMaxWidgetId; ++i) {
        UIWidget* w = m_screen->GetWidgetMutable(i);
        if (!w || w->type != UIWidgetType::Tab) continue;
        if (w->parentId != groupId) continue;
        w->isChecked = false;
    }

    // Activate the clicked tab
    tab->isChecked = true;

    // Update content panel visibility
    UpdateContentVisibility(groupId);

    // Fire callback
    if (m_callback) {
        m_callback(groupId, tabId, previousActive);
    }

    return true;
}

uint32_t TabManager::GetActiveTab(uint32_t groupId) const {
    if (!m_screen) return 0;

    for (uint32_t i = 1; i < kMaxWidgetId; ++i) {
        const UIWidget* w = m_screen->GetWidget(i);
        if (!w || w->type != UIWidgetType::Tab) continue;
        if (w->parentId != groupId) continue;
        if (w->isChecked) return i;
    }
    return 0;
}

void TabManager::SetTabContent(uint32_t tabId, uint32_t contentPanelId) {
    // Replace existing mapping or add new one
    for (auto& pair : m_tabContent) {
        if (pair.first == tabId) {
            pair.second = contentPanelId;
            return;
        }
    }
    m_tabContent.push_back({tabId, contentPanelId});
}

uint32_t TabManager::GetTabContent(uint32_t tabId) const {
    for (const auto& pair : m_tabContent) {
        if (pair.first == tabId) return pair.second;
    }
    return 0;
}

void TabManager::SetTabChangedCallback(TabChangedCallback callback) {
    m_callback = std::move(callback);
}

bool TabManager::HandleClick(int32_t mouseX, int32_t mouseY) {
    if (!m_screen) return false;

    for (uint32_t i = 1; i < kMaxWidgetId; ++i) {
        const UIWidget* w = m_screen->GetWidget(i);
        if (!w || w->type != UIWidgetType::Tab) continue;
        if (!w->visible) continue;

        if (mouseX >= w->x && mouseX < w->x + w->width &&
            mouseY >= w->y && mouseY < w->y + w->height) {
            return ActivateTab(i);
        }
    }
    return false;
}

void TabManager::UpdateContentVisibility(uint32_t groupId) {
    if (!m_screen) return;

    uint32_t activeTab = GetActiveTab(groupId);

    // For each tab in the group, show/hide its content panel
    for (uint32_t i = 1; i < kMaxWidgetId; ++i) {
        const UIWidget* w = m_screen->GetWidget(i);
        if (!w || w->type != UIWidgetType::Tab) continue;
        if (w->parentId != groupId) continue;

        uint32_t contentId = GetTabContent(i);
        if (contentId != 0) {
            m_screen->SetVisible(contentId, (i == activeTab));
        }
    }
}

} // namespace atlas::ui
