#include "ToolbarManager.h"
#include "UIConstants.h"
#include <algorithm>

namespace atlas::ui {

void ToolbarManager::Init(UIScreen* screen) {
    m_screen = screen;
    m_toggleButtons.clear();
}

bool ToolbarManager::HandleClick(int32_t mouseX, int32_t mouseY) {
    if (!m_screen) return false;

    // Find Button children of Toolbar widgets
    for (uint32_t i = 1; i < kMaxWidgetId; ++i) {
        const UIWidget* widget = m_screen->GetWidget(i);
        if (!widget || widget->type != UIWidgetType::Button) continue;
        if (!widget->visible) continue;

        // Check if the parent is a Toolbar
        const UIWidget* parent = m_screen->GetWidget(widget->parentId);
        if (!parent || parent->type != UIWidgetType::Toolbar) continue;

        // Hit test
        if (mouseX >= widget->x && mouseX < widget->x + widget->width &&
            mouseY >= widget->y && mouseY < widget->y + widget->height) {

            // Toggle if this is a toggle button
            if (IsToggleButton(i)) {
                UIWidget* mutableWidget = m_screen->GetWidgetMutable(i);
                if (mutableWidget) {
                    mutableWidget->isChecked = !mutableWidget->isChecked;
                }
            }

            // Invoke callback
            if (m_callback) {
                m_callback(widget->parentId, i);
            }

            return true;
        }
    }
    return false;
}

void ToolbarManager::SetButtonCallback(ToolbarButtonCallback callback) {
    m_callback = std::move(callback);
}

void ToolbarManager::SetToggleButton(uint32_t buttonId, bool isToggle) {
    auto it = std::find(m_toggleButtons.begin(), m_toggleButtons.end(), buttonId);
    if (isToggle) {
        if (it == m_toggleButtons.end()) {
            m_toggleButtons.push_back(buttonId);
        }
    } else {
        if (it != m_toggleButtons.end()) {
            m_toggleButtons.erase(it);
        }
    }
}

bool ToolbarManager::IsToggleButton(uint32_t buttonId) const {
    return std::find(m_toggleButtons.begin(), m_toggleButtons.end(), buttonId)
           != m_toggleButtons.end();
}

} // namespace atlas::ui
