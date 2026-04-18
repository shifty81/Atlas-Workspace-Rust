#include "../engine/ui/UIScreenGraph.h"
#include "../engine/ui/UIManager.h"
#include "../engine/ui/MenuManager.h"
#include <cassert>
#include <iostream>

// ============================================================
// Task 1: Disabled Menu Items
// ============================================================

void test_widget_disabled_state() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t item = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Undo", 10, 30, 120, 24);

    // Widgets are enabled by default
    assert(!screen.IsDisabled(item));

    // Disable the widget
    screen.SetDisabled(item, true);
    assert(screen.IsDisabled(item));

    // Re-enable
    screen.SetDisabled(item, false);
    assert(!screen.IsDisabled(item));

    std::cout << "[PASS] test_widget_disabled_state" << std::endl;
}

void test_disabled_item_not_clickable() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t menu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "Edit", 10, 10, 60, 20);
    uint32_t item = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Undo", 10, 30, 120, 24);
    screen.SetParent(item, menu);
    screen.SetDisabled(item, true);

    atlas::ui::MenuManager menuMgr;
    menuMgr.Init(&screen);

    bool callbackInvoked = false;
    menuMgr.SetMenuItemCallback([&](uint32_t, uint32_t) {
        callbackInvoked = true;
    });

    // Open the menu
    atlas::ui::UIEvent clickMenu;
    clickMenu.type = atlas::ui::UIEvent::Type::MouseDown;
    clickMenu.x = 40;
    clickMenu.y = 15;
    clickMenu.mouseButton = 0;
    menuMgr.HandleEvent(clickMenu);
    assert(screen.IsMenuOpen(menu));

    // Try clicking disabled item — should NOT invoke callback
    atlas::ui::UIEvent clickItem;
    clickItem.type = atlas::ui::UIEvent::Type::MouseDown;
    clickItem.x = 50;
    clickItem.y = 40;
    clickItem.mouseButton = 0;
    menuMgr.HandleEvent(clickItem);
    assert(!callbackInvoked);

    std::cout << "[PASS] test_disabled_item_not_clickable" << std::endl;
}

void test_disabled_item_widget_direct() {
    atlas::ui::UIWidget widget;
    widget.isDisabled = false;
    assert(!widget.isDisabled);
    widget.isDisabled = true;
    assert(widget.isDisabled);

    std::cout << "[PASS] test_disabled_item_widget_direct" << std::endl;
}

// ============================================================
// Task 2: Keyboard Shortcut Labels
// ============================================================

void test_shortcut_label_on_widget() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t item = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Save", 10, 30, 200, 24);
    screen.SetShortcutLabel(item, "Ctrl+S");

    const auto* w = screen.GetWidget(item);
    assert(w != nullptr);
    assert(w->shortcutLabel == "Ctrl+S");

    std::cout << "[PASS] test_shortcut_label_on_widget" << std::endl;
}

void test_shortcut_label_empty_by_default() {
    atlas::ui::UIWidget widget;
    assert(widget.shortcutLabel.empty());

    std::cout << "[PASS] test_shortcut_label_empty_by_default" << std::endl;
}

void test_shortcut_label_multiple_items() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t menu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 10, 60, 20);

    uint32_t itemNew  = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "New",  10, 30, 200, 24);
    uint32_t itemOpen = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Open", 10, 54, 200, 24);
    uint32_t itemSave = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Save", 10, 78, 200, 24);

    screen.SetParent(itemNew, menu);
    screen.SetParent(itemOpen, menu);
    screen.SetParent(itemSave, menu);

    screen.SetShortcutLabel(itemNew,  "Ctrl+N");
    screen.SetShortcutLabel(itemOpen, "Ctrl+O");
    screen.SetShortcutLabel(itemSave, "Ctrl+S");

    assert(screen.GetWidget(itemNew)->shortcutLabel  == "Ctrl+N");
    assert(screen.GetWidget(itemOpen)->shortcutLabel == "Ctrl+O");
    assert(screen.GetWidget(itemSave)->shortcutLabel == "Ctrl+S");

    std::cout << "[PASS] test_shortcut_label_multiple_items" << std::endl;
}

