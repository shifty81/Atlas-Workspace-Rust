#include "../engine/ui/UIScreenGraph.h"
#include "../engine/ui/UIManager.h"
#include "../engine/ui/CheckboxManager.h"
#include "../engine/ui/TreeNodeManager.h"
#include "../engine/ui/SplitterManager.h"
#include "../engine/ui/ColorPickerManager.h"
#include <cassert>
#include <iostream>
#include <cmath>

// ============================================================
// CheckboxManager Tests
// ============================================================

void test_checkbox_manager_init() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    atlas::ui::CheckboxManager mgr;
    mgr.Init(&screen);
    // Init should not crash
    std::cout << "[PASS] test_checkbox_manager_init" << std::endl;
}

void test_checkbox_manager_click_toggles() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t cb = screen.AddWidget(atlas::ui::UIWidgetType::Checkbox, "Grid", 10, 10, 150, 20);
    assert(!screen.IsChecked(cb));

    atlas::ui::CheckboxManager mgr;
    mgr.Init(&screen);

    bool callbackFired = false;
    bool callbackValue = false;
    mgr.SetCheckboxChangedCallback([&](uint32_t, bool checked) {
        callbackFired = true;
        callbackValue = checked;
    });

    // Click on the checkbox
    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.mouseButton = 0;
    click.x = 50;
    click.y = 18;
    bool consumed = mgr.HandleEvent(click);
    assert(consumed);
    assert(screen.IsChecked(cb));
    assert(callbackFired);
    assert(callbackValue == true);

    // Click again to uncheck
    callbackFired = false;
    consumed = mgr.HandleEvent(click);
    assert(consumed);
    assert(!screen.IsChecked(cb));
    assert(callbackFired);
    assert(callbackValue == false);

    std::cout << "[PASS] test_checkbox_manager_click_toggles" << std::endl;
}

void test_checkbox_manager_click_outside_ignored() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    screen.AddWidget(atlas::ui::UIWidgetType::Checkbox, "Grid", 10, 10, 150, 20);

    atlas::ui::CheckboxManager mgr;
    mgr.Init(&screen);

    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.mouseButton = 0;
    click.x = 500;
    click.y = 500;
    bool consumed = mgr.HandleEvent(click);
    assert(!consumed);

    std::cout << "[PASS] test_checkbox_manager_click_outside_ignored" << std::endl;
}

void test_checkbox_manager_ignores_non_click() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    screen.AddWidget(atlas::ui::UIWidgetType::Checkbox, "Grid", 10, 10, 150, 20);

    atlas::ui::CheckboxManager mgr;
    mgr.Init(&screen);

    atlas::ui::UIEvent move;
    move.type = atlas::ui::UIEvent::Type::MouseMove;
    move.x = 50;
    move.y = 18;
    bool consumed = mgr.HandleEvent(move);
    assert(!consumed);

    std::cout << "[PASS] test_checkbox_manager_ignores_non_click" << std::endl;
}

// ============================================================
// TreeNodeManager Tests
// ============================================================

void test_treenode_manager_init() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    atlas::ui::TreeNodeManager mgr;
    mgr.Init(&screen);
    std::cout << "[PASS] test_treenode_manager_init" << std::endl;
}

void test_treenode_manager_click_toggles_expand() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t tn = screen.AddWidget(atlas::ui::UIWidgetType::TreeNode, "World", 10, 10, 200, 20);
    assert(!screen.IsExpanded(tn));

    atlas::ui::TreeNodeManager mgr;
    mgr.Init(&screen);

    bool callbackFired = false;
    bool callbackExpanded = false;
    mgr.SetTreeNodeToggledCallback([&](uint32_t, bool expanded) {
        callbackFired = true;
        callbackExpanded = expanded;
    });

    // Click to expand
    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.mouseButton = 0;
    click.x = 50;
    click.y = 18;
    bool consumed = mgr.HandleEvent(click);
    assert(consumed);
    assert(screen.IsExpanded(tn));
    assert(callbackFired);
    assert(callbackExpanded == true);

    // Click again to collapse
    callbackFired = false;
    consumed = mgr.HandleEvent(click);
    assert(consumed);
    assert(!screen.IsExpanded(tn));
    assert(callbackFired);
    assert(callbackExpanded == false);

    std::cout << "[PASS] test_treenode_manager_click_toggles_expand" << std::endl;
}

