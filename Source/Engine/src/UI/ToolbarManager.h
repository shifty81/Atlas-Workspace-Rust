#pragma once
#include "UIScreenGraph.h"
#include <cstdint>
#include <functional>

namespace atlas::ui {

/// Callback invoked when a toolbar button is clicked.
/// Parameters: toolbarId, buttonId
using ToolbarButtonCallback = std::function<void(uint32_t, uint32_t)>;

/// Manages interactive toolbar buttons.
///
/// Toolbar widgets contain Button children.  The ToolbarManager handles
/// click events on those buttons and invokes a callback.  It also
/// supports toggle-state buttons (using isChecked on Button widgets).
class ToolbarManager {
public:
    /// Initialize with a reference to the UI screen.
    void Init(UIScreen* screen);

    /// Handle a mouse-down event.  Checks if a Button child of any
    /// Toolbar widget was clicked.  Returns true if consumed.
    bool HandleClick(int32_t mouseX, int32_t mouseY);

    /// Set a callback invoked when a toolbar button is clicked.
    void SetButtonCallback(ToolbarButtonCallback callback);

    /// Set whether a toolbar button is a toggle (uses isChecked).
    void SetToggleButton(uint32_t buttonId, bool isToggle);

    /// Check if a toolbar button is a toggle button.
    bool IsToggleButton(uint32_t buttonId) const;

private:
    UIScreen* m_screen = nullptr;
    ToolbarButtonCallback m_callback;

    // Button IDs that behave as toggles
    std::vector<uint32_t> m_toggleButtons;
};

} // namespace atlas::ui