// ============================================================
// Task 3: Keyboard Navigation
// ============================================================

void test_keyboard_nav_down() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t menu  = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 10, 60, 20);
    uint32_t item1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "New",  10, 30, 120, 24);
    uint32_t item2 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Open", 10, 54, 120, 24);
    uint32_t item3 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Save", 10, 78, 120, 24);
    screen.SetParent(item1, menu);
    screen.SetParent(item2, menu);
    screen.SetParent(item3, menu);

    atlas::ui::MenuManager menuMgr;
    menuMgr.Init(&screen);

    // Open menu
    atlas::ui::UIEvent clickMenu;
    clickMenu.type = atlas::ui::UIEvent::Type::MouseDown;
    clickMenu.x = 40;
    clickMenu.y = 15;
    clickMenu.mouseButton = 0;
    menuMgr.HandleEvent(clickMenu);
    assert(menuMgr.IsAnyMenuOpen());
    assert(menuMgr.GetOpenMenuId() == menu);

    // Press Down arrow — should focus first item
    atlas::ui::UIEvent keyDown;
    keyDown.type = atlas::ui::UIEvent::Type::KeyDown;
    keyDown.keyCode = 0x28; // VK_DOWN
    bool consumed = menuMgr.HandleEvent(keyDown);
    assert(consumed);
    assert(menuMgr.GetFocusedItemId() == item1);

    // Press Down again — should focus second item
    consumed = menuMgr.HandleEvent(keyDown);
    assert(consumed);
    assert(menuMgr.GetFocusedItemId() == item2);

    // Press Down again — should focus third item
    consumed = menuMgr.HandleEvent(keyDown);
    assert(consumed);
    assert(menuMgr.GetFocusedItemId() == item3);

    // Press Down again — should wrap to first item
    consumed = menuMgr.HandleEvent(keyDown);
    assert(consumed);
    assert(menuMgr.GetFocusedItemId() == item1);

    std::cout << "[PASS] test_keyboard_nav_down" << std::endl;
}

void test_keyboard_nav_up() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t menu  = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 10, 60, 20);
    uint32_t item1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "New",  10, 30, 120, 24);
    uint32_t item2 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Open", 10, 54, 120, 24);
    screen.SetParent(item1, menu);
    screen.SetParent(item2, menu);

    atlas::ui::MenuManager menuMgr;
    menuMgr.Init(&screen);

    // Open menu
    atlas::ui::UIEvent clickMenu;
    clickMenu.type = atlas::ui::UIEvent::Type::MouseDown;
    clickMenu.x = 40;
    clickMenu.y = 15;
    clickMenu.mouseButton = 0;
    menuMgr.HandleEvent(clickMenu);

    // Press Up arrow — should focus last item
    atlas::ui::UIEvent keyUp;
    keyUp.type = atlas::ui::UIEvent::Type::KeyDown;
    keyUp.keyCode = 0x26; // VK_UP
    menuMgr.HandleEvent(keyUp);
    assert(menuMgr.GetFocusedItemId() == item2);

    // Press Up again — should go to first
    menuMgr.HandleEvent(keyUp);
    assert(menuMgr.GetFocusedItemId() == item1);

    // Press Up again — should wrap to last
    menuMgr.HandleEvent(keyUp);
    assert(menuMgr.GetFocusedItemId() == item2);

    std::cout << "[PASS] test_keyboard_nav_up" << std::endl;
}

