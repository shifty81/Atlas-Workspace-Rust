#pragma once
#include "UIScreenGraph.h"
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

namespace atlas::ui {

enum class DockSide : uint8_t {
    Left,
    Right,
    Top,
    Bottom,
    Center
};

/// Callback invoked when a panel is docked.
/// Parameters: dockAreaId, panelId, side
using DockCallback = std::function<void(uint32_t, uint32_t, DockSide)>;

/// Manages DockArea widgets — handles panel arrangement within dock zones.
class DockManager {
public:
    void Init(UIScreen* screen);

    /// Register a DockArea widget.
    void RegisterDockArea(uint32_t dockAreaId);

    /// Dock a panel into a dock area at the specified side.
    /// Adjusts positions/sizes of existing panels in the dock area.
    bool DockPanel(uint32_t dockAreaId, uint32_t panelId, DockSide side, float ratio = 0.5f);

    /// Undock (remove) a panel from its dock area. Returns true if found.
    bool UndockPanel(uint32_t panelId);

    /// Returns the dock area ID that a panel belongs to (0 if none).
    uint32_t GetPanelDockArea(uint32_t panelId) const;

    /// Returns all panels docked in a given dock area.
    std::vector<uint32_t> GetDockedPanels(uint32_t dockAreaId) const;

    /// Returns true if the dock area is registered.
    bool IsDockArea(uint32_t widgetId) const;

    /// Set callback for dock events.
    void SetDockCallback(DockCallback callback);

    /// Resize a split within a dock area by moving the divider.
    void SetSplitRatio(uint32_t dockAreaId, float ratio);

    /// Get the split ratio for a dock area (default 0.5).
    float GetSplitRatio(uint32_t dockAreaId) const;

private:
    struct DockAreaState {
        std::vector<uint32_t> panels;
        float splitRatio = 0.5f;
    };

    UIScreen* m_screen = nullptr;
    std::unordered_map<uint32_t, DockAreaState> m_dockAreas;
    std::unordered_map<uint32_t, uint32_t> m_panelToDock; // panelId -> dockAreaId
    DockCallback m_callback;
};

} // namespace atlas::ui
