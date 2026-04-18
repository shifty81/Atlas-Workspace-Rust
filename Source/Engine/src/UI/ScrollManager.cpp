#include "ScrollManager.h"

namespace atlas::ui {

static constexpr float kScrollLineHeight = 20.0f;

void ScrollManager::Init(UIScreen* screen) {
    m_screen = screen;
    m_scrollStates.clear();
}

void ScrollManager::RegisterScrollView(uint32_t scrollViewId, float contentHeight) {
    if (!m_screen) return;

    const UIWidget* widget = m_screen->GetWidget(scrollViewId);
    if (!widget || widget->type != UIWidgetType::ScrollView) return;

    UIScrollState state;
    state.SetViewportHeight(widget->height);
    state.SetContentHeight(contentHeight);
    m_scrollStates[scrollViewId] = state;
}

bool ScrollManager::HandleScrollWheel(int32_t mouseX, int32_t mouseY, float delta) {
    if (!m_screen) return false;

    // Find the ScrollView under the mouse cursor
    for (auto& [id, state] : m_scrollStates) {
        const UIWidget* widget = m_screen->GetWidget(id);
        if (!widget || !widget->visible) continue;

        if (IsPointInWidget(widget, mouseX, mouseY)) {
            if (state.IsScrollable()) {
                state.Scroll(delta * kScrollLineHeight);
                return true;
            }
        }
    }
    return false;
}

const UIScrollState* ScrollManager::GetScrollState(uint32_t scrollViewId) const {
    auto it = m_scrollStates.find(scrollViewId);
    if (it != m_scrollStates.end()) {
        return &it->second;
    }
    return nullptr;
}

UIScrollState* ScrollManager::GetScrollStateMutable(uint32_t scrollViewId) {
    auto it = m_scrollStates.find(scrollViewId);
    if (it != m_scrollStates.end()) {
        return &it->second;
    }
    return nullptr;
}

void ScrollManager::SetContentHeight(uint32_t scrollViewId, float contentHeight) {
    auto it = m_scrollStates.find(scrollViewId);
    if (it != m_scrollStates.end()) {
        it->second.SetContentHeight(contentHeight);
    }
}

void ScrollManager::ScrollToTop(uint32_t scrollViewId) {
    auto it = m_scrollStates.find(scrollViewId);
    if (it != m_scrollStates.end()) {
        it->second.ScrollToTop();
    }
}

void ScrollManager::ScrollToBottom(uint32_t scrollViewId) {
    auto it = m_scrollStates.find(scrollViewId);
    if (it != m_scrollStates.end()) {
        it->second.ScrollToBottom();
    }
}

bool ScrollManager::IsPointInWidget(const UIWidget* widget, int32_t x, int32_t y) const {
    if (!widget) return false;
    return x >= widget->x && x < widget->x + widget->width &&
           y >= widget->y && y < widget->y + widget->height;
}

} // namespace atlas::ui