void test_keyboard_nav_enter_select() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t menu  = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 10, 60, 20);
    uint32_t item1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Open", 10, 30, 120, 24);
    screen.SetParent(item1, menu);

    atlas::ui::MenuManager menuMgr;
    menuMgr.Init(&screen);

    bool callbackInvoked = false;
    uint32_t selectedMenu = 0;
    uint32_t selectedItem = 0;
    menuMgr.SetMenuItemCallback([&](uint32_t m, uint32_t i) {
        callbackInvoked = true;
        selectedMenu = m;
        selectedItem = i;
    });

    // Open menu
    atlas::ui::UIEvent clickMenu;
    clickMenu.type = atlas::ui::UIEvent::Type::MouseDown;
    clickMenu.x = 40;
    clickMenu.y = 15;
    clickMenu.mouseButton = 0;
    menuMgr.HandleEvent(clickMenu);

    // Down to select first item
    atlas::ui::UIEvent keyDown;
    keyDown.type = atlas::ui::UIEvent::Type::KeyDown;
    keyDown.keyCode = 0x28;
    menuMgr.HandleEvent(keyDown);
    assert(menuMgr.GetFocusedItemId() == item1);

    // Press Enter
    atlas::ui::UIEvent keyEnter;
    keyEnter.type = atlas::ui::UIEvent::Type::KeyDown;
    keyEnter.keyCode = 0x0D;
    menuMgr.HandleEvent(keyEnter);

    assert(callbackInvoked);
    assert(selectedMenu == menu);
    assert(selectedItem == item1);
    // Menu should be closed after selection
    assert(!menuMgr.IsAnyMenuOpen());

    std::cout << "[PASS] test_keyboard_nav_enter_select" << std::endl;
}

void test_keyboard_nav_escape_close() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t menu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 10, 60, 20);
    uint32_t item = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "New", 10, 30, 120, 24);
    screen.SetParent(item, menu);

    atlas::ui::MenuManager menuMgr;
    menuMgr.Init(&screen);

    // Open menu
    atlas::ui::UIEvent clickMenu;
    clickMenu.type = atlas::ui::UIEvent::Type::MouseDown;
    clickMenu.x = 40;
    clickMenu.y = 15;
    clickMenu.mouseButton = 0;
    menuMgr.HandleEvent(clickMenu);
    assert(menuMgr.IsAnyMenuOpen());

    // Press Escape
    atlas::ui::UIEvent keyEsc;
    keyEsc.type = atlas::ui::UIEvent::Type::KeyDown;
    keyEsc.keyCode = 0x1B;
    bool consumed = menuMgr.HandleEvent(keyEsc);
    assert(consumed);
    assert(!menuMgr.IsAnyMenuOpen());

    std::cout << "[PASS] test_keyboard_nav_escape_close" << std::endl;
}

void test_keyboard_nav_skip_disabled() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t menu  = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "Edit", 10, 10, 60, 20);
    uint32_t item1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Undo",  10, 30, 120, 24);
    uint32_t item2 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Redo",  10, 54, 120, 24);
    uint32_t item3 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Cut",   10, 78, 120, 24);
    screen.SetParent(item1, menu);
    screen.SetParent(item2, menu);
    screen.SetParent(item3, menu);

    // Disable Redo
    screen.SetDisabled(item2, true);

    atlas::ui::MenuManager menuMgr;
    menuMgr.Init(&screen);

    // Open menu
    atlas::ui::UIEvent clickMenu;
    clickMenu.type = atlas::ui::UIEvent::Type::MouseDown;
    clickMenu.x = 40;
    clickMenu.y = 15;
    clickMenu.mouseButton = 0;
    menuMgr.HandleEvent(clickMenu);

    // Press Down — should focus first enabled item (Undo)
    atlas::ui::UIEvent keyDown;
    keyDown.type = atlas::ui::UIEvent::Type::KeyDown;
    keyDown.keyCode = 0x28;
    menuMgr.HandleEvent(keyDown);
    assert(menuMgr.GetFocusedItemId() == item1);

    // Press Down — should skip disabled item and go to Cut
    menuMgr.HandleEvent(keyDown);
    assert(menuMgr.GetFocusedItemId() == item3);

    std::cout << "[PASS] test_keyboard_nav_skip_disabled" << std::endl;
}

void test_keyboard_nav_skip_separator() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t menu  = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 10, 60, 20);
    uint32_t item1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "New",  10, 30, 120, 24);
    uint32_t sep   = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "",     10, 54, 120, 8);
    uint32_t item2 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Exit", 10, 62, 120, 24);
    screen.SetParent(item1, menu);
    screen.SetParent(sep, menu);
    screen.SetParent(item2, menu);
    screen.SetSeparator(sep, true);

    atlas::ui::MenuManager menuMgr;
    menuMgr.Init(&screen);

    // Open menu
    atlas::ui::UIEvent clickMenu;
    clickMenu.type = atlas::ui::UIEvent::Type::MouseDown;
    clickMenu.x = 40;
    clickMenu.y = 15;
    clickMenu.mouseButton = 0;
    menuMgr.HandleEvent(clickMenu);

    // Down → New
    atlas::ui::UIEvent keyDown;
    keyDown.type = atlas::ui::UIEvent::Type::KeyDown;
    keyDown.keyCode = 0x28;
    menuMgr.HandleEvent(keyDown);
    assert(menuMgr.GetFocusedItemId() == item1);

    // Down → skip separator → Exit
    menuMgr.HandleEvent(keyDown);
    assert(menuMgr.GetFocusedItemId() == item2);

    std::cout << "[PASS] test_keyboard_nav_skip_separator" << std::endl;
}

