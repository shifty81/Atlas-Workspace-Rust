#pragma once
#include "UIScreenGraph.h"
#include "UIEventRouter.h"
#include <cstdint>
#include <functional>
#include <vector>

namespace atlas::ui {

/// Callback invoked when a menu item is selected.
/// Parameters: menuId, menuItemId
using MenuItemCallback = std::function<void(uint32_t, uint32_t)>;

/// Manages menu state and interactions for dropdown and context menus.
/// Handles menu button clicks, dropdown visibility, keyboard navigation,
/// menu item selection, submenus, disabled items, and click-away behavior.
class MenuManager {
public:
    /// Initialize with a reference to the UI screen.
    void Init(UIScreen* screen);

    /// Update menu state based on input events.
    /// Call this before rendering to update hover states and handle clicks.
    /// Returns true if the event was consumed by the menu system.
    bool HandleEvent(const UIEvent& event);

    /// Update menu states (hover, etc) based on mouse position.
    void Update(int32_t mouseX, int32_t mouseY);

    /// Close all open menus.
    void CloseAllMenus();

    /// Set a callback to be invoked when a menu item is clicked.
    void SetMenuItemCallback(MenuItemCallback callback);

    /// Check if a point is inside a widget's bounds.
    bool IsPointInWidget(const UIWidget* widget, int32_t x, int32_t y) const;

    /// Returns true if any menu is currently open (dropdown or context).
    bool IsAnyMenuOpen() const;

    /// Returns the ID of the currently open menu (0 if none).
    uint32_t GetOpenMenuId() const;

    /// Returns the ID of the currently keyboard-focused menu item (0 if none).
    uint32_t GetFocusedItemId() const;

    // --- Context Menus ---

    /// Open a context menu at the given screen position.
    /// The contextMenuId should be a Menu widget already set up with MenuItem children.
    void OpenContextMenu(uint32_t contextMenuId, int32_t x, int32_t y);

    /// Close the currently open context menu, if any.
    void CloseContextMenu();

    /// Returns true if a context menu is currently open.
    bool IsContextMenuOpen() const;

    /// Returns the ID of the open context menu (0 if none).
    uint32_t GetContextMenuId() const;

    // --- Submenus ---

    /// Returns the ID of the currently open submenu (0 if none).
    uint32_t GetOpenSubmenuId() const;

private:
    /// Handle keyboard events for menu navigation.
    bool HandleKeyEvent(const UIEvent& event);

    /// Get ordered list of selectable item IDs for a given menu.
    std::vector<uint32_t> GetSelectableItems(uint32_t menuId) const;

    /// Move keyboard focus to the next/previous item.
    void MoveFocus(int direction);

    /// Select the currently focused item.
    void SelectFocusedItem();

    /// Open a submenu for the focused item, if it has one.
    void OpenSubmenu(uint32_t itemId);

    /// Close the currently open submenu.
    void CloseSubmenu();

    UIScreen* m_screen = nullptr;
    uint32_t m_openMenuId = 0;       // ID of currently open dropdown menu (0 = none)
    uint32_t m_focusedItemId = 0;    // ID of keyboard-focused menu item (0 = none)
    uint32_t m_contextMenuId = 0;    // ID of currently open context menu (0 = none)
    uint32_t m_openSubmenuId = 0;    // ID of currently open submenu (0 = none)
    MenuItemCallback m_menuItemCallback;
};

} // namespace atlas::ui
