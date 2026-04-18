#include "../engine/ui/UIScreenGraph.h"
#include "../engine/ui/UIManager.h"
#include "../engine/ui/FocusManager.h"
#include "../engine/ui/TooltipManager.h"
#include "../engine/ui/DockManager.h"
#include "../engine/ui/InputFieldManager.h"
#include <cassert>
#include <iostream>

// ============================================================
// FocusManager Tests
// ============================================================

void test_focus_manager_init() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    atlas::ui::FocusManager mgr;
    mgr.Init(&screen);

    assert(mgr.GetFocusedWidgetId() == 0);
    std::cout << "[PASS] test_focus_manager_init" << std::endl;
}

void test_focus_manager_click_input_field() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t field = screen.AddWidget(atlas::ui::UIWidgetType::InputField, "Search", 10, 10, 200, 24);

    atlas::ui::FocusManager mgr;
    mgr.Init(&screen);

    // Click on the input field — should gain focus (InputField is focusable by default)
    bool changed = mgr.HandleClick(50, 18);
    assert(changed);
    assert(mgr.GetFocusedWidgetId() == field);

    std::cout << "[PASS] test_focus_manager_click_input_field" << std::endl;
}

void test_focus_manager_click_outside() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    screen.AddWidget(atlas::ui::UIWidgetType::InputField, "Search", 10, 10, 200, 24);

    atlas::ui::FocusManager mgr;
    mgr.Init(&screen);

    // Click on the input field
    mgr.HandleClick(50, 18);

    // Click outside any focusable widget — focus cleared
    bool changed = mgr.HandleClick(500, 500);
    assert(changed);
    assert(mgr.GetFocusedWidgetId() == 0);

    std::cout << "[PASS] test_focus_manager_click_outside" << std::endl;
}

void test_focus_manager_set_focusable() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t btn = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Submit", 10, 10, 80, 24);

    atlas::ui::FocusManager mgr;
    mgr.Init(&screen);

    // Buttons are not focusable by default
    assert(!mgr.IsFocusable(btn));
    mgr.HandleClick(20, 18);
    assert(mgr.GetFocusedWidgetId() == 0);

    // Mark as focusable
    mgr.SetFocusable(btn, true);
    assert(mgr.IsFocusable(btn));
    mgr.HandleClick(20, 18);
    assert(mgr.GetFocusedWidgetId() == btn);

    std::cout << "[PASS] test_focus_manager_set_focusable" << std::endl;
}

void test_focus_manager_clear_focus() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t field = screen.AddWidget(atlas::ui::UIWidgetType::InputField, "Name", 10, 10, 200, 24);

    atlas::ui::FocusManager mgr;
    mgr.Init(&screen);

    mgr.SetFocus(field);
    assert(mgr.GetFocusedWidgetId() == field);

    mgr.ClearFocus();
    assert(mgr.GetFocusedWidgetId() == 0);

    std::cout << "[PASS] test_focus_manager_clear_focus" << std::endl;
}

void test_focus_manager_tab_cycles() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t f1 = screen.AddWidget(atlas::ui::UIWidgetType::InputField, "Field1", 10, 10, 200, 24);
    uint32_t f2 = screen.AddWidget(atlas::ui::UIWidgetType::InputField, "Field2", 10, 40, 200, 24);

    atlas::ui::FocusManager mgr;
    mgr.Init(&screen);

    atlas::ui::UIEvent tabKey;
    tabKey.type = atlas::ui::UIEvent::Type::KeyDown;
    tabKey.keyCode = 0x09; // Tab

    // Tab with no focus -> focus first field
    bool consumed = mgr.HandleKeyEvent(tabKey);
    assert(consumed);
    assert(mgr.GetFocusedWidgetId() == f1);

    // Tab again -> focus second field
    consumed = mgr.HandleKeyEvent(tabKey);
    assert(consumed);
    assert(mgr.GetFocusedWidgetId() == f2);

    // Tab again -> wrap around to first
    consumed = mgr.HandleKeyEvent(tabKey);
    assert(consumed);
    assert(mgr.GetFocusedWidgetId() == f1);

    std::cout << "[PASS] test_focus_manager_tab_cycles" << std::endl;
}