void test_keyboard_nav_left_right_switch_menu() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t menu1 = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 10, 60, 20);
    uint32_t menu2 = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "Edit", 80, 10, 60, 20);
    uint32_t item1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "New",  10, 30, 120, 24);
    uint32_t item2 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Undo", 80, 30, 120, 24);
    screen.SetParent(item1, menu1);
    screen.SetParent(item2, menu2);

    atlas::ui::MenuManager menuMgr;
    menuMgr.Init(&screen);

    // Open File menu
    atlas::ui::UIEvent clickMenu;
    clickMenu.type = atlas::ui::UIEvent::Type::MouseDown;
    clickMenu.x = 40;
    clickMenu.y = 15;
    clickMenu.mouseButton = 0;
    menuMgr.HandleEvent(clickMenu);
    assert(menuMgr.GetOpenMenuId() == menu1);

    // Press Right → switch to Edit menu
    atlas::ui::UIEvent keyRight;
    keyRight.type = atlas::ui::UIEvent::Type::KeyDown;
    keyRight.keyCode = 0x27;
    menuMgr.HandleEvent(keyRight);
    assert(menuMgr.GetOpenMenuId() == menu2);

    // Press Left → switch back to File menu
    atlas::ui::UIEvent keyLeft;
    keyLeft.type = atlas::ui::UIEvent::Type::KeyDown;
    keyLeft.keyCode = 0x25;
    menuMgr.HandleEvent(keyLeft);
    assert(menuMgr.GetOpenMenuId() == menu1);

    std::cout << "[PASS] test_keyboard_nav_left_right_switch_menu" << std::endl;
}

void test_keyboard_no_consume_when_closed() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t menu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 10, 60, 20);
    uint32_t item = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "New", 10, 30, 120, 24);
    screen.SetParent(item, menu);

    atlas::ui::MenuManager menuMgr;
    menuMgr.Init(&screen);

    // No menu is open, keyboard events should not be consumed
    atlas::ui::UIEvent keyDown;
    keyDown.type = atlas::ui::UIEvent::Type::KeyDown;
    keyDown.keyCode = 0x28;
    bool consumed = menuMgr.HandleEvent(keyDown);
    assert(!consumed);

    std::cout << "[PASS] test_keyboard_no_consume_when_closed" << std::endl;
}

// ============================================================
// Task 4: Context Menus
// ============================================================

void test_context_menu_open_close() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    // Create a context menu (initially hidden)
    uint32_t ctxMenu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "ContextMenu", 0, 0, 150, 0);
    screen.SetVisible(ctxMenu, false);
    uint32_t ctxItem1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Copy", 0, 0, 150, 24);
    uint32_t ctxItem2 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Paste", 0, 0, 150, 24);
    screen.SetParent(ctxItem1, ctxMenu);
    screen.SetParent(ctxItem2, ctxMenu);
    screen.SetVisible(ctxItem1, false);
    screen.SetVisible(ctxItem2, false);

    atlas::ui::MenuManager menuMgr;
    menuMgr.Init(&screen);

    assert(!menuMgr.IsContextMenuOpen());

    // Open context menu at position (200, 300)
    menuMgr.OpenContextMenu(ctxMenu, 200, 300);
    assert(menuMgr.IsContextMenuOpen());
    assert(menuMgr.GetContextMenuId() == ctxMenu);

    // Verify items are now visible and positioned
    const auto* item1 = screen.GetWidget(ctxItem1);
    assert(item1->visible);

    // Close it
    menuMgr.CloseContextMenu();
    assert(!menuMgr.IsContextMenuOpen());

    std::cout << "[PASS] test_context_menu_open_close" << std::endl;
}

