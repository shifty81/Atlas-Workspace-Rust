// S144 editor tests: ScrollVirtualizerV1, GraphHostContractV1, WorkspaceShellContract
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;
using Catch::Approx;

// ── ScrollVirtualizerV1 ───────────────────────────────────────────────────────

TEST_CASE("SvVirtualItem isValid", "[Editor][S144]") {
    SvVirtualItem item;
    REQUIRE(!item.isValid());
    item.id = 1;
    REQUIRE(item.isValid());
}

TEST_CASE("SvVisibleRange isValid and contains", "[Editor][S144]") {
    SvVisibleRange r{2, 5};
    REQUIRE(r.isValid());
    REQUIRE(r.count() == 4u);
    REQUIRE(r.contains(3));
    REQUIRE(!r.contains(1));
    REQUIRE(!r.contains(6));
}

TEST_CASE("ScrollVirtualizerV1 empty items gives zero height", "[Editor][S144]") {
    ScrollVirtualizerV1 sv;
    REQUIRE(sv.itemCount() == 0u);
    REQUIRE(sv.totalContentHeight() == Approx(0.f));
}

TEST_CASE("ScrollVirtualizerV1 totalContentHeight with fixed items", "[Editor][S144]") {
    ScrollVirtualizerV1 sv;
    std::vector<SvVirtualItem> items;
    for (uint32_t i = 1; i <= 10; ++i) {
        SvVirtualItem item; item.id = i; item.heightPx = 20.f;
        items.push_back(item);
    }
    sv.setItems(std::move(items));
    REQUIRE(sv.itemCount() == 10u);
    REQUIRE(sv.totalContentHeight() == Approx(200.f));
}

TEST_CASE("ScrollVirtualizerV1 setScrollOffset clamping", "[Editor][S144]") {
    ScrollVirtualizerV1 sv;
    std::vector<SvVirtualItem> items;
    for (uint32_t i = 1; i <= 20; ++i) {
        SvVirtualItem it; it.id = i; it.heightPx = 30.f; items.push_back(it);
    }
    sv.setItems(std::move(items));
    sv.setViewportHeight(100.f);
    sv.setScrollOffset(-50.f);
    REQUIRE(sv.scrollOffset() == Approx(0.f));
    sv.setScrollOffset(99999.f);
    REQUIRE(sv.scrollOffset() <= sv.totalContentHeight());
}

TEST_CASE("ScrollVirtualizerV1 computeVisibleRange basic", "[Editor][S144]") {
    ScrollVirtualizerV1 sv;
    std::vector<SvVirtualItem> items;
    for (uint32_t i = 1; i <= 100; ++i) {
        SvVirtualItem it; it.id = i; it.heightPx = 20.f; items.push_back(it);
    }
    sv.setItems(std::move(items));
    sv.setViewportHeight(100.f);
    sv.setScrollOffset(0.f);
    SvVisibleRange vr = sv.computeVisibleRange();
    REQUIRE(vr.isValid());
    REQUIRE(vr.count() > 0u);
    REQUIRE(vr.firstIndex == 0u);
}

TEST_CASE("ScrollVirtualizerV1 scrollToIndex", "[Editor][S144]") {
    ScrollVirtualizerV1 sv;
    std::vector<SvVirtualItem> items;
    for (uint32_t i = 1; i <= 50; ++i) {
        SvVirtualItem it; it.id = i; it.heightPx = 20.f; items.push_back(it);
    }
    sv.setItems(std::move(items));
    sv.setViewportHeight(100.f);
    sv.scrollToIndex(10);
    REQUIRE(sv.scrollOffset() == Approx(sv.itemOffset(10)));
}

TEST_CASE("ScrollVirtualizerV1 scrollBy", "[Editor][S144]") {
    ScrollVirtualizerV1 sv;
    std::vector<SvVirtualItem> items;
    for (uint32_t i = 1; i <= 20; ++i) {
        SvVirtualItem it; it.id = i; it.heightPx = 20.f; items.push_back(it);
    }
    sv.setItems(std::move(items));
    sv.setViewportHeight(100.f);
    sv.scrollBy(60.f);
    REQUIRE(sv.scrollOffset() == Approx(60.f));
    sv.scrollBy(-30.f);
    REQUIRE(sv.scrollOffset() == Approx(30.f));
}

