#pragma once
#include "UIScreenGraph.h"
#include <cstdint>
#include <unordered_map>

namespace atlas::ui {

/// Manages Tooltip widget visibility based on hover state and delay.
/// Widgets can have an associated tooltip widget ID. When the user
/// hovers over the owner widget for a configurable delay, the tooltip
/// becomes visible. When the mouse leaves, the tooltip is hidden.
class TooltipManager {
public:
    void Init(UIScreen* screen);

    /// Associate a tooltip widget with an owner widget.
    void SetTooltip(uint32_t ownerWidgetId, uint32_t tooltipWidgetId);

    /// Remove tooltip association for a widget.
    void RemoveTooltip(uint32_t ownerWidgetId);

    /// Returns the tooltip widget ID for an owner (0 if none).
    uint32_t GetTooltip(uint32_t ownerWidgetId) const;

    /// Set the hover delay in seconds before showing tooltip.
    void SetDelay(float seconds);

    /// Get the current hover delay in seconds.
    float GetDelay() const;

    /// Call each frame with mouse position and elapsed time since last frame.
    /// Updates tooltip visibility based on hover state and delay.
    void Update(int32_t mouseX, int32_t mouseY, float deltaTime);

    /// Hide all tooltips immediately.
    void HideAll();

    /// Returns the ID of the currently visible tooltip (0 if none).
    uint32_t GetActiveTooltip() const;

private:
    bool IsPointInWidget(const UIWidget* widget, int32_t x, int32_t y) const;

    UIScreen* m_screen = nullptr;
    std::unordered_map<uint32_t, uint32_t> m_tooltips; // ownerWidgetId -> tooltipWidgetId
    uint32_t m_hoveredOwner = 0;     // Widget currently hovered
    float m_hoverTime = 0.0f;        // How long we've been hovering
    float m_delay = 0.5f;            // Seconds before showing tooltip
    uint32_t m_activeTooltip = 0;    // Currently visible tooltip widget
};

} // namespace atlas::ui
