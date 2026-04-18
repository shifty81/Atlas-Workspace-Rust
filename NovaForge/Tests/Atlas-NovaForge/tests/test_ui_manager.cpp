#include "../engine/ui/UIManager.h"
#include "../engine/ui/UINodes.h"
#include <iostream>
#include <cassert>

using namespace atlas::ui;

void test_ui_manager_init() {
    UIManager mgr;
    assert(!mgr.IsInitialized());

    mgr.Init(GUIContext::Editor);
    assert(mgr.IsInitialized());
    assert(mgr.GetContext() == GUIContext::Editor);
    assert(mgr.GetScreen().GetName() == "EditorScreen");
    std::cout << "[PASS] test_ui_manager_init" << std::endl;
}

void test_ui_manager_context_switch() {
    UIManager mgr;
    mgr.Init(GUIContext::Editor);

    assert(mgr.GetContext() == GUIContext::Editor);

    mgr.SetContext(GUIContext::Game);
    assert(mgr.GetContext() == GUIContext::Game);

    mgr.SetContext(GUIContext::Server);
    assert(mgr.GetContext() == GUIContext::Server);
    std::cout << "[PASS] test_ui_manager_context_switch" << std::endl;
}

void test_ui_manager_screen_access() {
    UIManager mgr;
    mgr.Init(GUIContext::Game);

    auto& screen = mgr.GetScreen();
    assert(screen.GetName() == "GameScreen");

    uint32_t id = screen.AddWidget(UIWidgetType::Button, "test_btn", 0, 0, 100, 30);
    assert(screen.WidgetCount() == 1);
    assert(screen.GetWidget(id) != nullptr);
    std::cout << "[PASS] test_ui_manager_screen_access" << std::endl;
}

void test_ui_manager_graph_access() {
    UIManager mgr;
    mgr.Init(GUIContext::Editor);

    auto& graph = mgr.GetGraph();
    auto panelId = graph.AddNode(std::make_unique<PanelNode>());
    assert(graph.NodeCount() == 1);
    assert(graph.Compile());
    std::cout << "[PASS] test_ui_manager_graph_access" << std::endl;
}

void test_ui_manager_update() {
    UIManager mgr;
    mgr.Init(GUIContext::Editor);

    auto& graph = mgr.GetGraph();
    auto panel = std::make_unique<PanelNode>();
    panel->width = 400;
    panel->height = 300;
    auto panelId = graph.AddNode(std::move(panel));
    graph.Compile();

    UIContext ctx{1920.0f, 1080.0f, 0.016f, 1};
    mgr.Update(ctx);

    auto* output = graph.GetOutput(panelId, 0);
    assert(output != nullptr);
    assert(output->data[2] == 400.0f);
    assert(output->data[3] == 300.0f);
    std::cout << "[PASS] test_ui_manager_update" << std::endl;
}

void test_ui_manager_command_bus() {
    UIManager mgr;
    mgr.Init(GUIContext::Editor);

    int handlerCalled = 0;
    mgr.GetCommandBus().RegisterHandler(UICommandType::ButtonPress, [&](const UICommand&) {
        handlerCalled++;
    });

    UICommand cmd;
    cmd.type = UICommandType::ButtonPress;
    cmd.targetWidgetId = 5;
    mgr.GetCommandBus().Enqueue(cmd);

    UIContext ctx{1920.0f, 1080.0f, 0.016f, 1};
    mgr.Update(ctx);

    assert(handlerCalled == 1);
    std::cout << "[PASS] test_ui_manager_command_bus" << std::endl;
}

void test_ui_manager_shutdown() {
    UIManager mgr;
    mgr.Init(GUIContext::Editor);
    assert(mgr.IsInitialized());

    mgr.Shutdown();
    assert(!mgr.IsInitialized());
    std::cout << "[PASS] test_ui_manager_shutdown" << std::endl;
}

void test_ui_manager_server_context() {
    UIManager mgr;
    mgr.Init(GUIContext::Server);

    assert(mgr.GetScreen().GetName() == "ServerScreen");
    assert(mgr.GetContext() == GUIContext::Server);

    // Server can still manage widgets (headless GUI tree)
    auto& screen = mgr.GetScreen();
    uint32_t id = screen.AddWidget(UIWidgetType::Panel, "status", 0, 0, 800, 600);
    assert(screen.WidgetCount() == 1);
    std::cout << "[PASS] test_ui_manager_server_context" << std::endl;
}