TEST_CASE("ScrollVirtualizerV1 setItemVisible reduces content height", "[Editor][S144]") {
    ScrollVirtualizerV1 sv;
    std::vector<SvVirtualItem> items;
    for (uint32_t i = 1; i <= 5; ++i) {
        SvVirtualItem it; it.id = i; it.heightPx = 10.f; items.push_back(it);
    }
    sv.setItems(std::move(items));
    float before = sv.totalContentHeight();
    REQUIRE(sv.setItemVisible(3, false));
    float after = sv.totalContentHeight();
    REQUIRE(after < before);
    REQUIRE(sv.visibleItemCount() == 4u);
}

TEST_CASE("ScrollVirtualizerV1 setItemHeight updates content height", "[Editor][S144]") {
    ScrollVirtualizerV1 sv;
    std::vector<SvVirtualItem> items;
    SvVirtualItem it; it.id = 1; it.heightPx = 20.f; items.push_back(it);
    sv.setItems(std::move(items));
    REQUIRE(sv.setItemHeight(1, 40.f));
    REQUIRE(sv.totalContentHeight() == Approx(40.f));
}

// ── GraphHostContractV1 ───────────────────────────────────────────────────────

TEST_CASE("GraphCapV1 bitwise operators", "[Editor][S144]") {
    GraphCapV1 caps = GraphCapV1::Undo | GraphCapV1::Redo;
    REQUIRE(hasGraphCapV1(caps, GraphCapV1::Undo));
    REQUIRE(hasGraphCapV1(caps, GraphCapV1::Redo));
    REQUIRE(!hasGraphCapV1(caps, GraphCapV1::Zoom));
}

TEST_CASE("GraphPinType names", "[Editor][S144]") {
    REQUIRE(std::string(graphPinTypeName(GraphPinType::Exec))   == "Exec");
    REQUIRE(std::string(graphPinTypeName(GraphPinType::Float))  == "Float");
    REQUIRE(std::string(graphPinTypeName(GraphPinType::Object)) == "Object");
    REQUIRE(std::string(graphPinTypeName(GraphPinType::Any))    == "Any");
}

TEST_CASE("GraphPinV1 isValid", "[Editor][S144]") {
    GraphPinV1 p;
    REQUIRE(!p.isValid());
    p.id = 1; p.name = "Output";
    REQUIRE(p.isValid());
}

TEST_CASE("GraphNodeV1 isValid and pins", "[Editor][S144]") {
    GraphNodeV1 n;
    REQUIRE(!n.isValid());
    n.id = 1; n.title = "Add";
    REQUIRE(n.isValid());
    REQUIRE(n.pinCount() == 0u);

    GraphPinV1 p; p.id = 1; p.name = "A"; p.type = GraphPinType::Float;
    n.addPin(p);
    REQUIRE(n.pinCount() == 1u);
    REQUIRE(n.findPin(1) != nullptr);
    REQUIRE(n.findPin(99) == nullptr);
}

TEST_CASE("GraphEdgeV1 isValid", "[Editor][S144]") {
    GraphEdgeV1 e;
    REQUIRE(!e.isValid());
    e.id = 1; e.fromNode = 1; e.toNode = 2;
    REQUIRE(e.isValid());
}

TEST_CASE("GraphHostContractV1 initialize", "[Editor][S144]") {
    GraphHostContractV1 g("my-graph");
    REQUIRE(!g.isInitialized());
    g.initialize(GraphCapV1::Undo | GraphCapV1::Redo | GraphCapV1::Zoom);
    REQUIRE(g.isInitialized());
    REQUIRE(g.graphId() == "my-graph");
    REQUIRE(g.hasCapability(GraphCapV1::Undo));
    REQUIRE(!g.hasCapability(GraphCapV1::Pan));
}

TEST_CASE("GraphHostContractV1 addNode and removeNode", "[Editor][S144]") {
    GraphHostContractV1 g("g1");
    g.initialize();
    GraphNodeV1 n; n.id = 1; n.title = "Multiply"; n.x = 10.f; n.y = 20.f;
    REQUIRE(g.addNode(n));
    REQUIRE(g.nodeCount() == 1u);
    REQUIRE(g.changeSeq() == 1u);

    REQUIRE(!g.addNode(n));  // duplicate

    REQUIRE(g.removeNode(1));
    REQUIRE(g.nodeCount() == 0u);
}

TEST_CASE("GraphHostContractV1 addEdge validates endpoints", "[Editor][S144]") {
    GraphHostContractV1 g("g");
    g.initialize();

    GraphNodeV1 a; a.id = 1; a.title = "A";
    GraphNodeV1 b; b.id = 2; b.title = "B";
    g.addNode(a); g.addNode(b);

    GraphEdgeV1 e; e.id = 1; e.fromNode = 1; e.toNode = 2;
    REQUIRE(g.addEdge(e));
    REQUIRE(g.edgeCount() == 1u);

    GraphEdgeV1 bad; bad.id = 2; bad.fromNode = 1; bad.toNode = 999;  // toNode not in graph
    REQUIRE(!g.addEdge(bad));
}

