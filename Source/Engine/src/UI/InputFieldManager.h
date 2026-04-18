#pragma once
#include "UIScreenGraph.h"
#include "UIEventRouter.h"
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

namespace atlas::ui {

/// Callback invoked when text in an input field changes.
/// Parameters: widgetId, newText
using TextChangedCallback = std::function<void(uint32_t, const std::string&)>;

/// Callback invoked when Enter is pressed in an input field.
/// Parameters: widgetId, text
using TextSubmitCallback = std::function<void(uint32_t, const std::string&)>;

/// Tracks text editing state for a single InputField widget.
struct InputFieldState {
    std::string text;
    size_t cursorPos = 0;
    std::string placeholder;
};

/// Manages InputField widgets — handles text cursor, insertion, deletion,
/// and basic keyboard-driven text editing.
class InputFieldManager {
public:
    void Init(UIScreen* screen);

    /// Register an input field for text editing.
    void RegisterField(uint32_t widgetId, const std::string& placeholder = "");

    /// Handle a keyboard/text event for the currently focused input field.
    /// Returns true if the event was consumed.
    bool HandleEvent(const UIEvent& event, uint32_t focusedWidgetId);

    /// Get the current text of an input field.
    std::string GetText(uint32_t widgetId) const;

    /// Set the text of an input field programmatically.
    void SetText(uint32_t widgetId, const std::string& text);

    /// Get the cursor position in an input field.
    size_t GetCursorPos(uint32_t widgetId) const;

    /// Set cursor position.
    void SetCursorPos(uint32_t widgetId, size_t pos);

    /// Get the placeholder text.
    std::string GetPlaceholder(uint32_t widgetId) const;

    /// Returns true if the widget is a registered input field.
    bool IsRegistered(uint32_t widgetId) const;

    /// Set callback for text changes.
    void SetTextChangedCallback(TextChangedCallback callback);

    /// Set callback for text submission (Enter key).
    void SetTextSubmitCallback(TextSubmitCallback callback);

    /// Clear all registered fields.
    void Clear();

private:
    UIScreen* m_screen = nullptr;
    std::unordered_map<uint32_t, InputFieldState> m_fields;
    TextChangedCallback m_textChangedCallback;
    TextSubmitCallback m_textSubmitCallback;
};

} // namespace atlas::ui
