// Tests/Workspace/test_phase12_event_bus.cpp
// Phase 12 — Event Bus and Workspace Notifications
//
// Tests for:
//   1. WorkspaceEventType    — enum names
//   2. WorkspaceEvent        — construction, validity, factory, priority queries
//   3. WorkspaceEventBus     — subscribe, unsubscribe, publish, wildcard, per-type, stats
//   4. WorkspaceEventQueue   — enqueue, drain, priority ordering, tick-based drain
//   5. WorkspaceNotificationBus — notify, history, severity, markRead, layered bus events
//   6. Integration           — bus + queue + notification pipeline

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceEventBus.h"
#include <string>
#include <vector>

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — WorkspaceEventType
// ═════════════════════════════════════════════════════════════════

TEST_CASE("workspaceEventTypeName returns correct strings", "[Phase12][EventType]") {
    CHECK(std::string(workspaceEventTypeName(WorkspaceEventType::Tool))         == "Tool");
    CHECK(std::string(workspaceEventTypeName(WorkspaceEventType::Panel))        == "Panel");
    CHECK(std::string(workspaceEventTypeName(WorkspaceEventType::Project))      == "Project");
    CHECK(std::string(workspaceEventTypeName(WorkspaceEventType::Asset))        == "Asset");
    CHECK(std::string(workspaceEventTypeName(WorkspaceEventType::Command))      == "Command");
    CHECK(std::string(workspaceEventTypeName(WorkspaceEventType::Selection))    == "Selection");
    CHECK(std::string(workspaceEventTypeName(WorkspaceEventType::Layout))       == "Layout");
    CHECK(std::string(workspaceEventTypeName(WorkspaceEventType::Notification)) == "Notification");
    CHECK(std::string(workspaceEventTypeName(WorkspaceEventType::AI))           == "AI");
    CHECK(std::string(workspaceEventTypeName(WorkspaceEventType::System))       == "System");
    CHECK(std::string(workspaceEventTypeName(WorkspaceEventType::Custom))       == "Custom");
}