TEST_CASE("GraphHostContractV1 removeNode also removes edges", "[Editor][S144]") {
    GraphHostContractV1 g("g");
    g.initialize();
    GraphNodeV1 a; a.id = 1; a.title = "A";
    GraphNodeV1 b; b.id = 2; b.title = "B";
    g.addNode(a); g.addNode(b);
    GraphEdgeV1 e; e.id = 1; e.fromNode = 1; e.toNode = 2;
    g.addEdge(e);
    REQUIRE(g.edgeCount() == 1u);
    g.removeNode(1);
    REQUIRE(g.edgeCount() == 0u);
}

TEST_CASE("GraphHostContractV1 selectNode and clearSelection", "[Editor][S144]") {
    GraphHostContractV1 g("g");
    g.initialize();
    GraphNodeV1 a; a.id = 1; a.title = "A";
    GraphNodeV1 b; b.id = 2; b.title = "B";
    g.addNode(a); g.addNode(b);

    g.selectNode(1);
    REQUIRE(g.selectedNodeIds().size() == 1u);
    REQUIRE(g.selectedNodeIds()[0] == 1u);

    g.selectNode(2, false);  // non-exclusive
    REQUIRE(g.selectedNodeIds().size() == 2u);

    g.selectNode(2, true);   // exclusive
    REQUIRE(g.selectedNodeIds().size() == 1u);

    g.clearSelection();
    REQUIRE(g.selectedNodeIds().empty());
}

TEST_CASE("GraphHostContractV1 moveNode updates position", "[Editor][S144]") {
    GraphHostContractV1 g("g");
    g.initialize();
    GraphNodeV1 n; n.id = 1; n.title = "N"; n.x = 0.f; n.y = 0.f;
    g.addNode(n);
    REQUIRE(g.moveNode(1, 50.f, 100.f));
    REQUIRE(g.findNode(1)->x == Approx(50.f));
    REQUIRE(g.findNode(1)->y == Approx(100.f));
}

TEST_CASE("GraphHostContractV1 zoom clamping", "[Editor][S144]") {
    GraphHostContractV1 g("g");
    g.initialize();
    g.setViewZoom(0.001f);
    REQUIRE(g.zoom() == Approx(0.1f));
    g.setViewZoom(100.f);
    REQUIRE(g.zoom() == Approx(8.f));
    g.setViewZoom(1.5f);
    REQUIRE(g.zoom() == Approx(1.5f));
}

TEST_CASE("GraphHostContractV1 onChange callback fires", "[Editor][S144]") {
    GraphHostContractV1 g("g");
    g.initialize();
    int calls = 0;
    g.setOnChange([&]() { ++calls; });
    GraphNodeV1 n; n.id = 1; n.title = "N";
    g.addNode(n);
    REQUIRE(calls == 1);
    g.removeNode(1);
    REQUIRE(calls == 2);
}

// ── WorkspaceShellContract ────────────────────────────────────────────────────

TEST_CASE("ShellEventType names", "[Editor][S144]") {
    REQUIRE(std::string(shellEventTypeName(ShellEventType::AppLaunched))        == "AppLaunched");
    REQUIRE(std::string(shellEventTypeName(ShellEventType::ProjectOpened))      == "ProjectOpened");
    REQUIRE(std::string(shellEventTypeName(ShellEventType::LayoutChanged))      == "LayoutChanged");
    REQUIRE(std::string(shellEventTypeName(ShellEventType::FocusPanelChanged))  == "FocusPanelChanged");
}

TEST_CASE("ShellStatus names", "[Editor][S144]") {
    REQUIRE(std::string(shellStatusName(ShellStatus::Idle))        == "Idle");
    REQUIRE(std::string(shellStatusName(ShellStatus::Ready))       == "Ready");
    REQUIRE(std::string(shellStatusName(ShellStatus::Error))       == "Error");
    REQUIRE(std::string(shellStatusName(ShellStatus::ShuttingDown))== "ShuttingDown");
}

TEST_CASE("WorkspaceShellContract initialize and status", "[Editor][S144]") {
    WorkspaceShellContract shell;
    REQUIRE(!shell.isInitialized());
    REQUIRE(shell.status() == ShellStatus::Idle);
    shell.initialize();
    REQUIRE(shell.isInitialized());
    REQUIRE(shell.status() == ShellStatus::Ready);
    shell.shutdown();
    REQUIRE(shell.status() == ShellStatus::ShuttingDown);
}