void test_context_menu_item_click() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t ctxMenu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "ContextMenu", 0, 0, 150, 0);
    screen.SetVisible(ctxMenu, false);
    uint32_t ctxItem1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Delete", 0, 0, 150, 24);
    screen.SetParent(ctxItem1, ctxMenu);
    screen.SetVisible(ctxItem1, false);

    atlas::ui::MenuManager menuMgr;
    menuMgr.Init(&screen);

    bool callbackInvoked = false;
    uint32_t selectedItem = 0;
    menuMgr.SetMenuItemCallback([&](uint32_t, uint32_t itemId) {
        callbackInvoked = true;
        selectedItem = itemId;
    });

    // Open context menu at (100, 100)
    menuMgr.OpenContextMenu(ctxMenu, 100, 100);

    // Click on the item (positioned at 100, 100 by OpenContextMenu)
    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.x = 110;
    click.y = 110;
    click.mouseButton = 0;
    menuMgr.HandleEvent(click);

    assert(callbackInvoked);
    assert(selectedItem == ctxItem1);
    assert(!menuMgr.IsContextMenuOpen());

    std::cout << "[PASS] test_context_menu_item_click" << std::endl;
}

void test_context_menu_keyboard_nav() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t ctxMenu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "ContextMenu", 0, 0, 150, 0);
    screen.SetVisible(ctxMenu, false);
    uint32_t ctxItem1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Cut", 0, 0, 150, 24);
    uint32_t ctxItem2 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Copy", 0, 0, 150, 24);
    screen.SetParent(ctxItem1, ctxMenu);
    screen.SetParent(ctxItem2, ctxMenu);

    atlas::ui::MenuManager menuMgr;
    menuMgr.Init(&screen);

    menuMgr.OpenContextMenu(ctxMenu, 50, 50);
    assert(menuMgr.IsContextMenuOpen());

    // Navigate with keyboard
    atlas::ui::UIEvent keyDown;
    keyDown.type = atlas::ui::UIEvent::Type::KeyDown;
    keyDown.keyCode = 0x28;
    menuMgr.HandleEvent(keyDown);
    assert(menuMgr.GetFocusedItemId() == ctxItem1);

    menuMgr.HandleEvent(keyDown);
    assert(menuMgr.GetFocusedItemId() == ctxItem2);

    // Escape closes context menu
    atlas::ui::UIEvent keyEsc;
    keyEsc.type = atlas::ui::UIEvent::Type::KeyDown;
    keyEsc.keyCode = 0x1B;
    menuMgr.HandleEvent(keyEsc);
    assert(!menuMgr.IsContextMenuOpen());

    std::cout << "[PASS] test_context_menu_keyboard_nav" << std::endl;
}

// ============================================================
// Task 5: Submenus
// ============================================================

void test_submenu_flag_on_widget() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t item = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Export", 10, 30, 120, 24);

    // Not a submenu by default
    const auto* w = screen.GetWidget(item);
    assert(!w->hasSubmenu);

    screen.SetHasSubmenu(item, true);
    w = screen.GetWidget(item);
    assert(w->hasSubmenu);

    std::cout << "[PASS] test_submenu_flag_on_widget" << std::endl;
}

void test_submenu_open_via_keyboard() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t menu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 10, 60, 20);
    uint32_t item1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Export", 10, 30, 120, 24);
    screen.SetParent(item1, menu);
    screen.SetHasSubmenu(item1, true);

    // Submenu children
    uint32_t subItem1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "PNG", 130, 30, 120, 24);
    uint32_t subItem2 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "JPG", 130, 54, 120, 24);
    screen.SetParent(subItem1, item1);
    screen.SetParent(subItem2, item1);

    atlas::ui::MenuManager menuMgr;
    menuMgr.Init(&screen);

    // Open menu
    atlas::ui::UIEvent clickMenu;
    clickMenu.type = atlas::ui::UIEvent::Type::MouseDown;
    clickMenu.x = 40;
    clickMenu.y = 15;
    clickMenu.mouseButton = 0;
    menuMgr.HandleEvent(clickMenu);

    // Navigate to Export item
    atlas::ui::UIEvent keyDown;
    keyDown.type = atlas::ui::UIEvent::Type::KeyDown;
    keyDown.keyCode = 0x28;
    menuMgr.HandleEvent(keyDown);
    assert(menuMgr.GetFocusedItemId() == item1);

    // Press Right to open submenu
    atlas::ui::UIEvent keyRight;
    keyRight.type = atlas::ui::UIEvent::Type::KeyDown;
    keyRight.keyCode = 0x27;
    menuMgr.HandleEvent(keyRight);
    assert(menuMgr.GetOpenSubmenuId() == item1);

    std::cout << "[PASS] test_submenu_open_via_keyboard" << std::endl;
}

