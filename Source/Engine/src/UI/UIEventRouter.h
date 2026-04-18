#pragma once
#include "UIRenderer.h" // UIRect
#include <cstdint>
#include <vector>
#include <functional>

namespace atlas::ui {

/// Describes a single UI input event.
struct UIEvent {
    enum class Type : uint8_t {
        MouseMove,
        MouseDown,
        MouseUp,
        KeyDown,
        KeyUp,
        TextInput,
        ScrollWheel
    };

    Type type = Type::MouseMove;
    int32_t x = 0;
    int32_t y = 0;
    uint32_t keyCode = 0;
    uint8_t mouseButton = 0;
    char textChar = 0;
    float scrollDelta = 0.0f; ///< Scroll wheel delta (positive = down)
    uint8_t modifiers = 0;    ///< Bitmask: 1=Ctrl, 2=Shift, 4=Alt
};

/// Base class for any UI element that can receive routed events.
class UIEventTarget {
public:
    virtual ~UIEventTarget() = default;

    /// Returns true if the point (x, y) is inside this target.
    virtual bool HitTest(int32_t x, int32_t y) const = 0;

    /// Handle an event.  Return true to consume it (stop propagation).
    virtual bool OnEvent(const UIEvent& event) = 0;

    /// Z-order — higher values receive events first.
    int32_t zOrder = 0;
};

/// Tracks which UI element currently owns keyboard / mouse focus.
struct UIFocusState {
    UIEventTarget* hovered = nullptr;  ///< Element under the cursor
    UIEventTarget* active = nullptr;   ///< Element with mouse capture
    UIEventTarget* keyboard = nullptr; ///< Element with keyboard focus
};

/// Routes platform input events to the correct UI targets.
/// Targets are dispatched in descending z-order.  The first target
/// that consumes the event wins.
///
/// Usage:
///   router.Register(&myPanel);
///   ...
///   UIEvent e;
///   e.type = UIEvent::Type::MouseDown;
///   e.x = mouseX; e.y = mouseY;
///   bool consumed = router.Dispatch(e);
///   if (!consumed) { /* forward to gameplay */ }
class UIEventRouter {
public:
    /// Register a target for event dispatch.
    void Register(UIEventTarget* target);

    /// Remove a target from dispatch.
    void Unregister(UIEventTarget* target);

    /// Dispatch an event to all registered targets (highest z-order first).
    /// Returns true if the event was consumed by a target.
    bool Dispatch(const UIEvent& event);

    /// Access the current focus state.
    const UIFocusState& GetFocus() const;

    /// Clear all registered targets and reset focus state.
    void Clear();

    /// Returns the number of registered targets.
    size_t TargetCount() const;

private:
    std::vector<UIEventTarget*> m_targets;
    UIFocusState m_focus;
    bool m_needsSort = false;
};

} // namespace atlas::ui