void test_focus_manager_callback() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t field = screen.AddWidget(atlas::ui::UIWidgetType::InputField, "Name", 10, 10, 200, 24);

    atlas::ui::FocusManager mgr;
    mgr.Init(&screen);

    uint32_t cbNew = 0, cbOld = 0;
    mgr.SetFocusChangedCallback([&](uint32_t n, uint32_t o) {
        cbNew = n; cbOld = o;
    });

    mgr.SetFocus(field);
    assert(cbNew == field);
    assert(cbOld == 0);

    mgr.ClearFocus();
    assert(cbNew == 0);
    assert(cbOld == field);

    std::cout << "[PASS] test_focus_manager_callback" << std::endl;
}

// ============================================================
// TooltipManager Tests
// ============================================================

void test_tooltip_manager_init() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    atlas::ui::TooltipManager mgr;
    mgr.Init(&screen);

    assert(mgr.GetActiveTooltip() == 0);
    assert(mgr.GetDelay() == 0.5f);

    std::cout << "[PASS] test_tooltip_manager_init" << std::endl;
}

void test_tooltip_manager_set_tooltip() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t btn = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Save", 10, 10, 80, 24);
    uint32_t tip = screen.AddWidget(atlas::ui::UIWidgetType::Tooltip, "Save file", 10, 36, 100, 20);

    atlas::ui::TooltipManager mgr;
    mgr.Init(&screen);
    mgr.SetTooltip(btn, tip);

    assert(mgr.GetTooltip(btn) == tip);
    assert(!screen.IsVisible(tip)); // Tooltip starts hidden

    std::cout << "[PASS] test_tooltip_manager_set_tooltip" << std::endl;
}

void test_tooltip_manager_show_after_delay() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t btn = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Save", 10, 10, 80, 24);
    uint32_t tip = screen.AddWidget(atlas::ui::UIWidgetType::Tooltip, "Save file", 10, 36, 100, 20);

    atlas::ui::TooltipManager mgr;
    mgr.Init(&screen);
    mgr.SetTooltip(btn, tip);
    mgr.SetDelay(0.3f);

    // Hover over the button for less than delay — tooltip not shown
    mgr.Update(30, 18, 0.1f);
    assert(mgr.GetActiveTooltip() == 0);
    assert(!screen.IsVisible(tip));

    // Continue hovering — pass the delay
    mgr.Update(30, 18, 0.3f);
    assert(mgr.GetActiveTooltip() == tip);
    assert(screen.IsVisible(tip));

    std::cout << "[PASS] test_tooltip_manager_show_after_delay" << std::endl;
}

void test_tooltip_manager_hide_on_leave() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t btn = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Save", 10, 10, 80, 24);
    uint32_t tip = screen.AddWidget(atlas::ui::UIWidgetType::Tooltip, "Save file", 10, 36, 100, 20);

    atlas::ui::TooltipManager mgr;
    mgr.Init(&screen);
    mgr.SetTooltip(btn, tip);
    mgr.SetDelay(0.1f);

    // Show tooltip
    mgr.Update(30, 18, 0.2f);
    assert(mgr.GetActiveTooltip() == tip);

    // Move mouse away
    mgr.Update(500, 500, 0.1f);
    assert(mgr.GetActiveTooltip() == 0);
    assert(!screen.IsVisible(tip));

    std::cout << "[PASS] test_tooltip_manager_hide_on_leave" << std::endl;
}

void test_tooltip_manager_hide_all() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t btn = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Save", 10, 10, 80, 24);
    uint32_t tip = screen.AddWidget(atlas::ui::UIWidgetType::Tooltip, "Save file", 10, 36, 100, 20);

    atlas::ui::TooltipManager mgr;
    mgr.Init(&screen);
    mgr.SetTooltip(btn, tip);
    mgr.SetDelay(0.1f);

    mgr.Update(30, 18, 0.2f);
    assert(mgr.GetActiveTooltip() == tip);

    mgr.HideAll();
    assert(mgr.GetActiveTooltip() == 0);

    std::cout << "[PASS] test_tooltip_manager_hide_all" << std::endl;
}