void test_submenu_close_via_left() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t menu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 10, 60, 20);
    uint32_t item1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Export", 10, 30, 120, 24);
    screen.SetParent(item1, menu);
    screen.SetHasSubmenu(item1, true);

    uint32_t subItem1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "PNG", 130, 30, 120, 24);
    screen.SetParent(subItem1, item1);

    atlas::ui::MenuManager menuMgr;
    menuMgr.Init(&screen);

    // Open menu
    atlas::ui::UIEvent clickMenu;
    clickMenu.type = atlas::ui::UIEvent::Type::MouseDown;
    clickMenu.x = 40;
    clickMenu.y = 15;
    clickMenu.mouseButton = 0;
    menuMgr.HandleEvent(clickMenu);

    // Navigate to Export and open submenu
    atlas::ui::UIEvent keyDown;
    keyDown.type = atlas::ui::UIEvent::Type::KeyDown;
    keyDown.keyCode = 0x28;
    menuMgr.HandleEvent(keyDown);

    atlas::ui::UIEvent keyRight;
    keyRight.type = atlas::ui::UIEvent::Type::KeyDown;
    keyRight.keyCode = 0x27;
    menuMgr.HandleEvent(keyRight);
    assert(menuMgr.GetOpenSubmenuId() == item1);

    // Press Left to close submenu
    atlas::ui::UIEvent keyLeft;
    keyLeft.type = atlas::ui::UIEvent::Type::KeyDown;
    keyLeft.keyCode = 0x25;
    menuMgr.HandleEvent(keyLeft);
    assert(menuMgr.GetOpenSubmenuId() == 0);
    // Focus should return to Export item
    assert(menuMgr.GetFocusedItemId() == item1);

    std::cout << "[PASS] test_submenu_close_via_left" << std::endl;
}

void test_submenu_escape_closes_submenu_first() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t menu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 10, 60, 20);
    uint32_t item1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Export", 10, 30, 120, 24);
    screen.SetParent(item1, menu);
    screen.SetHasSubmenu(item1, true);

    uint32_t subItem1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "PNG", 130, 30, 120, 24);
    screen.SetParent(subItem1, item1);

    atlas::ui::MenuManager menuMgr;
    menuMgr.Init(&screen);

    // Open menu, navigate to Export, open submenu
    atlas::ui::UIEvent clickMenu;
    clickMenu.type = atlas::ui::UIEvent::Type::MouseDown;
    clickMenu.x = 40;
    clickMenu.y = 15;
    clickMenu.mouseButton = 0;
    menuMgr.HandleEvent(clickMenu);

    atlas::ui::UIEvent keyDown;
    keyDown.type = atlas::ui::UIEvent::Type::KeyDown;
    keyDown.keyCode = 0x28;
    menuMgr.HandleEvent(keyDown);

    atlas::ui::UIEvent keyRight;
    keyRight.type = atlas::ui::UIEvent::Type::KeyDown;
    keyRight.keyCode = 0x27;
    menuMgr.HandleEvent(keyRight);
    assert(menuMgr.GetOpenSubmenuId() == item1);

    // First Escape closes submenu, not the main menu
    atlas::ui::UIEvent keyEsc;
    keyEsc.type = atlas::ui::UIEvent::Type::KeyDown;
    keyEsc.keyCode = 0x1B;
    menuMgr.HandleEvent(keyEsc);
    assert(menuMgr.GetOpenSubmenuId() == 0);
    assert(menuMgr.IsAnyMenuOpen()); // Main menu still open

    // Second Escape closes main menu
    menuMgr.HandleEvent(keyEsc);
    assert(!menuMgr.IsAnyMenuOpen());

    std::cout << "[PASS] test_submenu_escape_closes_submenu_first" << std::endl;
}

