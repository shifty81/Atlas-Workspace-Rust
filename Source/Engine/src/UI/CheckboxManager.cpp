#include "CheckboxManager.h"
#include "UIConstants.h"

namespace atlas::ui {

void CheckboxManager::Init(UIScreen* screen) {
    m_screen = screen;
}

bool CheckboxManager::HandleEvent(const UIEvent& event) {
    if (!m_screen) return false;

    if (event.type != UIEvent::Type::MouseDown || event.mouseButton != 0) {
        return false;
    }

    for (uint32_t i = 1; i < kMaxWidgetId; ++i) {
        const UIWidget* w = m_screen->GetWidget(i);
        if (!w || !w->visible) continue;
        if (w->type != UIWidgetType::Checkbox) continue;
        if (event.x >= w->x && event.x < w->x + w->width &&
            event.y >= w->y && event.y < w->y + w->height) {
            bool newChecked = !w->isChecked;
            m_screen->SetChecked(i, newChecked);
            if (m_callback) {
                m_callback(i, newChecked);
            }
            return true;
        }
    }

    return false;
}

void CheckboxManager::SetCheckboxChangedCallback(CheckboxChangedCallback callback) {
    m_callback = std::move(callback);
}

} // namespace atlas::ui
