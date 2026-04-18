#include "TreeNodeManager.h"
#include "UIConstants.h"

namespace atlas::ui {

void TreeNodeManager::Init(UIScreen* screen) {
    m_screen = screen;
}

bool TreeNodeManager::HandleEvent(const UIEvent& event) {
    if (!m_screen) return false;

    if (event.type != UIEvent::Type::MouseDown || event.mouseButton != 0) {
        return false;
    }

    for (uint32_t i = 1; i < kMaxWidgetId; ++i) {
        const UIWidget* w = m_screen->GetWidget(i);
        if (!w || !w->visible) continue;
        if (w->type != UIWidgetType::TreeNode) continue;
        if (event.x >= w->x && event.x < w->x + w->width &&
            event.y >= w->y && event.y < w->y + w->height) {
            bool newExpanded = !w->isExpanded;
            m_screen->SetExpanded(i, newExpanded);
            if (m_callback) {
                m_callback(i, newExpanded);
            }
            return true;
        }
    }

    return false;
}

void TreeNodeManager::SetTreeNodeToggledCallback(TreeNodeToggledCallback callback) {
    m_callback = std::move(callback);
}

} // namespace atlas::ui