// ============================================================
// Task 6: Toolbar Widget
// ============================================================

void test_toolbar_widget_creation() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t toolbar = screen.AddWidget(atlas::ui::UIWidgetType::Toolbar, "MainToolbar", 0, 30, 800, 36);
    assert(screen.WidgetCount() == 1);

    const auto* w = screen.GetWidget(toolbar);
    assert(w != nullptr);
    assert(w->type == atlas::ui::UIWidgetType::Toolbar);
    assert(w->name == "MainToolbar");
    assert(w->width == 800.0f);
    assert(w->height == 36.0f);

    std::cout << "[PASS] test_toolbar_widget_creation" << std::endl;
}

void test_toolbar_with_button_children() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t toolbar = screen.AddWidget(atlas::ui::UIWidgetType::Toolbar, "Toolbar", 0, 30, 800, 36);
    uint32_t btn1 = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Save", 4, 34, 28, 28);
    uint32_t btn2 = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Undo", 36, 34, 28, 28);
    uint32_t btn3 = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Redo", 68, 34, 28, 28);
    screen.SetParent(btn1, toolbar);
    screen.SetParent(btn2, toolbar);
    screen.SetParent(btn3, toolbar);

    auto children = screen.GetChildren(toolbar);
    assert(children.size() == 3);

    std::cout << "[PASS] test_toolbar_with_button_children" << std::endl;
}

void test_toolbar_rendering_with_null_renderer() {
    atlas::ui::UIManager uiMgr;
    uiMgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = uiMgr.GetScreen();
    uint32_t toolbar = screen.AddWidget(atlas::ui::UIWidgetType::Toolbar, "Toolbar", 0, 30, 800, 36);

    atlas::ui::NullUIRenderer renderer;
    // Should not crash
    uiMgr.Render(&renderer);

    std::cout << "[PASS] test_toolbar_rendering_with_null_renderer" << std::endl;
}

// ============================================================
// Task 7: StatusBar Widget
// ============================================================

void test_statusbar_widget_creation() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t statusBar = screen.AddWidget(atlas::ui::UIWidgetType::StatusBar, "Ready", 0, 580, 800, 20);
    assert(screen.WidgetCount() == 1);

    const auto* w = screen.GetWidget(statusBar);
    assert(w != nullptr);
    assert(w->type == atlas::ui::UIWidgetType::StatusBar);
    assert(w->name == "Ready");
    assert(w->y == 580.0f);
    assert(w->height == 20.0f);

    std::cout << "[PASS] test_statusbar_widget_creation" << std::endl;
}

void test_statusbar_text_update() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t statusBar = screen.AddWidget(atlas::ui::UIWidgetType::StatusBar, "Ready", 0, 580, 800, 20);

    auto* w = screen.GetWidgetMutable(statusBar);
    assert(w != nullptr);
    w->name = "Building project...";
    assert(screen.GetWidget(statusBar)->name == "Building project...");

    std::cout << "[PASS] test_statusbar_text_update" << std::endl;
}

void test_statusbar_rendering_with_null_renderer() {
    atlas::ui::UIManager uiMgr;
    uiMgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = uiMgr.GetScreen();
    screen.AddWidget(atlas::ui::UIWidgetType::StatusBar, "Ready | Ln 1, Col 1", 0, 580, 800, 20);

    atlas::ui::NullUIRenderer renderer;
    // Should not crash
    uiMgr.Render(&renderer);

    std::cout << "[PASS] test_statusbar_rendering_with_null_renderer" << std::endl;
}

// ============================================================
// Task 8: Integration Tests
// ============================================================