void test_tooltip_manager_remove_tooltip() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t btn = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Save", 10, 10, 80, 24);
    uint32_t tip = screen.AddWidget(atlas::ui::UIWidgetType::Tooltip, "Save file", 10, 36, 100, 20);

    atlas::ui::TooltipManager mgr;
    mgr.Init(&screen);
    mgr.SetTooltip(btn, tip);
    assert(mgr.GetTooltip(btn) == tip);

    mgr.RemoveTooltip(btn);
    assert(mgr.GetTooltip(btn) == 0);

    std::cout << "[PASS] test_tooltip_manager_remove_tooltip" << std::endl;
}

// ============================================================
// DockManager Tests
// ============================================================

void test_dock_manager_init() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    atlas::ui::DockManager mgr;
    mgr.Init(&screen);

    assert(!mgr.IsDockArea(1));
    std::cout << "[PASS] test_dock_manager_init" << std::endl;
}

void test_dock_manager_register() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t dock = screen.AddWidget(atlas::ui::UIWidgetType::DockArea, "MainDock", 0, 60, 1280, 640);

    atlas::ui::DockManager mgr;
    mgr.Init(&screen);
    mgr.RegisterDockArea(dock);

    assert(mgr.IsDockArea(dock));
    assert(mgr.GetDockedPanels(dock).empty());

    std::cout << "[PASS] test_dock_manager_register" << std::endl;
}

void test_dock_manager_dock_panel_left() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t dock = screen.AddWidget(atlas::ui::UIWidgetType::DockArea, "Dock", 0, 0, 800, 600);
    uint32_t panel = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "Assets", 0, 0, 100, 100);

    atlas::ui::DockManager mgr;
    mgr.Init(&screen);
    mgr.RegisterDockArea(dock);

    bool ok = mgr.DockPanel(dock, panel, atlas::ui::DockSide::Left, 0.25f);
    assert(ok);
    assert(mgr.GetPanelDockArea(panel) == dock);

    const auto* w = screen.GetWidget(panel);
    assert(w != nullptr);
    assert(w->x == 0.0f);
    assert(w->width == 200.0f); // 800 * 0.25

    auto panels = mgr.GetDockedPanels(dock);
    assert(panels.size() == 1);
    assert(panels[0] == panel);

    std::cout << "[PASS] test_dock_manager_dock_panel_left" << std::endl;
}

void test_dock_manager_dock_panel_right() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t dock = screen.AddWidget(atlas::ui::UIWidgetType::DockArea, "Dock", 0, 0, 800, 600);
    uint32_t panel = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "Inspector", 0, 0, 100, 100);

    atlas::ui::DockManager mgr;
    mgr.Init(&screen);
    mgr.RegisterDockArea(dock);

    bool ok = mgr.DockPanel(dock, panel, atlas::ui::DockSide::Right, 0.3f);
    assert(ok);

    const auto* w = screen.GetWidget(panel);
    assert(w->x == 800.0f * 0.7f);
    assert(w->width == 800.0f * 0.3f);

    std::cout << "[PASS] test_dock_manager_dock_panel_right" << std::endl;
}

void test_dock_manager_dock_panel_center() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t dock = screen.AddWidget(atlas::ui::UIWidgetType::DockArea, "Dock", 10, 20, 400, 300);
    uint32_t panel = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "Viewport", 0, 0, 100, 100);

    atlas::ui::DockManager mgr;
    mgr.Init(&screen);
    mgr.RegisterDockArea(dock);

    mgr.DockPanel(dock, panel, atlas::ui::DockSide::Center);

    const auto* w = screen.GetWidget(panel);
    assert(w->x == 10.0f);
    assert(w->y == 20.0f);
    assert(w->width == 400.0f);
    assert(w->height == 300.0f);

    std::cout << "[PASS] test_dock_manager_dock_panel_center" << std::endl;
}

