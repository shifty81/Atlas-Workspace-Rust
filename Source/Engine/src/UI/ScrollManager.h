#pragma once
#include "UIScreenGraph.h"
#include "UIScrollState.h"
#include <cstdint>
#include <unordered_map>

namespace atlas::ui {

/// Manages scroll state for ScrollView widgets.
///
/// Each ScrollView widget gets its own UIScrollState.  The ScrollManager
/// handles scroll-wheel events by updating the state of the targeted
/// ScrollView and provides query methods for the renderer.
class ScrollManager {
public:
    /// Initialize with a reference to the UI screen.
    void Init(UIScreen* screen);

    /// Register a ScrollView widget for scroll tracking.
    /// Must be called after adding the widget to the screen.
    void RegisterScrollView(uint32_t scrollViewId, float contentHeight);

    /// Handle a scroll-wheel event at (mouseX, mouseY) with the given
    /// delta (positive = scroll down).  Returns true if a ScrollView
    /// consumed the event.
    bool HandleScrollWheel(int32_t mouseX, int32_t mouseY, float delta);

    /// Returns the scroll state for a given ScrollView widget.
    /// Returns nullptr if the widget is not registered.
    const UIScrollState* GetScrollState(uint32_t scrollViewId) const;

    /// Mutable access to the scroll state (e.g. to set content height).
    UIScrollState* GetScrollStateMutable(uint32_t scrollViewId);

    /// Update content height for a ScrollView.
    void SetContentHeight(uint32_t scrollViewId, float contentHeight);

    /// Scroll a specific ScrollView to the top.
    void ScrollToTop(uint32_t scrollViewId);

    /// Scroll a specific ScrollView to the bottom.
    void ScrollToBottom(uint32_t scrollViewId);

private:
    /// Check if a point is inside a widget's bounds.
    bool IsPointInWidget(const UIWidget* widget, int32_t x, int32_t y) const;

    UIScreen* m_screen = nullptr;
    std::unordered_map<uint32_t, UIScrollState> m_scrollStates;
};

} // namespace atlas::ui
