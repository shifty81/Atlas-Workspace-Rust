#pragma once
#include "UIScreenGraph.h"
#include "UIEventRouter.h"
#include <cstdint>
#include <functional>

namespace atlas::ui {

/// Callback invoked when keyboard focus changes.
/// Parameters: newFocusId (0 = no focus), previousFocusId
using FocusChangedCallback = std::function<void(uint32_t, uint32_t)>;

/// Centralized widget focus tracking.
/// Determines which widget receives keyboard input and handles focus
/// transitions on mouse click.
class FocusManager {
public:
    void Init(UIScreen* screen);

    /// Handle a mouse click — sets focus to the widget under the cursor.
    /// Returns true if focus changed.
    bool HandleClick(int32_t x, int32_t y);

    /// Handle a keyboard event (Tab key cycles focus among focusable widgets).
    /// Returns true if the event was consumed.
    bool HandleKeyEvent(const UIEvent& event);

    /// Returns the ID of the widget that currently has keyboard focus (0 = none).
    uint32_t GetFocusedWidgetId() const;

    /// Programmatically set focus to a specific widget.
    void SetFocus(uint32_t widgetId);

    /// Clear focus (no widget focused).
    void ClearFocus();

    /// Mark a widget as focusable or not.
    void SetFocusable(uint32_t widgetId, bool focusable);

    /// Returns true if the widget is focusable.
    bool IsFocusable(uint32_t widgetId) const;

    /// Set callback for focus changes.
    void SetFocusChangedCallback(FocusChangedCallback callback);

private:
    /// Get ordered list of focusable widget IDs.
    std::vector<uint32_t> GetFocusableWidgets() const;

    UIScreen* m_screen = nullptr;
    uint32_t m_focusedWidgetId = 0;
    std::unordered_map<uint32_t, bool> m_focusable; // widgetId -> focusable
    FocusChangedCallback m_callback;
};

} // namespace atlas::ui