void test_dock_manager_undock_panel() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t dock = screen.AddWidget(atlas::ui::UIWidgetType::DockArea, "Dock", 0, 0, 800, 600);
    uint32_t panel = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "Assets", 0, 0, 100, 100);

    atlas::ui::DockManager mgr;
    mgr.Init(&screen);
    mgr.RegisterDockArea(dock);

    mgr.DockPanel(dock, panel, atlas::ui::DockSide::Left);
    assert(mgr.GetPanelDockArea(panel) == dock);

    bool ok = mgr.UndockPanel(panel);
    assert(ok);
    assert(mgr.GetPanelDockArea(panel) == 0);
    assert(mgr.GetDockedPanels(dock).empty());

    std::cout << "[PASS] test_dock_manager_undock_panel" << std::endl;
}

void test_dock_manager_callback() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t dock = screen.AddWidget(atlas::ui::UIWidgetType::DockArea, "Dock", 0, 0, 800, 600);
    uint32_t panel = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "Panel", 0, 0, 100, 100);

    atlas::ui::DockManager mgr;
    mgr.Init(&screen);
    mgr.RegisterDockArea(dock);

    uint32_t cbDock = 0, cbPanel = 0;
    atlas::ui::DockSide cbSide = atlas::ui::DockSide::Center;
    mgr.SetDockCallback([&](uint32_t d, uint32_t p, atlas::ui::DockSide s) {
        cbDock = d; cbPanel = p; cbSide = s;
    });

    mgr.DockPanel(dock, panel, atlas::ui::DockSide::Bottom, 0.3f);
    assert(cbDock == dock);
    assert(cbPanel == panel);
    assert(cbSide == atlas::ui::DockSide::Bottom);

    std::cout << "[PASS] test_dock_manager_callback" << std::endl;
}

void test_dock_manager_split_ratio() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t dock = screen.AddWidget(atlas::ui::UIWidgetType::DockArea, "Dock", 0, 0, 800, 600);

    atlas::ui::DockManager mgr;
    mgr.Init(&screen);
    mgr.RegisterDockArea(dock);

    assert(mgr.GetSplitRatio(dock) == 0.5f);
    mgr.SetSplitRatio(dock, 0.7f);
    assert(mgr.GetSplitRatio(dock) == 0.7f);

    std::cout << "[PASS] test_dock_manager_split_ratio" << std::endl;
}

// ============================================================
// InputFieldManager Tests
// ============================================================

void test_input_field_manager_init() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    atlas::ui::InputFieldManager mgr;
    mgr.Init(&screen);

    assert(!mgr.IsRegistered(1));
    std::cout << "[PASS] test_input_field_manager_init" << std::endl;
}

void test_input_field_manager_register() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t field = screen.AddWidget(atlas::ui::UIWidgetType::InputField, "command...", 10, 10, 200, 24);

    atlas::ui::InputFieldManager mgr;
    mgr.Init(&screen);
    mgr.RegisterField(field, "command...");

    assert(mgr.IsRegistered(field));
    assert(mgr.GetPlaceholder(field) == "command...");

    std::cout << "[PASS] test_input_field_manager_register" << std::endl;
}

void test_input_field_manager_type_text() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t field = screen.AddWidget(atlas::ui::UIWidgetType::InputField, "", 10, 10, 200, 24);

    atlas::ui::InputFieldManager mgr;
    mgr.Init(&screen);
    mgr.RegisterField(field);

    // Type 'H'
    atlas::ui::UIEvent e;
    e.type = atlas::ui::UIEvent::Type::TextInput;
    e.textChar = 'H';
    bool consumed = mgr.HandleEvent(e, field);
    assert(consumed);
    assert(mgr.GetText(field) == "H");
    assert(mgr.GetCursorPos(field) == 1);

    // Type 'i'
    e.textChar = 'i';
    mgr.HandleEvent(e, field);
    assert(mgr.GetText(field) == "Hi");
    assert(mgr.GetCursorPos(field) == 2);

    // Widget name updated
    const auto* w = screen.GetWidget(field);
    assert(w->name == "Hi");

    std::cout << "[PASS] test_input_field_manager_type_text" << std::endl;
}