TEST_CASE("workspaceEventPriorityName returns correct strings", "[Phase12][EventType]") {
    CHECK(std::string(workspaceEventPriorityName(WorkspaceEventPriority::Low))      == "Low");
    CHECK(std::string(workspaceEventPriorityName(WorkspaceEventPriority::Normal))   == "Normal");
    CHECK(std::string(workspaceEventPriorityName(WorkspaceEventPriority::High))     == "High");
    CHECK(std::string(workspaceEventPriorityName(WorkspaceEventPriority::Critical)) == "Critical");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — WorkspaceEvent
// ═════════════════════════════════════════════════════════════════

TEST_CASE("WorkspaceEvent default-constructed is not valid", "[Phase12][Event]") {
    WorkspaceEvent ev;
    CHECK_FALSE(ev.isValid());
    CHECK(ev.eventType == WorkspaceEventType::Custom);
    CHECK(ev.priority  == WorkspaceEventPriority::Normal);
}

TEST_CASE("WorkspaceEvent::make creates valid event", "[Phase12][Event]") {
    auto ev = WorkspaceEvent::make(WorkspaceEventType::Tool, "scene_editor", "activated", WorkspaceEventPriority::High, 12345);
    CHECK(ev.isValid());
    CHECK(ev.eventType      == WorkspaceEventType::Tool);
    CHECK(ev.source         == "scene_editor");
    CHECK(ev.payload        == "activated");
    CHECK(ev.priority       == WorkspaceEventPriority::High);
    CHECK(ev.timestampToken == 12345);
}

TEST_CASE("WorkspaceEvent isHighPriority and isCritical", "[Phase12][Event]") {
    auto low  = WorkspaceEvent::make(WorkspaceEventType::Asset, "s", "", WorkspaceEventPriority::Low);
    auto norm = WorkspaceEvent::make(WorkspaceEventType::Asset, "s", "", WorkspaceEventPriority::Normal);
    auto high = WorkspaceEvent::make(WorkspaceEventType::Asset, "s", "", WorkspaceEventPriority::High);
    auto crit = WorkspaceEvent::make(WorkspaceEventType::Asset, "s", "", WorkspaceEventPriority::Critical);

    CHECK_FALSE(low.isHighPriority());
    CHECK_FALSE(norm.isHighPriority());
    CHECK(high.isHighPriority());
    CHECK(crit.isHighPriority());

    CHECK_FALSE(low.isCritical());
    CHECK_FALSE(high.isCritical());
    CHECK(crit.isCritical());
}

TEST_CASE("WorkspaceEvent with empty source is not valid", "[Phase12][Event]") {
    auto ev = WorkspaceEvent::make(WorkspaceEventType::System, "", "payload");
    CHECK_FALSE(ev.isValid());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — WorkspaceEventBus
// ═════════════════════════════════════════════════════════════════

TEST_CASE("WorkspaceEventBus empty on construction", "[Phase12][Bus]") {
    WorkspaceEventBus bus;
    CHECK(bus.empty());
    CHECK(bus.subscriptionCount() == 0);
    CHECK(bus.totalPublished()    == 0);
    CHECK(bus.totalDispatches()   == 0);
}

TEST_CASE("WorkspaceEventBus subscribe returns nonzero id", "[Phase12][Bus]") {
    WorkspaceEventBus bus;
    auto id = bus.subscribe(WorkspaceEventType::Tool, [](const WorkspaceEvent&){});
    CHECK(id != 0);
    CHECK(bus.subscriptionCount() == 1);
}

TEST_CASE("WorkspaceEventBus publish dispatches to matching subscriber", "[Phase12][Bus]") {
    WorkspaceEventBus bus;
    int callCount = 0;
    bus.subscribe(WorkspaceEventType::Tool, [&](const WorkspaceEvent& ev) {
        CHECK(ev.source == "scene_editor");
        ++callCount;
    });

    auto ev = WorkspaceEvent::make(WorkspaceEventType::Tool, "scene_editor", "activated");
    size_t dispatched = bus.publish(ev);

    CHECK(dispatched == 1);
    CHECK(callCount  == 1);
    CHECK(bus.totalPublished()  == 1);
    CHECK(bus.totalDispatches() == 1);
}

TEST_CASE("WorkspaceEventBus does not dispatch to non-matching type", "[Phase12][Bus]") {
    WorkspaceEventBus bus;
    int callCount = 0;
    bus.subscribe(WorkspaceEventType::Asset, [&](const WorkspaceEvent&) { ++callCount; });

    auto ev = WorkspaceEvent::make(WorkspaceEventType::Tool, "src", "p");
    bus.publish(ev);

    CHECK(callCount == 0);
    CHECK(bus.totalPublished()  == 1);
    CHECK(bus.totalDispatches() == 0);
}

TEST_CASE("WorkspaceEventBus source filter restricts delivery", "[Phase12][Bus]") {
    WorkspaceEventBus bus;
    int callCount = 0;
    bus.subscribe(WorkspaceEventType::Tool, [&](const WorkspaceEvent&) { ++callCount; }, "scene_editor");

    // Matching source
    bus.publish(WorkspaceEvent::make(WorkspaceEventType::Tool, "scene_editor"));
    CHECK(callCount == 1);

    // Non-matching source
    bus.publish(WorkspaceEvent::make(WorkspaceEventType::Tool, "asset_editor"));
    CHECK(callCount == 1);
}

TEST_CASE("WorkspaceEventBus wildcard subscription receives all types", "[Phase12][Bus]") {
    WorkspaceEventBus bus;
    int callCount = 0;
    bus.subscribeAll([&](const WorkspaceEvent&) { ++callCount; });

    bus.publish(WorkspaceEvent::make(WorkspaceEventType::Tool,      "a"));
    bus.publish(WorkspaceEvent::make(WorkspaceEventType::Asset,     "b"));
    bus.publish(WorkspaceEvent::make(WorkspaceEventType::Selection, "c"));

    CHECK(callCount == 3);
}

TEST_CASE("WorkspaceEventBus unsubscribe removes subscription", "[Phase12][Bus]") {
    WorkspaceEventBus bus;
    int callCount = 0;
    auto id = bus.subscribe(WorkspaceEventType::Tool, [&](const WorkspaceEvent&) { ++callCount; });
    CHECK(bus.subscriptionCount() == 1);

    bool removed = bus.unsubscribe(id);
    CHECK(removed);
    CHECK(bus.subscriptionCount() == 0);

    bus.publish(WorkspaceEvent::make(WorkspaceEventType::Tool, "src"));
    CHECK(callCount == 0);
}

TEST_CASE("WorkspaceEventBus unsubscribe unknown id returns false", "[Phase12][Bus]") {
    WorkspaceEventBus bus;
    CHECK_FALSE(bus.unsubscribe(9999));
}

TEST_CASE("WorkspaceEventBus publish invalid event returns 0", "[Phase12][Bus]") {
    WorkspaceEventBus bus;
    bus.subscribe(WorkspaceEventType::Tool, [](const WorkspaceEvent&){});

    WorkspaceEvent invalid; // source empty → invalid
    CHECK(bus.publish(invalid) == 0);
    CHECK(bus.totalPublished() == 0);
}

TEST_CASE("WorkspaceEventBus multiple subscribers same type", "[Phase12][Bus]") {
    WorkspaceEventBus bus;
    int a = 0, b = 0;
    bus.subscribe(WorkspaceEventType::Project, [&](const WorkspaceEvent&) { ++a; });
    bus.subscribe(WorkspaceEventType::Project, [&](const WorkspaceEvent&) { ++b; });

    bus.publish(WorkspaceEvent::make(WorkspaceEventType::Project, "proj_mgr"));
    CHECK(a == 1);
    CHECK(b == 1);
    CHECK(bus.totalDispatches() == 2);
}

TEST_CASE("WorkspaceEventBus find by id", "[Phase12][Bus]") {
    WorkspaceEventBus bus;
    auto id = bus.subscribe(WorkspaceEventType::System, [](const WorkspaceEvent&){});

    auto* sub = bus.find(id);
    REQUIRE(sub != nullptr);
    CHECK(sub->type() == WorkspaceEventType::System);
    CHECK(bus.find(9999) == nullptr);
}

TEST_CASE("WorkspaceEventBus countByType", "[Phase12][Bus]") {
    WorkspaceEventBus bus;
    bus.subscribe(WorkspaceEventType::Tool,  [](const WorkspaceEvent&){});
    bus.subscribe(WorkspaceEventType::Tool,  [](const WorkspaceEvent&){});
    bus.subscribe(WorkspaceEventType::Asset, [](const WorkspaceEvent&){});

    CHECK(bus.countByType(WorkspaceEventType::Tool)  == 2);
    CHECK(bus.countByType(WorkspaceEventType::Asset) == 1);
    CHECK(bus.countByType(WorkspaceEventType::Panel) == 0);
}

TEST_CASE("WorkspaceEventBus deliveryCount tracks per subscription", "[Phase12][Bus]") {
    WorkspaceEventBus bus;
    auto id = bus.subscribe(WorkspaceEventType::Command, [](const WorkspaceEvent&){});

    bus.publish(WorkspaceEvent::make(WorkspaceEventType::Command, "cmd_bus"));
    bus.publish(WorkspaceEvent::make(WorkspaceEventType::Command, "cmd_bus"));
    bus.publish(WorkspaceEvent::make(WorkspaceEventType::Command, "cmd_bus"));

    auto* sub = bus.find(id);
    REQUIRE(sub != nullptr);
    CHECK(sub->deliveryCount() == 3);
}

TEST_CASE("WorkspaceEventBus clear resets everything", "[Phase12][Bus]") {
    WorkspaceEventBus bus;
    bus.subscribe(WorkspaceEventType::Tool, [](const WorkspaceEvent&){});
    bus.publish(WorkspaceEvent::make(WorkspaceEventType::Tool, "src"));

    bus.clear();
    CHECK(bus.empty());
    CHECK(bus.totalPublished()  == 0);
    CHECK(bus.totalDispatches() == 0);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — WorkspaceEventQueue
// ═════════════════════════════════════════════════════════════════

TEST_CASE("WorkspaceEventQueue empty on construction", "[Phase12][Queue]") {
    WorkspaceEventBus bus;
    WorkspaceEventQueue queue(bus);
    CHECK(queue.empty());
    CHECK(queue.queueSize()    == 0);
    CHECK(queue.totalDrained() == 0);
}

TEST_CASE("WorkspaceEventQueue enqueue adds event", "[Phase12][Queue]") {
    WorkspaceEventBus bus;
    WorkspaceEventQueue queue(bus);

    bool ok = queue.enqueue(WorkspaceEvent::make(WorkspaceEventType::Asset, "importer"));
    CHECK(ok);
    CHECK(queue.queueSize() == 1);
}

TEST_CASE("WorkspaceEventQueue enqueue rejects invalid event", "[Phase12][Queue]") {
    WorkspaceEventBus bus;
    WorkspaceEventQueue queue(bus);

    WorkspaceEvent invalid; // no source
    CHECK_FALSE(queue.enqueue(invalid));
    CHECK(queue.queueSize() == 0);
}

TEST_CASE("WorkspaceEventQueue drain dispatches and clears", "[Phase12][Queue]") {
    WorkspaceEventBus bus;
    int callCount = 0;
    bus.subscribe(WorkspaceEventType::Tool, [&](const WorkspaceEvent&) { ++callCount; });

    WorkspaceEventQueue queue(bus);
    queue.enqueue(WorkspaceEvent::make(WorkspaceEventType::Tool, "src1"));
    queue.enqueue(WorkspaceEvent::make(WorkspaceEventType::Tool, "src2"));

    size_t deliveries = queue.drain();
    CHECK(deliveries == 2);
    CHECK(queue.empty());
    CHECK(queue.totalDrained() == 2);
}

TEST_CASE("WorkspaceEventQueue drain sorts by priority", "[Phase12][Queue]") {
    WorkspaceEventBus bus;
    std::vector<std::string> order;
    bus.subscribeAll([&](const WorkspaceEvent& ev) {
        order.push_back(ev.payload);
    });

    WorkspaceEventQueue queue(bus);
    queue.enqueue(WorkspaceEvent::make(WorkspaceEventType::Tool, "s", "low",  WorkspaceEventPriority::Low));
    queue.enqueue(WorkspaceEvent::make(WorkspaceEventType::Tool, "s", "crit", WorkspaceEventPriority::Critical));
    queue.enqueue(WorkspaceEvent::make(WorkspaceEventType::Tool, "s", "norm", WorkspaceEventPriority::Normal));
    queue.enqueue(WorkspaceEvent::make(WorkspaceEventType::Tool, "s", "high", WorkspaceEventPriority::High));

    queue.drain();

    REQUIRE(order.size() == 4);
    CHECK(order[0] == "crit");
    CHECK(order[1] == "high");
    CHECK(order[2] == "norm");
    CHECK(order[3] == "low");
}

TEST_CASE("WorkspaceEventQueue drain empty returns 0", "[Phase12][Queue]") {
    WorkspaceEventBus bus;
    WorkspaceEventQueue queue(bus);
    CHECK(queue.drain() == 0);
}

TEST_CASE("WorkspaceEventQueue tick drains after interval", "[Phase12][Queue]") {
    WorkspaceEventBus bus;
    int callCount = 0;
    bus.subscribe(WorkspaceEventType::System, [&](const WorkspaceEvent&) { ++callCount; });

    WorkspaceEventQueue queue(bus);
    queue.setDrainInterval(0.5f);
    queue.enqueue(WorkspaceEvent::make(WorkspaceEventType::System, "sys"));

    // Not enough time elapsed
    queue.tick(0.3f);
    CHECK(callCount == 0);

    // Now enough time
    queue.tick(0.3f);
    CHECK(callCount == 1);
    CHECK(queue.empty());
}

TEST_CASE("WorkspaceEventQueue tick does not drain when empty", "[Phase12][Queue]") {
    WorkspaceEventBus bus;
    WorkspaceEventQueue queue(bus);
    queue.setDrainInterval(0.1f);
    CHECK(queue.tick(1.0f) == 0);
}

TEST_CASE("WorkspaceEventQueue clearQueue removes pending events", "[Phase12][Queue]") {
    WorkspaceEventBus bus;
    WorkspaceEventQueue queue(bus);
    queue.enqueue(WorkspaceEvent::make(WorkspaceEventType::Tool, "s"));
    queue.enqueue(WorkspaceEvent::make(WorkspaceEventType::Tool, "s"));
    CHECK(queue.queueSize() == 2);

    queue.clearQueue();
    CHECK(queue.empty());
}

TEST_CASE("WorkspaceEventQueue pending returns read-only view", "[Phase12][Queue]") {
    WorkspaceEventBus bus;
    WorkspaceEventQueue queue(bus);
    queue.enqueue(WorkspaceEvent::make(WorkspaceEventType::Asset, "src", "data1"));
    queue.enqueue(WorkspaceEvent::make(WorkspaceEventType::Asset, "src", "data2"));

    const auto& pending = queue.pending();
    CHECK(pending.size() == 2);
    CHECK(pending[0].payload == "data1");
    CHECK(pending[1].payload == "data2");
}

TEST_CASE("WorkspaceEventQueue drainInterval defaults and setter", "[Phase12][Queue]") {
    WorkspaceEventBus bus;
    WorkspaceEventQueue queue(bus);
    CHECK(queue.drainInterval() > 0.f);

    queue.setDrainInterval(2.0f);
    CHECK(queue.drainInterval() == 2.0f);

    // Negative → clamped to minimum
    queue.setDrainInterval(-1.0f);
    CHECK(queue.drainInterval() > 0.f);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — WorkspaceNotificationBus
// ═════════════════════════════════════════════════════════════════

TEST_CASE("wsNotificationSeverityName returns correct strings", "[Phase12][Notification]") {
    CHECK(std::string(wsNotificationSeverityName(WsNotificationSeverity::Info))     == "Info");
    CHECK(std::string(wsNotificationSeverityName(WsNotificationSeverity::Success))  == "Success");
    CHECK(std::string(wsNotificationSeverityName(WsNotificationSeverity::Warning))  == "Warning");
    CHECK(std::string(wsNotificationSeverityName(WsNotificationSeverity::Error))    == "Error");
    CHECK(std::string(wsNotificationSeverityName(WsNotificationSeverity::Critical)) == "Critical");
}

TEST_CASE("WorkspaceNotificationEntry default is not valid", "[Phase12][Notification]") {
    WorkspaceNotificationEntry e;
    CHECK_FALSE(e.isValid());
    CHECK_FALSE(e.isError());
    CHECK(e.isUnread());
}

TEST_CASE("WorkspaceNotificationEntry markRead toggles state", "[Phase12][Notification]") {
    WorkspaceNotificationEntry e;
    e.title = "Test";
    CHECK(e.isUnread());
    e.markRead();
    CHECK_FALSE(e.isUnread());
}

TEST_CASE("WorkspaceNotificationEntry isError and isCritical", "[Phase12][Notification]") {
    WorkspaceNotificationEntry e;
    e.title = "x";
    e.severity = WsNotificationSeverity::Warning;
    CHECK_FALSE(e.isError());

    e.severity = WsNotificationSeverity::Error;
    CHECK(e.isError());
    CHECK_FALSE(e.isCritical());

    e.severity = WsNotificationSeverity::Critical;
    CHECK(e.isError());
    CHECK(e.isCritical());
}

TEST_CASE("WorkspaceNotificationBus empty on construction", "[Phase12][Notification]") {
    WorkspaceEventBus bus;
    WorkspaceNotificationBus notifBus(bus);
    CHECK(notifBus.empty());
    CHECK(notifBus.historySize() == 0);
    CHECK(notifBus.unreadCount() == 0);
}

TEST_CASE("WorkspaceNotificationBus notify stores in history", "[Phase12][Notification]") {
    WorkspaceEventBus bus;
    WorkspaceNotificationBus notifBus(bus);

    uint32_t id = notifBus.notify(WsNotificationSeverity::Info, "Build", "Succeeded", "build_tool");
    CHECK(id != 0);
    CHECK(notifBus.historySize() == 1);

    auto* entry = notifBus.find(id);
    REQUIRE(entry != nullptr);
    CHECK(entry->title    == "Build");
    CHECK(entry->message  == "Succeeded");
    CHECK(entry->source   == "build_tool");
    CHECK(entry->severity == WsNotificationSeverity::Info);
    CHECK(entry->isUnread());
}

TEST_CASE("WorkspaceNotificationBus notify publishes event on bus", "[Phase12][Notification]") {
    WorkspaceEventBus bus;
    int busCallCount = 0;
    bus.subscribe(WorkspaceEventType::Notification, [&](const WorkspaceEvent& ev) {
        CHECK(ev.source == "build_tool");
        ++busCallCount;
    });

    WorkspaceNotificationBus notifBus(bus);
    notifBus.notify(WsNotificationSeverity::Warning, "Compile", "3 warnings", "build_tool");

    CHECK(busCallCount == 1);
}

TEST_CASE("WorkspaceNotificationBus convenience helpers", "[Phase12][Notification]") {
    WorkspaceEventBus bus;
    WorkspaceNotificationBus notifBus(bus);

    notifBus.info("A", "a");
    notifBus.success("B", "b");
    notifBus.warning("C", "c");
    notifBus.error("D", "d");
    notifBus.critical("E", "e");

    CHECK(notifBus.historySize() == 5);
    CHECK(notifBus.countBySeverity(WsNotificationSeverity::Info)     == 1);
    CHECK(notifBus.countBySeverity(WsNotificationSeverity::Success)  == 1);
    CHECK(notifBus.countBySeverity(WsNotificationSeverity::Warning)  == 1);
    CHECK(notifBus.countBySeverity(WsNotificationSeverity::Error)    == 1);
    CHECK(notifBus.countBySeverity(WsNotificationSeverity::Critical) == 1);
}

TEST_CASE("WorkspaceNotificationBus markRead", "[Phase12][Notification]") {
    WorkspaceEventBus bus;
    WorkspaceNotificationBus notifBus(bus);

    auto id1 = notifBus.info("A", "a");
    auto id2 = notifBus.info("B", "b");
    CHECK(notifBus.unreadCount() == 2);

    CHECK(notifBus.markRead(id1));
    CHECK(notifBus.unreadCount() == 1);

    CHECK_FALSE(notifBus.markRead(9999)); // unknown id
}

TEST_CASE("WorkspaceNotificationBus markAllRead", "[Phase12][Notification]") {
    WorkspaceEventBus bus;
    WorkspaceNotificationBus notifBus(bus);

    notifBus.info("A", "a");
    notifBus.info("B", "b");
    notifBus.info("C", "c");
    CHECK(notifBus.unreadCount() == 3);

    notifBus.markAllRead();
    CHECK(notifBus.unreadCount() == 0);
}

TEST_CASE("WorkspaceNotificationBus errorCount", "[Phase12][Notification]") {
    WorkspaceEventBus bus;
    WorkspaceNotificationBus notifBus(bus);

    notifBus.info("A", "a");
    notifBus.warning("B", "b");
    notifBus.error("C", "c");
    notifBus.critical("D", "d");

    CHECK(notifBus.errorCount() == 2); // Error + Critical
}

TEST_CASE("WorkspaceNotificationBus clearHistory", "[Phase12][Notification]") {
    WorkspaceEventBus bus;
    WorkspaceNotificationBus notifBus(bus);

    notifBus.info("A", "a");
    notifBus.info("B", "b");
    CHECK(notifBus.historySize() == 2);

    notifBus.clearHistory();
    CHECK(notifBus.empty());
}

TEST_CASE("WorkspaceNotificationBus error/critical publish High/Critical priority events", "[Phase12][Notification]") {
    WorkspaceEventBus bus;
    std::vector<WorkspaceEventPriority> receivedPriorities;
    bus.subscribe(WorkspaceEventType::Notification, [&](const WorkspaceEvent& ev) {
        receivedPriorities.push_back(ev.priority);
    });

    WorkspaceNotificationBus notifBus(bus);
    notifBus.info("A", "a");       // Normal priority
    notifBus.error("B", "b");      // High priority
    notifBus.critical("C", "c");   // Critical priority

    REQUIRE(receivedPriorities.size() == 3);
    CHECK(receivedPriorities[0] == WorkspaceEventPriority::Normal);
    CHECK(receivedPriorities[1] == WorkspaceEventPriority::High);
    CHECK(receivedPriorities[2] == WorkspaceEventPriority::Critical);
}

TEST_CASE("WorkspaceNotificationBus default source is workspace", "[Phase12][Notification]") {
    WorkspaceEventBus bus;
    WorkspaceNotificationBus notifBus(bus);

    auto id = notifBus.info("Test", "msg"); // no source provided
    auto* entry = notifBus.find(id);
    REQUIRE(entry != nullptr);
    CHECK(entry->source == "workspace");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 6 — Integration Tests
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: event bus dispatches to multiple type-specific subscribers", "[Phase12][Integration]") {
    WorkspaceEventBus bus;
    int toolCount = 0, assetCount = 0, wildcardCount = 0;

    bus.subscribe(WorkspaceEventType::Tool,  [&](const WorkspaceEvent&) { ++toolCount;     });
    bus.subscribe(WorkspaceEventType::Asset, [&](const WorkspaceEvent&) { ++assetCount;    });
    bus.subscribeAll([&](const WorkspaceEvent&) { ++wildcardCount; });

    bus.publish(WorkspaceEvent::make(WorkspaceEventType::Tool,  "editor"));
    bus.publish(WorkspaceEvent::make(WorkspaceEventType::Asset, "importer"));
    bus.publish(WorkspaceEvent::make(WorkspaceEventType::Panel, "inspector"));

    CHECK(toolCount     == 1);
    CHECK(assetCount    == 1);
    CHECK(wildcardCount == 3);
    CHECK(bus.totalPublished()  == 3);
    CHECK(bus.totalDispatches() == 5); // 1+1 type-specific + 3 wildcard
}

TEST_CASE("Integration: queue accumulates and drains through bus", "[Phase12][Integration]") {
    WorkspaceEventBus bus;
    std::vector<std::string> received;
    bus.subscribeAll([&](const WorkspaceEvent& ev) {
        received.push_back(ev.payload);
    });

    WorkspaceEventQueue queue(bus);
    queue.enqueue(WorkspaceEvent::make(WorkspaceEventType::System, "s", "startup"));
    queue.enqueue(WorkspaceEvent::make(WorkspaceEventType::Tool,   "s", "tool_activated"));
    queue.enqueue(WorkspaceEvent::make(WorkspaceEventType::Panel,  "s", "panel_opened"));

    // Events not dispatched until drain
    CHECK(received.empty());

    queue.drain();
    CHECK(received.size() == 3);
}

TEST_CASE("Integration: notification bus fires bus events that queue can defer", "[Phase12][Integration]") {
    WorkspaceEventBus bus;

    // Subscribe to notification events on the bus
    std::vector<std::string> notifications;
    bus.subscribe(WorkspaceEventType::Notification, [&](const WorkspaceEvent& ev) {
        notifications.push_back(ev.payload);
    });

    WorkspaceNotificationBus notifBus(bus);

    // Notifications are published synchronously (bypass queue)
    notifBus.info("Build", "Started", "ci");
    notifBus.success("Build", "Completed", "ci");
    notifBus.error("Build", "Failed: linker error", "ci");

    CHECK(notifications.size() == 3);
    CHECK(notifBus.historySize() == 3);
    CHECK(notifBus.errorCount()  == 1);
    CHECK(notifBus.unreadCount() == 3);
}

TEST_CASE("Integration: tick-based queue with mixed priorities", "[Phase12][Integration]") {
    WorkspaceEventBus bus;
    std::vector<std::string> deliveryOrder;
    bus.subscribeAll([&](const WorkspaceEvent& ev) {
        deliveryOrder.push_back(ev.payload);
    });

    WorkspaceEventQueue queue(bus);
    queue.setDrainInterval(0.2f);

    queue.enqueue(WorkspaceEvent::make(WorkspaceEventType::System,  "s", "low_prio",  WorkspaceEventPriority::Low));
    queue.enqueue(WorkspaceEvent::make(WorkspaceEventType::System,  "s", "critical",  WorkspaceEventPriority::Critical));
    queue.enqueue(WorkspaceEvent::make(WorkspaceEventType::System,  "s", "normal",    WorkspaceEventPriority::Normal));

    // tick but not enough time
    queue.tick(0.1f);
    CHECK(deliveryOrder.empty());

    // tick again — enough time
    queue.tick(0.15f);
    REQUIRE(deliveryOrder.size() == 3);
    CHECK(deliveryOrder[0] == "critical");
    CHECK(deliveryOrder[1] == "normal");
    CHECK(deliveryOrder[2] == "low_prio");
}

TEST_CASE("Integration: full pipeline — notification + queue + bus", "[Phase12][Integration]") {
    WorkspaceEventBus bus;

    // Track all events
    size_t totalEvents = 0;
    bus.subscribeAll([&](const WorkspaceEvent&) { ++totalEvents; });

    // Notification bus publishes directly
    WorkspaceNotificationBus notifBus(bus);
    notifBus.info("Session", "Started");

    // Queue accumulates additional events
    WorkspaceEventQueue queue(bus);
    queue.enqueue(WorkspaceEvent::make(WorkspaceEventType::Tool, "scene", "activate"));
    queue.enqueue(WorkspaceEvent::make(WorkspaceEventType::Layout, "dock", "restore"));

    // 1 from notification
    CHECK(totalEvents == 1);

    // Drain adds 2 more
    queue.drain();
    CHECK(totalEvents == 3);

    // Stats
    CHECK(bus.totalPublished() == 3);
    CHECK(notifBus.historySize() == 1);
    CHECK(queue.totalDrained() == 2);
}
