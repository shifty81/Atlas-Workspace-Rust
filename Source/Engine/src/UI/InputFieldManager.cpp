#include "InputFieldManager.h"

namespace atlas::ui {

static constexpr uint32_t kKeyBackspace = 0x08;
static constexpr uint32_t kKeyDelete    = 0x2E;
static constexpr uint32_t kKeyLeft      = 0x25;
static constexpr uint32_t kKeyRight     = 0x27;
static constexpr uint32_t kKeyHome      = 0x24;
static constexpr uint32_t kKeyEnd       = 0x23;
static constexpr uint32_t kKeyEnter     = 0x0D;

void InputFieldManager::Init(UIScreen* screen) {
    m_screen = screen;
    m_fields.clear();
}

void InputFieldManager::RegisterField(uint32_t widgetId, const std::string& placeholder) {
    InputFieldState state;
    state.placeholder = placeholder;
    // Initialize text from the widget's name if it looks like content
    if (m_screen) {
        const UIWidget* w = m_screen->GetWidget(widgetId);
        if (w && !w->name.empty() && w->name != placeholder) {
            state.text = w->name;
            state.cursorPos = state.text.size();
        }
    }
    m_fields[widgetId] = std::move(state);
}

bool InputFieldManager::HandleEvent(const UIEvent& event, uint32_t focusedWidgetId) {
    if (focusedWidgetId == 0) return false;

    auto it = m_fields.find(focusedWidgetId);
    if (it == m_fields.end()) return false;

    auto& state = it->second;

    // Handle text input (character typed)
    if (event.type == UIEvent::Type::TextInput && event.textChar != 0) {
        state.text.insert(state.cursorPos, 1, event.textChar);
        state.cursorPos++;
        if (m_screen) {
            if (UIWidget* w = m_screen->GetWidgetMutable(focusedWidgetId)) {
                w->name = state.text;
            }
        }
        if (m_textChangedCallback) {
            m_textChangedCallback(focusedWidgetId, state.text);
        }
        return true;
    }

    // Handle key events
    if (event.type == UIEvent::Type::KeyDown) {
        switch (event.keyCode) {
            case kKeyBackspace:
                if (state.cursorPos > 0) {
                    state.text.erase(state.cursorPos - 1, 1);
                    state.cursorPos--;
                    if (m_screen) {
                        if (UIWidget* w = m_screen->GetWidgetMutable(focusedWidgetId)) {
                            w->name = state.text;
                        }
                    }
                    if (m_textChangedCallback) {
                        m_textChangedCallback(focusedWidgetId, state.text);
                    }
                }
                return true;

            case kKeyDelete:
                if (state.cursorPos < state.text.size()) {
                    state.text.erase(state.cursorPos, 1);
                    if (m_screen) {
                        if (UIWidget* w = m_screen->GetWidgetMutable(focusedWidgetId)) {
                            w->name = state.text;
                        }
                    }
                    if (m_textChangedCallback) {
                        m_textChangedCallback(focusedWidgetId, state.text);
                    }
                }
                return true;

            case kKeyLeft:
                if (state.cursorPos > 0) {
                    state.cursorPos--;
                }
                return true;

            case kKeyRight:
                if (state.cursorPos < state.text.size()) {
                    state.cursorPos++;
                }
                return true;

            case kKeyHome:
                state.cursorPos = 0;
                return true;

            case kKeyEnd:
                state.cursorPos = state.text.size();
                return true;

            case kKeyEnter:
                if (m_textSubmitCallback) {
                    m_textSubmitCallback(focusedWidgetId, state.text);
                }
                // Clear the field after submission so the user can type again
                state.text.clear();
                state.cursorPos = 0;
                if (m_screen) {
                    if (UIWidget* w = m_screen->GetWidgetMutable(focusedWidgetId)) {
                        w->name.clear();
                    }
                }
                return true;

            default:
                break;
        }
    }

    return false;
}

std::string InputFieldManager::GetText(uint32_t widgetId) const {
    auto it = m_fields.find(widgetId);
    return (it != m_fields.end()) ? it->second.text : "";
}

void InputFieldManager::SetText(uint32_t widgetId, const std::string& text) {
    auto it = m_fields.find(widgetId);
    if (it == m_fields.end()) return;

    it->second.text = text;
    it->second.cursorPos = text.size();

    if (m_screen) {
        if (UIWidget* w = m_screen->GetWidgetMutable(widgetId)) {
            w->name = text;
        }
    }
}

size_t InputFieldManager::GetCursorPos(uint32_t widgetId) const {
    auto it = m_fields.find(widgetId);
    return (it != m_fields.end()) ? it->second.cursorPos : 0;
}

void InputFieldManager::SetCursorPos(uint32_t widgetId, size_t pos) {
    auto it = m_fields.find(widgetId);
    if (it == m_fields.end()) return;

    if (pos > it->second.text.size()) {
        pos = it->second.text.size();
    }
    it->second.cursorPos = pos;
}

std::string InputFieldManager::GetPlaceholder(uint32_t widgetId) const {
    auto it = m_fields.find(widgetId);
    return (it != m_fields.end()) ? it->second.placeholder : "";
}

bool InputFieldManager::IsRegistered(uint32_t widgetId) const {
    return m_fields.find(widgetId) != m_fields.end();
}

void InputFieldManager::SetTextChangedCallback(TextChangedCallback callback) {
    m_textChangedCallback = std::move(callback);
}

void InputFieldManager::SetTextSubmitCallback(TextSubmitCallback callback) {
    m_textSubmitCallback = std::move(callback);
}

void InputFieldManager::Clear() {
    m_fields.clear();
}

} // namespace atlas::ui
