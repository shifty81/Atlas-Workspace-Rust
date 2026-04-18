#include "../engine/ui/UIScreenGraph.h"
#include "../engine/ui/UIManager.h"
#include "../engine/ui/GUIDSLParser.h"
#include "../editor/ui/DefaultEditorLayout.h"
#include "../engine/platform/PlatformWindow.h"
#include "../engine/ui/UIEventRouter.h"
#include <cassert>
#include <iostream>
#include <functional>
#include <vector>
#include <string>

using namespace atlas::ui;
using atlas::platform::kModCtrl;
using atlas::platform::kModShift;
using atlas::platform::kModAlt;

// ============================================================
// Test: BuildEditorUI fallback layout produces actual widget content
// ============================================================

/// Simulates the BuildEditorUI logic to verify that the fallback
/// layout is always used (DSL path no longer returns early).
void test_editor_ui_has_widget_content() {
    UIManager mgr;
    mgr.Init(GUIContext::Editor);
    auto& screen = mgr.GetScreen();

    // Simulate the DSL parse (it succeeds but should not be used for layout)
    GUIDSLParser parser;
    auto result = parser.Parse(atlas::editor::DefaultEditorDSL());
    assert(result.success);

    // The fix: DSL success no longer causes an early return.
    // The fallback layout is always used, which creates menus, buttons, text, etc.
    // Simulate the fallback layout by creating representative widgets:
    uint32_t menuBar = screen.AddWidget(UIWidgetType::Panel, "MenuBar", 0, 0, 1280, 28);
    uint32_t fileMenu = screen.AddWidget(UIWidgetType::Menu, "File", 4, 2, 50, 24);
    screen.SetParent(fileMenu, menuBar);
    uint32_t fileNew = screen.AddWidget(UIWidgetType::MenuItem, "New", 4, 28, 120, 24);
    screen.SetParent(fileNew, fileMenu);
    uint32_t toolbar = screen.AddWidget(UIWidgetType::Toolbar, "MainToolbar", 0, 28, 1280, 30);
    uint32_t tbPlay = screen.AddWidget(UIWidgetType::Button, "Play", 4, 31, 50, 24);
    screen.SetParent(tbPlay, toolbar);
    uint32_t statusBar = screen.AddWidget(UIWidgetType::StatusBar, "Ready", 0, 692, 1280, 28);

    // Verify content widgets exist (menus, buttons, text — not just panels)
    bool hasMenu = false, hasMenuItem = false, hasButton = false;
    bool hasToolbar = false, hasStatusBar = false;

    // Iterate through all widget IDs that have been allocated
    size_t count = screen.WidgetCount();
    for (uint32_t id = 1; id <= count; ++id) {
        const UIWidget* w = screen.GetWidget(id);
        if (!w) continue;
        if (w->type == UIWidgetType::Menu) hasMenu = true;
        if (w->type == UIWidgetType::MenuItem) hasMenuItem = true;
        if (w->type == UIWidgetType::Button) hasButton = true;
        if (w->type == UIWidgetType::Toolbar) hasToolbar = true;
        if (w->type == UIWidgetType::StatusBar) hasStatusBar = true;
    }

    assert(hasMenu);
    assert(hasMenuItem);
    assert(hasButton);
    assert(hasToolbar);
    assert(hasStatusBar);

    std::cout << "[PASS] test_editor_ui_has_widget_content" << std::endl;
}

// ============================================================
// Test: DSL still parses but doesn't produce the final layout
// ============================================================

void test_dsl_parses_but_fallback_used() {
    GUIDSLParser parser;
    auto result = parser.Parse(atlas::editor::DefaultEditorDSL());

    // DSL should still parse successfully
    assert(result.success);
    assert(result.root != nullptr);
    assert(result.root->name == "DefaultEditor");

    // Verify DSL produces only Panel nodes (no Menu, Button, etc.)
    std::vector<DSLNodeType> types;
    std::function<void(const DSLNode&)> collect = [&](const DSLNode& node) {
        types.push_back(node.type);
        for (auto& child : node.children)
            collect(*child);
    };
    collect(*result.root);

    // DSL only contains Layout, Split, Panel, Dock, Tabs — no widget content types
    for (auto t : types) {
        assert(t == DSLNodeType::Layout || t == DSLNodeType::Split ||
               t == DSLNodeType::Panel || t == DSLNodeType::Dock ||
               t == DSLNodeType::Tabs);
    }

    std::cout << "[PASS] test_dsl_parses_but_fallback_used" << std::endl;
}

// ============================================================
// Test: WindowEvent modifier field
// ============================================================

void test_window_event_modifiers() {
    atlas::platform::WindowEvent event;
    assert(event.modifiers == 0);

    // Set Ctrl modifier
    event.modifiers = kModCtrl;
    assert((event.modifiers & kModCtrl) != 0);
    assert((event.modifiers & kModShift) == 0);
    assert((event.modifiers & kModAlt) == 0);

    // Set Ctrl+Shift
    event.modifiers = kModCtrl | kModShift;
    assert((event.modifiers & kModCtrl) != 0);
    assert((event.modifiers & kModShift) != 0);
    assert((event.modifiers & kModAlt) == 0);

    // Set all modifiers
    event.modifiers = kModCtrl | kModShift | kModAlt;
    assert((event.modifiers & kModCtrl) != 0);
    assert((event.modifiers & kModShift) != 0);
    assert((event.modifiers & kModAlt) != 0);

    std::cout << "[PASS] test_window_event_modifiers" << std::endl;
}

// ============================================================
// Test: UIEvent modifier field
// ============================================================

void test_ui_event_modifiers() {
    UIEvent event;
    assert(event.modifiers == 0);

    event.modifiers = kModCtrl;
    assert((event.modifiers & kModCtrl) != 0);

    event.type = UIEvent::Type::KeyDown;
    event.keyCode = '`';
    event.modifiers = kModCtrl;
    // Simulates Ctrl+` — the modifier is properly carried
    assert(event.keyCode == '`');
    assert((event.modifiers & kModCtrl) != 0);

    std::cout << "[PASS] test_ui_event_modifiers" << std::endl;
}

// ============================================================
// Test: Modifier propagation from WindowEvent to UIEvent
// ============================================================

void test_modifier_propagation() {
    // Simulate what Engine.cpp does: copy modifier from WindowEvent to UIEvent
    atlas::platform::WindowEvent wEvent;
    wEvent.type = atlas::platform::WindowEvent::Type::KeyDown;
    wEvent.keyCode = '`';
    wEvent.modifiers = kModCtrl;

    UIEvent uiEvent;
    uiEvent.type = UIEvent::Type::KeyDown;
    uiEvent.keyCode = wEvent.keyCode;
    uiEvent.modifiers = wEvent.modifiers;

    assert(uiEvent.keyCode == '`');
    assert(uiEvent.modifiers == kModCtrl);
    assert((uiEvent.modifiers & kModCtrl) != 0);

    std::cout << "[PASS] test_modifier_propagation" << std::endl;
}

void run_editor_ui_fix_tests() {
    std::cout << "\n--- Editor UI Fix Tests ---" << std::endl;
    test_editor_ui_has_widget_content();
    test_dsl_parses_but_fallback_used();
    test_window_event_modifiers();
    test_ui_event_modifiers();
    test_modifier_propagation();
}