TEST_CASE("WorkspaceShellContract subscribe and receive events", "[Editor][S144]") {
    WorkspaceShellContract shell;
    shell.initialize();

    std::vector<ShellEvent> received;
    REQUIRE(shell.subscribe("sub-1", [&](const ShellEvent& e) { received.push_back(e); }));

    Notification n;
    n.id = "notif-1"; n.title = "Test"; n.severity = NotificationSeverity::Info;
    shell.postNotification(n);

    REQUIRE(!received.empty());
    REQUIRE(received.back().type == ShellEventType::NotificationPosted);
    REQUIRE(shell.notificationCount() == 1u);
}

TEST_CASE("WorkspaceShellContract reject duplicate subscriber", "[Editor][S144]") {
    WorkspaceShellContract shell;
    shell.initialize();
    REQUIRE(shell.subscribe("sub-1", [](const ShellEvent&) {}));
    REQUIRE(!shell.subscribe("sub-1", [](const ShellEvent&) {}));
}

TEST_CASE("WorkspaceShellContract unsubscribe", "[Editor][S144]") {
    WorkspaceShellContract shell;
    shell.initialize();
    shell.subscribe("sub-1", [](const ShellEvent&) {});
    REQUIRE(shell.subscriberCount() == 1u);
    REQUIRE(shell.unsubscribe("sub-1"));
    REQUIRE(shell.subscriberCount() == 0u);
    REQUIRE(!shell.unsubscribe("sub-1")); // already gone
}

TEST_CASE("WorkspaceShellContract setFocusPanel posts event", "[Editor][S144]") {
    WorkspaceShellContract shell;
    shell.initialize();
    ShellEvent lastEvent;
    shell.subscribe("s", [&](const ShellEvent& e) { lastEvent = e; });
    shell.setFocusPanel("scene-hierarchy");
    REQUIRE(shell.focusPanelId() == "scene-hierarchy");
    REQUIRE(lastEvent.type    == ShellEventType::FocusPanelChanged);
    REQUIRE(lastEvent.payload == "scene-hierarchy");
}

TEST_CASE("WorkspaceShellContract applyLayout with manager", "[Editor][S144]") {
    WorkspaceShellContract shell;
    shell.initialize();

    LayoutManagerV1 mgr;
    LayoutSlot s; s.id = 1; s.name = "Default";
    mgr.addSlot(s);
    shell.bindLayoutManager(&mgr);

    ShellEvent lastEvent;
    shell.subscribe("s", [&](const ShellEvent& e) { lastEvent = e; });

    REQUIRE(shell.applyLayout(1));
    REQUIRE(shell.activeLayoutSlotId() == 1u);
    REQUIRE(lastEvent.type == ShellEventType::LayoutChanged);

    REQUIRE(!shell.applyLayout(99));  // nonexistent slot
}

TEST_CASE("WorkspaceShellContract launchApp without registry always uses allowDirectLaunch default", "[Editor][S144]") {
    WorkspaceShellContract shell;
    shell.initialize();
    // Without a bound registry, launchApp should succeed
    REQUIRE(shell.launchApp(WorkspaceAppId::TileEditor));
    REQUIRE(shell.isAppRunning(WorkspaceAppId::TileEditor));
    REQUIRE(shell.launchCount() == 1u);
    REQUIRE(shell.stopApp(WorkspaceAppId::TileEditor));
    REQUIRE(!shell.isAppRunning(WorkspaceAppId::TileEditor));
}

TEST_CASE("WorkspaceShellContract postNotification with notification limit", "[Editor][S144]") {
    WorkspaceShellContract shell;
    shell.initialize();
    for (int i = 0; i < 110; ++i) {
        Notification n;
        n.id = "notif-" + std::to_string(i + 1);
        n.title = "N" + std::to_string(i);
        n.severity = NotificationSeverity::Info;
        shell.postNotification(n);
    }
    // Max stored notifications is 100
    REQUIRE(shell.recentNotifications().size() <= 100u);
    REQUIRE(shell.notificationCount() == 110u);
}

TEST_CASE("WorkspaceShellContract event count increments", "[Editor][S144]") {
    WorkspaceShellContract shell;
    shell.initialize();
    REQUIRE(shell.eventCount() == 0u);
    shell.setFocusPanel("p1");
    REQUIRE(shell.eventCount() == 1u);
    shell.setFocusPanel("p2");
    REQUIRE(shell.eventCount() == 2u);
}