void test_treenode_manager_click_outside_ignored() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    screen.AddWidget(atlas::ui::UIWidgetType::TreeNode, "World", 10, 10, 200, 20);

    atlas::ui::TreeNodeManager mgr;
    mgr.Init(&screen);

    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.mouseButton = 0;
    click.x = 500;
    click.y = 500;
    bool consumed = mgr.HandleEvent(click);
    assert(!consumed);

    std::cout << "[PASS] test_treenode_manager_click_outside_ignored" << std::endl;
}

// ============================================================
// SplitterManager Tests
// ============================================================

void test_splitter_manager_init() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    atlas::ui::SplitterManager mgr;
    mgr.Init(&screen);
    assert(!mgr.IsDragging());
    assert(mgr.GetDraggingId() == 0);
    std::cout << "[PASS] test_splitter_manager_init" << std::endl;
}

void test_splitter_manager_drag_moves() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    // Vertical splitter (width < height)
    uint32_t sp = screen.AddWidget(atlas::ui::UIWidgetType::Splitter, "VSplit", 260, 30, 4, 690);

    atlas::ui::SplitterManager mgr;
    mgr.Init(&screen);

    bool callbackFired = false;
    float callbackPos = 0.0f;
    mgr.SetSplitterMovedCallback([&](uint32_t, float pos) {
        callbackFired = true;
        callbackPos = pos;
    });

    // Click on the splitter
    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.mouseButton = 0;
    click.x = 261;
    click.y = 100;
    bool consumed = mgr.HandleEvent(click);
    assert(consumed);
    assert(mgr.IsDragging());
    assert(mgr.GetDraggingId() == sp);

    // Drag to new position
    atlas::ui::UIEvent drag;
    drag.type = atlas::ui::UIEvent::Type::MouseMove;
    drag.x = 300;
    drag.y = 100;
    consumed = mgr.HandleEvent(drag);
    assert(consumed);
    assert(callbackFired);
    assert(std::fabs(callbackPos - 300.0f) < 0.01f);

    // Check that splitter position was updated
    const auto* w = screen.GetWidget(sp);
    assert(std::fabs(w->x - 300.0f) < 0.01f);

    // Release
    atlas::ui::UIEvent release;
    release.type = atlas::ui::UIEvent::Type::MouseUp;
    release.x = 300;
    release.y = 100;
    mgr.HandleEvent(release);
    assert(!mgr.IsDragging());

    std::cout << "[PASS] test_splitter_manager_drag_moves" << std::endl;
}

void test_splitter_manager_click_outside_ignored() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    screen.AddWidget(atlas::ui::UIWidgetType::Splitter, "VSplit", 260, 30, 4, 690);

    atlas::ui::SplitterManager mgr;
    mgr.Init(&screen);

    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.mouseButton = 0;
    click.x = 500;
    click.y = 500;
    bool consumed = mgr.HandleEvent(click);
    assert(!consumed);
    assert(!mgr.IsDragging());

    std::cout << "[PASS] test_splitter_manager_click_outside_ignored" << std::endl;
}

void test_splitter_manager_horizontal_drag() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    // Horizontal splitter (width >= height)
    uint32_t sp = screen.AddWidget(atlas::ui::UIWidgetType::Splitter, "HSplit", 0, 400, 800, 4);

    atlas::ui::SplitterManager mgr;
    mgr.Init(&screen);

    // Click on the splitter
    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.mouseButton = 0;
    click.x = 200;
    click.y = 401;
    bool consumed = mgr.HandleEvent(click);
    assert(consumed);
    assert(mgr.IsDragging());

    // Drag vertically
    atlas::ui::UIEvent drag;
    drag.type = atlas::ui::UIEvent::Type::MouseMove;
    drag.x = 200;
    drag.y = 350;
    consumed = mgr.HandleEvent(drag);
    assert(consumed);

    // Check that y was updated (horizontal splitter moves vertically)
    const auto* w = screen.GetWidget(sp);
    assert(std::fabs(w->y - 350.0f) < 0.01f);

    // Release
    atlas::ui::UIEvent release;
    release.type = atlas::ui::UIEvent::Type::MouseUp;
    release.x = 200;
    release.y = 350;
    mgr.HandleEvent(release);
    assert(!mgr.IsDragging());

    std::cout << "[PASS] test_splitter_manager_horizontal_drag" << std::endl;
}

