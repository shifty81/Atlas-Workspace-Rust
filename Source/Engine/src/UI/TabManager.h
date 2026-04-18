#pragma once
#include "UIScreenGraph.h"
#include <cstdint>
#include <functional>
#include <vector>

namespace atlas::ui {

/// Callback invoked when the active tab changes.
/// Parameters: tabGroupId, newActiveTabId, previousActiveTabId
using TabChangedCallback = std::function<void(uint32_t, uint32_t, uint32_t)>;

/// Manages tab switching and content panel visibility for Tab widgets.
///
/// Tabs are grouped by their parent widget ID.  Within each group,
/// exactly one tab is active at a time.  When the active tab changes,
/// the associated content panels are shown/hidden accordingly.
class TabManager {
public:
    /// Initialize with a reference to the UI screen.
    void Init(UIScreen* screen);

    /// Handle a click on a tab widget.  Activates the clicked tab
    /// and deactivates siblings in the same group.
    /// Returns true if a tab change occurred.
    bool ActivateTab(uint32_t tabId);

    /// Returns the currently active tab in a group (0 if none).
    uint32_t GetActiveTab(uint32_t groupId) const;

    /// Associate a content panel with a tab.  When the tab is active
    /// the panel is shown; when inactive the panel is hidden.
    void SetTabContent(uint32_t tabId, uint32_t contentPanelId);

    /// Returns the content panel associated with a tab (0 if none).
    uint32_t GetTabContent(uint32_t tabId) const;

    /// Set a callback invoked when the active tab changes.
    void SetTabChangedCallback(TabChangedCallback callback);

    /// Handle a mouse-down event.  Checks if a Tab widget was clicked
    /// and activates it.  Returns true if the event was consumed.
    bool HandleClick(int32_t mouseX, int32_t mouseY);

private:
    /// Update visibility of content panels based on active tab state.
    void UpdateContentVisibility(uint32_t groupId);

    UIScreen* m_screen = nullptr;
    TabChangedCallback m_callback;

    // tabId -> contentPanelId
    std::vector<std::pair<uint32_t, uint32_t>> m_tabContent;
};

} // namespace atlas::ui
