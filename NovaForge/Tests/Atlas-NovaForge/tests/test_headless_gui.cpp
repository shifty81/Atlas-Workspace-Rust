#include "../engine/ui/HeadlessGUI.h"
#include "../engine/ui/DiagnosticsOverlay.h"
#include <iostream>
#include <cassert>
#include <string>

using namespace atlas::ui;

void test_headless_gui_init() {
    UIManager manager;
    manager.Init(GUIContext::Server);

    HeadlessGUI gui;
    gui.Init(&manager);

    assert(gui.IsInitialized());
    assert(gui.CommandCount() == 10);

    gui.Shutdown();
    assert(!gui.IsInitialized());

    manager.Shutdown();
    std::cout << "[PASS] test_headless_gui_init" << std::endl;
}

void test_headless_gui_available_commands() {
    UIManager manager;
    manager.Init(GUIContext::Server);

    HeadlessGUI gui;
    gui.Init(&manager);

    auto commands = gui.AvailableCommands();
    assert(commands.size() == 10);

    // Check that all built-in commands are present
    bool hasHelp = false, hasStatus = false, hasWidgetList = false;
    bool hasWidgetInfo = false, hasWidgetCount = false, hasEnqueue = false;
    for (const auto& c : commands) {
        if (c == "help") hasHelp = true;
        if (c == "status") hasStatus = true;
        if (c == "widget.list") hasWidgetList = true;
        if (c == "widget.info") hasWidgetInfo = true;
        if (c == "widget.count") hasWidgetCount = true;
        if (c == "enqueue") hasEnqueue = true;
    }
    assert(hasHelp && hasStatus && hasWidgetList && hasWidgetInfo && hasWidgetCount && hasEnqueue);

    gui.Shutdown();
    manager.Shutdown();
    std::cout << "[PASS] test_headless_gui_available_commands" << std::endl;
}

void test_headless_gui_widget_count() {
    UIManager manager;
    manager.Init(GUIContext::Server);

    // Add some widgets
    manager.GetScreen().Init("test");
    manager.GetScreen().AddWidget(UIWidgetType::Panel, "Root", 0, 0, 100, 100);
    manager.GetScreen().AddWidget(UIWidgetType::Button, "Btn", 10, 10, 50, 30);

    HeadlessGUI gui;
    gui.Init(&manager);

    auto result = gui.ExecuteCommand("widget.count");
    assert(result.success);
    assert(result.output == "2");

    gui.Shutdown();
    manager.Shutdown();
    std::cout << "[PASS] test_headless_gui_widget_count" << std::endl;
}

void test_headless_gui_widget_info() {
    UIManager manager;
    manager.Init(GUIContext::Server);

    manager.GetScreen().Init("test");
    uint32_t id = manager.GetScreen().AddWidget(UIWidgetType::Button, "FireBtn", 10, 20, 80, 40);

    HeadlessGUI gui;
    gui.Init(&manager);

    auto result = gui.ExecuteCommand("widget.info " + std::to_string(id));
    assert(result.success);
    assert(result.output.find("FireBtn") != std::string::npos);
    assert(result.output.find("Button") != std::string::npos);

    gui.Shutdown();
    manager.Shutdown();
    std::cout << "[PASS] test_headless_gui_widget_info" << std::endl;
}

void test_headless_gui_widget_info_not_found() {
    UIManager manager;
    manager.Init(GUIContext::Server);

    HeadlessGUI gui;
    gui.Init(&manager);

    auto result = gui.ExecuteCommand("widget.info 9999");
    assert(!result.success);
    assert(result.output.find("not found") != std::string::npos);

    gui.Shutdown();
    manager.Shutdown();
    std::cout << "[PASS] test_headless_gui_widget_info_not_found" << std::endl;
}

void test_headless_gui_unknown_command() {
    UIManager manager;
    manager.Init(GUIContext::Server);

    HeadlessGUI gui;
    gui.Init(&manager);

    auto result = gui.ExecuteCommand("nonexistent.command");
    assert(!result.success);
    assert(result.output.find("Unknown") != std::string::npos);

    gui.Shutdown();
    manager.Shutdown();
    std::cout << "[PASS] test_headless_gui_unknown_command" << std::endl;
}

void test_headless_gui_empty_command() {
    UIManager manager;
    manager.Init(GUIContext::Server);

    HeadlessGUI gui;
    gui.Init(&manager);

    auto result = gui.ExecuteCommand("");
    assert(!result.success);

    gui.Shutdown();
    manager.Shutdown();
    std::cout << "[PASS] test_headless_gui_empty_command" << std::endl;
}

void test_headless_gui_help() {
    UIManager manager;
    manager.Init(GUIContext::Server);

    HeadlessGUI gui;
    gui.Init(&manager);

    auto result = gui.ExecuteCommand("help");
    assert(result.success);
    assert(result.output.find("widget.list") != std::string::npos);
    assert(result.output.find("status") != std::string::npos);

    gui.Shutdown();
    manager.Shutdown();
    std::cout << "[PASS] test_headless_gui_help" << std::endl;
}