// ============================================================
// ColorPickerManager Tests
// ============================================================

void test_colorpicker_manager_init() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    atlas::ui::ColorPickerManager mgr;
    mgr.Init(&screen);
    assert(!mgr.IsOpen());
    assert(mgr.GetOpenPickerId() == 0);
    std::cout << "[PASS] test_colorpicker_manager_init" << std::endl;
}

void test_colorpicker_manager_click_opens() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t cp = screen.AddWidget(atlas::ui::UIWidgetType::ColorPicker, "Tint", 10, 10, 24, 24);

    atlas::ui::ColorPickerManager mgr;
    mgr.Init(&screen);

    // Click on the swatch
    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.mouseButton = 0;
    click.x = 20;
    click.y = 20;
    bool consumed = mgr.HandleEvent(click);
    assert(consumed);
    assert(mgr.IsOpen());
    assert(mgr.GetOpenPickerId() == cp);

    std::cout << "[PASS] test_colorpicker_manager_click_opens" << std::endl;
}

void test_colorpicker_manager_click_outside_closes() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    screen.AddWidget(atlas::ui::UIWidgetType::ColorPicker, "Tint", 10, 10, 24, 24);

    atlas::ui::ColorPickerManager mgr;
    mgr.Init(&screen);

    // Open
    atlas::ui::UIEvent open;
    open.type = atlas::ui::UIEvent::Type::MouseDown;
    open.mouseButton = 0;
    open.x = 20;
    open.y = 20;
    mgr.HandleEvent(open);
    assert(mgr.IsOpen());

    // Click outside
    atlas::ui::UIEvent outside;
    outside.type = atlas::ui::UIEvent::Type::MouseDown;
    outside.mouseButton = 0;
    outside.x = 500;
    outside.y = 500;
    mgr.HandleEvent(outside);
    assert(!mgr.IsOpen());

    std::cout << "[PASS] test_colorpicker_manager_click_outside_closes" << std::endl;
}

void test_colorpicker_manager_select_color() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t cp = screen.AddWidget(atlas::ui::UIWidgetType::ColorPicker, "Tint", 10, 10, 24, 24);

    atlas::ui::ColorPickerManager mgr;
    mgr.Init(&screen);

    bool callbackFired = false;
    mgr.SetColorChangedCallback([&](uint32_t, uint8_t, uint8_t, uint8_t, uint8_t) {
        callbackFired = true;
    });

    // Open picker
    atlas::ui::UIEvent open;
    open.type = atlas::ui::UIEvent::Type::MouseDown;
    open.mouseButton = 0;
    open.x = 20;
    open.y = 20;
    mgr.HandleEvent(open);
    assert(mgr.IsOpen());

    // Click in popup area (below swatch: x=10..110, y=34..114)
    atlas::ui::UIEvent selectColor;
    selectColor.type = atlas::ui::UIEvent::Type::MouseDown;
    selectColor.mouseButton = 0;
    selectColor.x = 60;  // middle of popup (normX ~= 0.5)
    selectColor.y = 54;  // near top of popup (normY ~= 0.25)
    mgr.HandleEvent(selectColor);
    assert(callbackFired);
    assert(!mgr.IsOpen());

    // Check that color was set on the widget
    uint8_t r, g, b, a;
    screen.GetColor(cp, r, g, b, a);
    assert(r > 0);   // Should be non-zero based on normX
    assert(a == 255); // Alpha should be 255

    std::cout << "[PASS] test_colorpicker_manager_select_color" << std::endl;
}