void test_input_field_manager_backspace() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t field = screen.AddWidget(atlas::ui::UIWidgetType::InputField, "", 10, 10, 200, 24);

    atlas::ui::InputFieldManager mgr;
    mgr.Init(&screen);
    mgr.RegisterField(field);
    mgr.SetText(field, "Hello");

    // Backspace
    atlas::ui::UIEvent e;
    e.type = atlas::ui::UIEvent::Type::KeyDown;
    e.keyCode = 0x08; // Backspace
    mgr.HandleEvent(e, field);

    assert(mgr.GetText(field) == "Hell");
    assert(mgr.GetCursorPos(field) == 4);

    std::cout << "[PASS] test_input_field_manager_backspace" << std::endl;
}

void test_input_field_manager_delete_key() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t field = screen.AddWidget(atlas::ui::UIWidgetType::InputField, "", 10, 10, 200, 24);

    atlas::ui::InputFieldManager mgr;
    mgr.Init(&screen);
    mgr.RegisterField(field);
    mgr.SetText(field, "Hello");
    mgr.SetCursorPos(field, 0);

    // Delete
    atlas::ui::UIEvent e;
    e.type = atlas::ui::UIEvent::Type::KeyDown;
    e.keyCode = 0x2E; // Delete
    mgr.HandleEvent(e, field);

    assert(mgr.GetText(field) == "ello");
    assert(mgr.GetCursorPos(field) == 0);

    std::cout << "[PASS] test_input_field_manager_delete_key" << std::endl;
}

void test_input_field_manager_arrow_keys() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t field = screen.AddWidget(atlas::ui::UIWidgetType::InputField, "", 10, 10, 200, 24);

    atlas::ui::InputFieldManager mgr;
    mgr.Init(&screen);
    mgr.RegisterField(field);
    mgr.SetText(field, "ABC");

    // Cursor at end (3), left arrow
    atlas::ui::UIEvent e;
    e.type = atlas::ui::UIEvent::Type::KeyDown;
    e.keyCode = 0x25; // Left
    mgr.HandleEvent(e, field);
    assert(mgr.GetCursorPos(field) == 2);

    // Right arrow
    e.keyCode = 0x27; // Right
    mgr.HandleEvent(e, field);
    assert(mgr.GetCursorPos(field) == 3);

    // Home
    e.keyCode = 0x24; // Home
    mgr.HandleEvent(e, field);
    assert(mgr.GetCursorPos(field) == 0);

    // End
    e.keyCode = 0x23; // End
    mgr.HandleEvent(e, field);
    assert(mgr.GetCursorPos(field) == 3);

    std::cout << "[PASS] test_input_field_manager_arrow_keys" << std::endl;
}

void test_input_field_manager_enter_submits() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t field = screen.AddWidget(atlas::ui::UIWidgetType::InputField, "", 10, 10, 200, 24);

    atlas::ui::InputFieldManager mgr;
    mgr.Init(&screen);
    mgr.RegisterField(field);
    mgr.SetText(field, "run command");

    uint32_t submitId = 0;
    std::string submitText;
    mgr.SetTextSubmitCallback([&](uint32_t id, const std::string& text) {
        submitId = id; submitText = text;
    });

    atlas::ui::UIEvent e;
    e.type = atlas::ui::UIEvent::Type::KeyDown;
    e.keyCode = 0x0D; // Enter
    mgr.HandleEvent(e, field);

    assert(submitId == field);
    assert(submitText == "run command");
    // Field should be cleared after submit
    assert(mgr.GetText(field).empty());
    assert(mgr.GetCursorPos(field) == 0);
    const atlas::ui::UIWidget* w = screen.GetWidget(field);
    assert(w && w->name.empty());

    std::cout << "[PASS] test_input_field_manager_enter_submits" << std::endl;
}

void test_input_field_manager_text_changed_callback() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t field = screen.AddWidget(atlas::ui::UIWidgetType::InputField, "", 10, 10, 200, 24);

    atlas::ui::InputFieldManager mgr;
    mgr.Init(&screen);
    mgr.RegisterField(field);

    uint32_t changedId = 0;
    std::string changedText;
    mgr.SetTextChangedCallback([&](uint32_t id, const std::string& text) {
        changedId = id; changedText = text;
    });

    atlas::ui::UIEvent e;
    e.type = atlas::ui::UIEvent::Type::TextInput;
    e.textChar = 'X';
    mgr.HandleEvent(e, field);

    assert(changedId == field);
    assert(changedText == "X");

    std::cout << "[PASS] test_input_field_manager_text_changed_callback" << std::endl;
}