void test_headless_gui_status() {
    UIManager manager;
    manager.Init(GUIContext::Server);

    HeadlessGUI gui;
    gui.Init(&manager);

    auto result = gui.ExecuteCommand("status");
    assert(result.success);
    assert(result.output.find("Server") != std::string::npos);
    assert(result.output.find("widgets") != std::string::npos);

    gui.Shutdown();
    manager.Shutdown();
    std::cout << "[PASS] test_headless_gui_status" << std::endl;
}

void test_headless_gui_enqueue() {
    UIManager manager;
    manager.Init(GUIContext::Server);

    manager.GetScreen().Init("test");
    uint32_t id = manager.GetScreen().AddWidget(UIWidgetType::Button, "Btn", 0, 0, 50, 30);

    HeadlessGUI gui;
    gui.Init(&manager);

    auto result = gui.ExecuteCommand("enqueue ButtonPress " + std::to_string(id));
    assert(result.success);
    assert(manager.GetCommandBus().PendingCount() == 1);

    gui.Shutdown();
    manager.Shutdown();
    std::cout << "[PASS] test_headless_gui_enqueue" << std::endl;
}

void test_headless_gui_custom_command() {
    UIManager manager;
    manager.Init(GUIContext::Server);

    HeadlessGUI gui;
    gui.Init(&manager);

    gui.RegisterCommand("custom.ping", [](const std::vector<std::string>&) -> GUIQueryResult {
        return {true, "pong"};
    });

    assert(gui.CommandCount() == 11);

    auto result = gui.ExecuteCommand("custom.ping");
    assert(result.success);
    assert(result.output == "pong");

    gui.Shutdown();
    manager.Shutdown();
    std::cout << "[PASS] test_headless_gui_custom_command" << std::endl;
}

void test_headless_gui_quoted_args() {
    UIManager manager;
    manager.Init(GUIContext::Server);

    HeadlessGUI gui;
    gui.Init(&manager);

    gui.RegisterCommand("echo", [](const std::vector<std::string>& args) -> GUIQueryResult {
        std::string out;
        for (const auto& a : args) out += a + " ";
        return {true, out};
    });

    auto result = gui.ExecuteCommand(R"(echo "hello world" test)");
    assert(result.success);
    assert(result.output.find("hello world") != std::string::npos);
    assert(result.output.find("test") != std::string::npos);

    gui.Shutdown();
    manager.Shutdown();
    std::cout << "[PASS] test_headless_gui_quoted_args" << std::endl;
}

void test_headless_gui_diag_toggle() {
    UIManager manager;
    manager.Init(GUIContext::Server);

    HeadlessGUI gui;
    gui.Init(&manager);

    atlas::ui::DiagnosticsOverlay::SetEnabled(false);

    auto result = gui.ExecuteCommand("diag.toggle");
    assert(result.success);
    assert(result.output == "on");
    assert(atlas::ui::DiagnosticsOverlay::IsEnabled());

    result = gui.ExecuteCommand("diag.toggle");
    assert(result.success);
    assert(result.output == "off");
    assert(!atlas::ui::DiagnosticsOverlay::IsEnabled());

    gui.Shutdown();
    manager.Shutdown();
    std::cout << "[PASS] test_headless_gui_diag_toggle" << std::endl;
}

void test_headless_gui_diag_show_hide() {
    UIManager manager;
    manager.Init(GUIContext::Server);

    HeadlessGUI gui;
    gui.Init(&manager);

    atlas::ui::DiagnosticsOverlay::SetEnabled(false);

    auto result = gui.ExecuteCommand("diag.show");
    assert(result.success);
    assert(result.output == "on");
    assert(atlas::ui::DiagnosticsOverlay::IsEnabled());

    result = gui.ExecuteCommand("diag.hide");
    assert(result.success);
    assert(result.output == "off");
    assert(!atlas::ui::DiagnosticsOverlay::IsEnabled());

    gui.Shutdown();
    manager.Shutdown();
    std::cout << "[PASS] test_headless_gui_diag_show_hide" << std::endl;
}

void test_headless_gui_diag_status() {
    UIManager manager;
    manager.Init(GUIContext::Server);

    HeadlessGUI gui;
    gui.Init(&manager);

    atlas::ui::DiagnosticsOverlay::SetEnabled(false);
    auto result = gui.ExecuteCommand("diag.status");
    assert(result.success);
    assert(result.output == "off");

    atlas::ui::DiagnosticsOverlay::SetEnabled(true);
    result = gui.ExecuteCommand("diag.status");
    assert(result.success);
    assert(result.output == "on");

    atlas::ui::DiagnosticsOverlay::SetEnabled(false);
    gui.Shutdown();
    manager.Shutdown();
    std::cout << "[PASS] test_headless_gui_diag_status" << std::endl;
}