void test_colorpicker_manager_set_color_fires_callback() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");
    uint32_t cp = screen.AddWidget(atlas::ui::UIWidgetType::ColorPicker, "Tint", 10, 10, 24, 24);

    atlas::ui::ColorPickerManager mgr;
    mgr.Init(&screen);

    bool callbackFired = false;
    uint8_t cbR = 0, cbG = 0, cbB = 0, cbA = 0;
    mgr.SetColorChangedCallback([&](uint32_t, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        callbackFired = true;
        cbR = r; cbG = g; cbB = b; cbA = a;
    });

    mgr.SetColor(cp, 100, 200, 50, 255);
    assert(callbackFired);
    assert(cbR == 100 && cbG == 200 && cbB == 50 && cbA == 255);

    uint8_t r, g, b, a;
    screen.GetColor(cp, r, g, b, a);
    assert(r == 100 && g == 200 && b == 50 && a == 255);

    std::cout << "[PASS] test_colorpicker_manager_set_color_fires_callback" << std::endl;
}

// ============================================================
// UIManager Integration Tests
// ============================================================

void test_ui_manager_checkbox_integration() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    uint32_t cb = screen.AddWidget(atlas::ui::UIWidgetType::Checkbox, "Grid", 10, 10, 150, 20);
    assert(!screen.IsChecked(cb));

    bool callbackFired = false;
    mgr.GetCheckboxManager().SetCheckboxChangedCallback([&](uint32_t, bool) {
        callbackFired = true;
    });

    // Click on checkbox through UIManager dispatch
    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.mouseButton = 0;
    click.x = 50;
    click.y = 18;
    bool consumed = mgr.DispatchEvent(click);
    assert(consumed);
    assert(callbackFired);
    assert(screen.IsChecked(cb));

    std::cout << "[PASS] test_ui_manager_checkbox_integration" << std::endl;
}

void test_ui_manager_treenode_integration() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    uint32_t tn = screen.AddWidget(atlas::ui::UIWidgetType::TreeNode, "World", 10, 10, 200, 20);
    assert(!screen.IsExpanded(tn));

    bool callbackFired = false;
    mgr.GetTreeNodeManager().SetTreeNodeToggledCallback([&](uint32_t, bool) {
        callbackFired = true;
    });

    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.mouseButton = 0;
    click.x = 50;
    click.y = 18;
    bool consumed = mgr.DispatchEvent(click);
    assert(consumed);
    assert(callbackFired);
    assert(screen.IsExpanded(tn));

    std::cout << "[PASS] test_ui_manager_treenode_integration" << std::endl;
}

void test_ui_manager_splitter_integration() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    uint32_t sp = screen.AddWidget(atlas::ui::UIWidgetType::Splitter, "Split", 260, 30, 4, 690);

    bool callbackFired = false;
    mgr.GetSplitterManager().SetSplitterMovedCallback([&](uint32_t, float) {
        callbackFired = true;
    });

    // Click on splitter
    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.mouseButton = 0;
    click.x = 261;
    click.y = 100;
    bool consumed = mgr.DispatchEvent(click);
    assert(consumed);
    assert(mgr.GetSplitterManager().IsDragging());

    // Drag
    atlas::ui::UIEvent drag;
    drag.type = atlas::ui::UIEvent::Type::MouseMove;
    drag.x = 300;
    drag.y = 100;
    consumed = mgr.DispatchEvent(drag);
    assert(consumed);
    assert(callbackFired);

    // Check position updated
    const auto* w = screen.GetWidget(sp);
    assert(std::fabs(w->x - 300.0f) < 0.01f);

    // Release
    atlas::ui::UIEvent release;
    release.type = atlas::ui::UIEvent::Type::MouseUp;
    release.x = 300;
    release.y = 100;
    mgr.DispatchEvent(release);
    assert(!mgr.GetSplitterManager().IsDragging());

    std::cout << "[PASS] test_ui_manager_splitter_integration" << std::endl;
}

void test_ui_manager_colorpicker_integration() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    screen.AddWidget(atlas::ui::UIWidgetType::ColorPicker, "Tint", 10, 10, 24, 24);

    // Click on swatch to open picker
    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.mouseButton = 0;
    click.x = 20;
    click.y = 20;
    bool consumed = mgr.DispatchEvent(click);
    assert(consumed);
    assert(mgr.GetColorPickerManager().IsOpen());

    std::cout << "[PASS] test_ui_manager_colorpicker_integration" << std::endl;
}