void test_input_field_manager_set_text() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    uint32_t field = screen.AddWidget(atlas::ui::UIWidgetType::InputField, "", 10, 10, 200, 24);

    atlas::ui::InputFieldManager mgr;
    mgr.Init(&screen);
    mgr.RegisterField(field);

    mgr.SetText(field, "Hello World");
    assert(mgr.GetText(field) == "Hello World");
    assert(mgr.GetCursorPos(field) == 11); // cursor moves to end of new text

    // Widget name also updated
    const auto* w = screen.GetWidget(field);
    assert(w->name == "Hello World");

    std::cout << "[PASS] test_input_field_manager_set_text" << std::endl;
}

void test_input_field_manager_not_registered() {
    atlas::ui::UIScreen screen;
    screen.Init("TestScreen");

    atlas::ui::InputFieldManager mgr;
    mgr.Init(&screen);

    // Trying to handle event on unregistered field should not crash
    atlas::ui::UIEvent e;
    e.type = atlas::ui::UIEvent::Type::TextInput;
    e.textChar = 'A';
    bool consumed = mgr.HandleEvent(e, 99);
    assert(!consumed);

    assert(mgr.GetText(99).empty());

    std::cout << "[PASS] test_input_field_manager_not_registered" << std::endl;
}

// ============================================================
// UIManager Integration Tests
// ============================================================

void test_ui_manager_focus_integration() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    uint32_t field = screen.AddWidget(atlas::ui::UIWidgetType::InputField, "", 10, 10, 200, 24);
    mgr.GetInputFieldManager().RegisterField(field);

    // Click on input field to focus it
    atlas::ui::UIEvent click;
    click.type = atlas::ui::UIEvent::Type::MouseDown;
    click.mouseButton = 0;
    click.x = 50;
    click.y = 18;
    mgr.DispatchEvent(click);

    assert(mgr.GetFocusManager().GetFocusedWidgetId() == field);

    // Type a character — should go to the focused input field
    atlas::ui::UIEvent textEvt;
    textEvt.type = atlas::ui::UIEvent::Type::TextInput;
    textEvt.textChar = 'A';
    bool consumed = mgr.DispatchEvent(textEvt);
    assert(consumed);
    assert(mgr.GetInputFieldManager().GetText(field) == "A");

    std::cout << "[PASS] test_ui_manager_focus_integration" << std::endl;
}

void test_ui_manager_dock_integration() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    uint32_t dock = screen.AddWidget(atlas::ui::UIWidgetType::DockArea, "Dock", 0, 0, 800, 600);
    uint32_t panel = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "Assets", 0, 0, 100, 100);

    mgr.GetDockManager().RegisterDockArea(dock);
    mgr.GetDockManager().DockPanel(dock, panel, atlas::ui::DockSide::Left, 0.25f);

    auto panels = mgr.GetDockManager().GetDockedPanels(dock);
    assert(panels.size() == 1);
    assert(panels[0] == panel);

    // Render with null renderer — should not crash
    atlas::ui::NullUIRenderer renderer;
    mgr.Render(&renderer);

    std::cout << "[PASS] test_ui_manager_dock_integration" << std::endl;
}

void test_ui_manager_tooltip_integration() {
    atlas::ui::UIManager mgr;
    mgr.Init(atlas::ui::GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    uint32_t btn = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Save", 10, 10, 80, 24);
    uint32_t tip = screen.AddWidget(atlas::ui::UIWidgetType::Tooltip, "Save file", 10, 36, 100, 20);

    mgr.GetTooltipManager().SetTooltip(btn, tip);
    mgr.GetTooltipManager().SetDelay(0.1f);

    // Simulate hover
    mgr.GetTooltipManager().Update(30, 18, 0.2f);
    assert(mgr.GetTooltipManager().GetActiveTooltip() == tip);
    assert(screen.IsVisible(tip));

    std::cout << "[PASS] test_ui_manager_tooltip_integration" << std::endl;
}
