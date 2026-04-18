#include "UIEventRouter.h"
#include <algorithm>

namespace atlas::ui {

void UIEventRouter::Register(UIEventTarget* target) {
    if (!target) return;
    m_targets.push_back(target);
    m_needsSort = true;
}

void UIEventRouter::Unregister(UIEventTarget* target) {
    m_targets.erase(
        std::remove(m_targets.begin(), m_targets.end(), target),
        m_targets.end());

    // Clear focus if the removed target held any focus role.
    if (m_focus.hovered == target)  m_focus.hovered = nullptr;
    if (m_focus.active == target)   m_focus.active = nullptr;
    if (m_focus.keyboard == target) m_focus.keyboard = nullptr;
}

bool UIEventRouter::Dispatch(const UIEvent& event) {
    if (m_needsSort) {
        std::sort(m_targets.begin(), m_targets.end(),
                  [](const UIEventTarget* a, const UIEventTarget* b) {
                      return a->zOrder > b->zOrder; // descending
                  });
        m_needsSort = false;
    }

    // Update hover tracking for mouse-move events.
    if (event.type == UIEvent::Type::MouseMove) {
        m_focus.hovered = nullptr;
        for (auto* t : m_targets) {
            if (t->HitTest(event.x, event.y)) {
                m_focus.hovered = t;
                break;
            }
        }
    }

    // Keyboard and text events go to the keyboard-focus target.
    if (event.type == UIEvent::Type::KeyDown ||
        event.type == UIEvent::Type::KeyUp ||
        event.type == UIEvent::Type::TextInput) {
        if (m_focus.keyboard) {
            return m_focus.keyboard->OnEvent(event);
        }
        return false;
    }

    // Active target captures all mouse events until release.
    if (m_focus.active) {
        bool consumed = m_focus.active->OnEvent(event);
        if (event.type == UIEvent::Type::MouseUp) {
            m_focus.active = nullptr;
        }
        return consumed;
    }

    for (auto* t : m_targets) {
        if (t->HitTest(event.x, event.y)) {
            if (t->OnEvent(event)) {
                // Mouse-down sets active capture and keyboard focus.
                if (event.type == UIEvent::Type::MouseDown) {
                    m_focus.active = t;
                    m_focus.keyboard = t;
                }
                return true; // consumed
            }
        }
    }

    return false;
}

const UIFocusState& UIEventRouter::GetFocus() const {
    return m_focus;
}

void UIEventRouter::Clear() {
    m_targets.clear();
    m_focus = {};
    m_needsSort = false;
}

size_t UIEventRouter::TargetCount() const {
    return m_targets.size();
}

} // namespace atlas::ui
