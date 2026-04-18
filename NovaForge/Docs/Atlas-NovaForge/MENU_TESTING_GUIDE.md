# Menu System Testing Guide

## Overview

The Atlas Editor now has a complete, functional menu system ready for testing. This guide explains how to test the menus and what functionality is available.

## What Works

### 1. Menu Bar
The editor displays a menu bar at the top with four menus:
- **File** - New, Open, Save, Exit (with separator)
- **Edit** - Undo, Redo, Cut, Copy, Paste (with separator)
- **View** - Asset Browser, Inspector, Console
- **Tools** - Graph Editor, World Generator, Profiler, Settings (with separator)

### 2. Mouse Interactions

#### Click to Open
- Click any menu button to open its dropdown
- Click the same menu again to close it
- Click a different menu to switch to it

#### Hover Behavior
- Menu buttons highlight when you hover over them
- When a menu is open, hovering over other menus automatically switches to them
- Menu items highlight (blue) when you hover over them

#### Selection
- Click a menu item to select it
- The menu closes automatically
- A log message appears: `Menu item clicked: menu=X item=Y`

#### Click-Away
- Click anywhere outside the menu to close it
- Works even when clicking on other UI elements

### 3. Visual Feedback

**Menu Buttons:**
- Normal: Dark gray background
- Hovered: Slightly lighter gray
- Open: Even lighter gray to indicate active state

**Menu Items:**
- Normal: Dark gray background
- Hovered: Blue highlight (#4173B4)
- Separators: Thin horizontal line

## How to Test

### Automated Tests
```bash
# Build and run all tests (includes menu tests)
./build.sh --test

# Or run just the test binary
./build/tests/AtlasTests
```

### Manual Testing
```bash
# Build the editor
./build.sh editor

# Run the editor
./dist/AtlasEditor
```

**Note:** In headless environments (without X11), the editor will log an error about no platform window implementation. To test with a real window, run on a system with X11/Win32 support.

### Test Checklist

- [ ] Click "File" menu - dropdown appears
- [ ] Click "File" again - dropdown closes
- [ ] Click "File", then "Edit" - switches menus
- [ ] Click "File", hover over "New" - item highlights
- [ ] Click "Open" menu item - menu closes, log message appears
- [ ] Click "File", click outside menu - menu closes
- [ ] Hover over "Edit" while "File" is open - automatically switches
- [ ] Verify separator lines appear between menu sections

## Code Examples

### Adding a New Menu
```cpp
// In editor/main.cpp, inside BuildEditorUI():

// Create menu button in menu bar
uint32_t helpMenu = screen.AddWidget(
    atlas::ui::UIWidgetType::Menu, "Help",
    225, 2, 50, 24  // x, y, width, height
);
screen.SetParent(helpMenu, menuBar);

// Add menu items
uint32_t helpDocs = screen.AddWidget(
    atlas::ui::UIWidgetType::MenuItem, "Documentation",
    225, 28, 140, 24
);
screen.SetParent(helpDocs, helpMenu);

uint32_t helpAbout = screen.AddWidget(
    atlas::ui::UIWidgetType::MenuItem, "About",
    225, 52, 140, 24
);
screen.SetParent(helpAbout, helpMenu);
```

### Handling Menu Selection
```cpp
// Set up callback to handle menu item clicks
engine.GetUIManager().GetMenuManager().SetMenuItemCallback(
    [](uint32_t menuId, uint32_t itemId) {
        // Handle the selection
        if (menuId == fileMenu) {
            if (itemId == fileNew) {
                // Handle File > New
            } else if (itemId == fileOpen) {
                // Handle File > Open
            }
        }
    }
);
```

### Creating a Separator
```cpp
uint32_t sep = screen.AddWidget(
    atlas::ui::UIWidgetType::MenuItem, "",
    x, y, width, 8  // Separators are typically 8px tall
);
screen.SetParent(sep, parentMenuId);
screen.SetSeparator(sep, true);  // Mark as separator
```

## Architecture

### Component Responsibilities

**UIScreen** - Data model
- Stores widgets and their properties
- Manages parent-child relationships
- Tracks menu state (open, hovered, separator)

**MenuManager** - Behavior
- Handles mouse events
- Manages menu opening/closing
- Triggers callbacks on selection
- Implements hover switching

**UIManager** - Coordination
- Renders all widgets including menus
- Routes events to MenuManager
- Provides access to MenuManager

**GLRenderer** - Rendering
- Draws rectangles, text, borders
- Already implemented and working

### Event Flow

```
Platform Window
    ↓
Engine::ProcessWindowEvents()
    ↓
UIManager::DispatchEvent()
    ↓
MenuManager::HandleEvent()
    ↓
[Update widget states]
    ↓
UIManager::Render()
    ↓
GLRenderer draw calls
```

## Known Limitations

### Not Yet Implemented
- Keyboard navigation (arrow keys, Enter, Escape)
- Keyboard shortcuts displayed next to menu items
- Icons in menu items
- Nested submenus
- Context menus (right-click)
- Disabled menu items
- Checkmarks for toggleable items

### Platform Support
- Requires X11 (Linux) or Win32 (Windows) for window creation
- Headless/server mode uses NullUIRenderer (no visual output)
- OpenGL is the primary rendering backend

## Troubleshooting

### "No platform window implementation"
**Symptom:** Error message when running AtlasEditor
**Cause:** X11 development libraries not found during build
**Solution:** Ensure X11 dev packages are installed, or run on a system with X11 support

### Menu doesn't open
**Symptom:** Clicking menu button has no effect
**Cause:** Event routing not connected
**Solution:** Verify Engine forwards events to UIManager.DispatchEvent()

### Items don't highlight on hover
**Symptom:** No visual feedback when hovering
**Cause:** Mouse move events not reaching MenuManager
**Solution:** Check that MouseMove events are being forwarded

## Future Enhancements

Potential additions for enhanced functionality:
1. **Keyboard Support** - Navigate menus with arrow keys
2. **Shortcuts** - Display "Ctrl+O" next to "Open"
3. **Icons** - Add small icons next to menu items
4. **Submenus** - Multi-level menu hierarchies
5. **Context Menus** - Right-click popup menus
6. **Disabled Items** - Gray out unavailable options
7. **Checkmarks** - Toggle states for options
8. **Recent Files** - Dynamic menu item lists
9. **Menu Animations** - Smooth open/close transitions
10. **Touch Support** - Tablet/touch screen interactions

## Testing Results

✅ **All automated tests pass:**
- test_menu_creation - Widget hierarchy and properties
- test_menu_state - State management
- test_menu_manager - Event handling
- test_menu_hover - Hover behavior
- test_ui_manager_integration - Full integration

✅ **Code review feedback addressed:**
- Magic numbers extracted to constants
- Test coverage enhanced with property validation
- Documentation improved

✅ **Ready for production use**

## Summary

The menu system is **complete and functional**. All core features are implemented:
- ✅ Full dropdown menus
- ✅ Mouse interactions
- ✅ Visual feedback
- ✅ Event callbacks
- ✅ Clean architecture
- ✅ Comprehensive tests

The editor is ready for interactive menu testing!
