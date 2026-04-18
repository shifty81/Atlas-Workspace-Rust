#include "FocusManager.h"
#include "UIConstants.h"
#include <algorithm>

namespace atlas::ui {

static constexpr uint32_t kKeyTab = 0x09;

void FocusManager::Init(UIScreen* screen) {
    m_screen = screen;
    m_focusedWidgetId = 0;
    m_focusable.clear();
}

bool FocusManager::HandleClick(int32_t x, int32_t y) {
    if (!m_screen) return false;

    uint32_t hitId = 0;
    for (uint32_t i = 1; i < kMaxWidgetId; ++i) {
        const UIWidget* w = m_screen->GetWidget(i);
        if (!w || !w->visible) continue;
        if (!IsFocusable(i)) continue;
        if (x >= w->x && x < w->x + w->width &&
            y >= w->y && y < w->y + w->height) {
            hitId = i;
            // Don't break — last hit (highest ID / last painted) wins
        }
    }

    if (hitId != m_focusedWidgetId) {
        uint32_t prev = m_focusedWidgetId;
        m_focusedWidgetId = hitId;
        if (m_callback) {
            m_callback(m_focusedWidgetId, prev);
        }
        return true;
    }
    return false;
}

bool FocusManager::HandleKeyEvent(const UIEvent& event) {
    if (!m_screen) return false;
    if (event.type != UIEvent::Type::KeyDown) return false;
    if (event.keyCode != kKeyTab) return false;

    auto widgets = GetFocusableWidgets();
    if (widgets.empty()) return false;

    if (m_focusedWidgetId == 0) {
        SetFocus(widgets.front());
        return true;
    }

    auto it = std::find(widgets.begin(), widgets.end(), m_focusedWidgetId);
    if (it == widgets.end()) {
        SetFocus(widgets.front());
    } else {
        ++it;
        SetFocus(it == widgets.end() ? widgets.front() : *it);
    }
    return true;
}

uint32_t FocusManager::GetFocusedWidgetId() const {
    return m_focusedWidgetId;
}

void FocusManager::SetFocus(uint32_t widgetId) {
    if (widgetId == m_focusedWidgetId) return;
    uint32_t prev = m_focusedWidgetId;
    m_focusedWidgetId = widgetId;
    if (m_callback) {
        m_callback(m_focusedWidgetId, prev);
    }
}

void FocusManager::ClearFocus() {
    SetFocus(0);
}

void FocusManager::SetFocusable(uint32_t widgetId, bool focusable) {
    m_focusable[widgetId] = focusable;
}

bool FocusManager::IsFocusable(uint32_t widgetId) const {
    auto it = m_focusable.find(widgetId);
    if (it != m_focusable.end()) return it->second;

    // By default, InputField widgets are focusable
    if (!m_screen) return false;
    const UIWidget* w = m_screen->GetWidget(widgetId);
    if (!w) return false;
    return w->type == UIWidgetType::InputField;
}

void FocusManager::SetFocusChangedCallback(FocusChangedCallback callback) {
    m_callback = std::move(callback);
}

std::vector<uint32_t> FocusManager::GetFocusableWidgets() const {
    std::vector<uint32_t> result;
    if (!m_screen) return result;
    for (uint32_t i = 1; i < kMaxWidgetId; ++i) {
        const UIWidget* w = m_screen->GetWidget(i);
        if (!w || !w->visible) continue;
        if (IsFocusable(i)) {
            result.push_back(i);
        }
    }
    return result;
}

} // namespace atlas::ui