void test_full_editor_menu_setup() {
    atlas::ui::UIManager uiMgr;
    uiMgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = uiMgr.GetScreen();

    // Menu bar
    uint32_t menuBar = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "MenuBar", 0, 0, 800, 30);

    // File menu
    uint32_t fileMenu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 5, 60, 20);
    screen.SetParent(fileMenu, menuBar);

    uint32_t fileNew  = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "New",  10, 30, 200, 24);
    uint32_t fileOpen = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Open", 10, 54, 200, 24);
    uint32_t fileSave = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Save", 10, 78, 200, 24);
    uint32_t fileSep  = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "",     10, 102, 200, 8);
    uint32_t fileExit = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Exit", 10, 110, 200, 24);
    screen.SetParent(fileNew, fileMenu);
    screen.SetParent(fileOpen, fileMenu);
    screen.SetParent(fileSave, fileMenu);
    screen.SetParent(fileSep, fileMenu);
    screen.SetParent(fileExit, fileMenu);

    screen.SetShortcutLabel(fileNew,  "Ctrl+N");
    screen.SetShortcutLabel(fileOpen, "Ctrl+O");
    screen.SetShortcutLabel(fileSave, "Ctrl+S");
    screen.SetSeparator(fileSep, true);

    // Edit menu with disabled items
    uint32_t editMenu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "Edit", 80, 5, 60, 20);
    screen.SetParent(editMenu, menuBar);

    uint32_t editUndo = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Undo", 80, 30, 200, 24);
    uint32_t editRedo = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Redo", 80, 54, 200, 24);
    screen.SetParent(editUndo, editMenu);
    screen.SetParent(editRedo, editMenu);
    screen.SetShortcutLabel(editUndo, "Ctrl+Z");
    screen.SetShortcutLabel(editRedo, "Ctrl+Y");
    screen.SetDisabled(editRedo, true);

    // Toolbar
    uint32_t toolbar = screen.AddWidget(atlas::ui::UIWidgetType::Toolbar, "MainToolbar", 0, 30, 800, 36);

    // StatusBar
    uint32_t statusBar = screen.AddWidget(atlas::ui::UIWidgetType::StatusBar, "Ready", 0, 580, 800, 20);

    // Verify full structure
    assert(screen.GetWidget(fileNew)->shortcutLabel == "Ctrl+N");
    assert(screen.GetWidget(editRedo)->isDisabled);
    assert(screen.GetWidget(toolbar)->type == atlas::ui::UIWidgetType::Toolbar);
    assert(screen.GetWidget(statusBar)->type == atlas::ui::UIWidgetType::StatusBar);

    // Render with null renderer — should not crash
    atlas::ui::NullUIRenderer renderer;
    uiMgr.Render(&renderer);

    std::cout << "[PASS] test_full_editor_menu_setup" << std::endl;
}

void test_menu_manager_is_any_open() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t menu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 10, 60, 20);
    uint32_t item = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "New", 10, 30, 120, 24);
    screen.SetParent(item, menu);

    atlas::ui::MenuManager menuMgr;
    menuMgr.Init(&screen);

    assert(!menuMgr.IsAnyMenuOpen());

    // Open menu
    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.x = 40;
    click.y = 15;
    click.mouseButton = 0;
    menuMgr.HandleEvent(click);
    assert(menuMgr.IsAnyMenuOpen());

    menuMgr.CloseAllMenus();
    assert(!menuMgr.IsAnyMenuOpen());

    std::cout << "[PASS] test_menu_manager_is_any_open" << std::endl;
}

void test_close_all_menus_clears_focus() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t menu  = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File", 10, 10, 60, 20);
    uint32_t item1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "New", 10, 30, 120, 24);
    screen.SetParent(item1, menu);

    atlas::ui::MenuManager menuMgr;
    menuMgr.Init(&screen);

    // Open and navigate
    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.x = 40;
    click.y = 15;
    click.mouseButton = 0;
    menuMgr.HandleEvent(click);

    atlas::ui::UIEvent keyDown;
    keyDown.type = atlas::ui::UIEvent::Type::KeyDown;
    keyDown.keyCode = 0x28;
    menuMgr.HandleEvent(keyDown);
    assert(menuMgr.GetFocusedItemId() != 0);

    menuMgr.CloseAllMenus();
    assert(menuMgr.GetFocusedItemId() == 0);

    std::cout << "[PASS] test_close_all_menus_clears_focus" << std::endl;
}
