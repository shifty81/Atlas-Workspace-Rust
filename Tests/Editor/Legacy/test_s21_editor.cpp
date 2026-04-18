#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// --- EditorEventPriority names ---

TEST_CASE("EditorEventPriority names cover all 8 values", "[Editor][S21]") {
    REQUIRE(std::string(editorEventPriorityName(EditorEventPriority::Lowest))   == "Lowest");
    REQUIRE(std::string(editorEventPriorityName(EditorEventPriority::Low))      == "Low");
    REQUIRE(std::string(editorEventPriorityName(EditorEventPriority::Normal))   == "Normal");
    REQUIRE(std::string(editorEventPriorityName(EditorEventPriority::High))     == "High");
    REQUIRE(std::string(editorEventPriorityName(EditorEventPriority::Highest))  == "Highest");
    REQUIRE(std::string(editorEventPriorityName(EditorEventPriority::System))   == "System");
    REQUIRE(std::string(editorEventPriorityName(EditorEventPriority::Critical)) == "Critical");
    REQUIRE(std::string(editorEventPriorityName(EditorEventPriority::Realtime)) == "Realtime");
}

// --- EditorBusEvent predicates ---

TEST_CASE("EditorBusEvent consume + isConsumed", "[Editor][S21]") {
    EditorBusEvent ev;
    REQUIRE_FALSE(ev.isConsumed());
    ev.consume();
    REQUIRE(ev.isConsumed());
}

TEST_CASE("EditorBusEvent isHighPrio at High+", "[Editor][S21]") {
    EditorBusEvent ev;
    ev.priority = EditorEventPriority::Normal;
    REQUIRE_FALSE(ev.isHighPrio());
    ev.priority = EditorEventPriority::High;
    REQUIRE(ev.isHighPrio());
    ev.priority = EditorEventPriority::Realtime;
    REQUIRE(ev.isHighPrio());
}

TEST_CASE("EditorBusEvent isCritical at Critical+", "[Editor][S21]") {
    EditorBusEvent ev;
    ev.priority = EditorEventPriority::Highest;
    REQUIRE_FALSE(ev.isCritical());
    ev.priority = EditorEventPriority::Critical;
    REQUIRE(ev.isCritical());
    ev.priority = EditorEventPriority::Realtime;
    REQUIRE(ev.isCritical());
}

// --- EditorEventSubscription ---

TEST_CASE("EditorEventSubscription delivers matching priority events", "[Editor][S21]") {
    size_t received = 0;
    EditorEventSubscription sub("assets", EditorEventPriority::Normal,
                                [&](const EditorBusEvent&) { received++; });

    EditorBusEvent low;  low.topic  = "assets"; low.priority  = EditorEventPriority::Low;
    EditorBusEvent norm; norm.topic = "assets"; norm.priority = EditorEventPriority::Normal;
    EditorBusEvent high; high.topic = "assets"; high.priority = EditorEventPriority::High;

    sub.deliver(low);   // below minPriority — rejected
    sub.deliver(norm);  // at minPriority — accepted
    sub.deliver(high);  // above minPriority — accepted

    REQUIRE(received == 2);
    REQUIRE(sub.callCount() == 2);
}

TEST_CASE("EditorEventSubscription cancel stops deliveries", "[Editor][S21]") {
    size_t count = 0;
    EditorEventSubscription sub("topic", EditorEventPriority::Lowest,
                                [&](const EditorBusEvent&) { count++; });
    sub.cancel();
    REQUIRE_FALSE(sub.isActive());

    EditorBusEvent ev; ev.topic = "topic";
    sub.deliver(ev);
    REQUIRE(count == 0);
}

// --- EditorEventBus ---

TEST_CASE("EditorEventBus initial state is Idle with empty queue", "[Editor][S21]") {
    EditorEventBus bus;
    REQUIRE(bus.state() == EditorBusState::Idle);
    REQUIRE(bus.queueSize() == 0);
    REQUIRE(bus.subscriptionCount() == 0);
}

TEST_CASE("EditorEventBus subscribe + post + flush routing", "[Editor][S21]") {
    EditorEventBus bus;
    std::vector<std::string> received;

    bus.subscribe("log", EditorEventPriority::Lowest,
                  [&](const EditorBusEvent& ev) { received.push_back(ev.payload); });

    EditorBusEvent ev1; ev1.topic = "log"; ev1.payload = "A";
    EditorBusEvent ev2; ev2.topic = "log"; ev2.payload = "B";
    EditorBusEvent ev3; ev3.topic = "other"; ev3.payload = "C"; // different topic

    bus.post(ev1); bus.post(ev2); bus.post(ev3);

    REQUIRE(bus.queueSize() == 3);
    bus.flush();
    REQUIRE(bus.queueSize() == 0);
    REQUIRE(received.size() == 2);
    REQUIRE(received[0] == "A");
    REQUIRE(received[1] == "B");
}

TEST_CASE("EditorEventBus wildcard '*' subscription receives all topics", "[Editor][S21]") {
    EditorEventBus bus;
    size_t count = 0;
    bus.subscribe("*", EditorEventPriority::Lowest, [&](const EditorBusEvent&) { count++; });

    EditorBusEvent a; a.topic = "assets";
    EditorBusEvent b; b.topic = "scene";
    EditorBusEvent c; c.topic = "editor";
    bus.post(a); bus.post(b); bus.post(c);
    bus.flush();

    REQUIRE(count == 3);
}

TEST_CASE("EditorEventBus suspend blocks post and flush", "[Editor][S21]") {
    EditorEventBus bus;
    size_t count = 0;
    bus.subscribe("t", EditorEventPriority::Lowest, [&](const EditorBusEvent&) { count++; });

    bus.suspend();
    REQUIRE(bus.isSuspended());

    EditorBusEvent ev; ev.topic = "t";
    REQUIRE_FALSE(bus.post(ev));
    bus.flush();
    REQUIRE(count == 0);
}

TEST_CASE("EditorEventBus resume after suspend allows posting again", "[Editor][S21]") {
    EditorEventBus bus;
    size_t count = 0;
    bus.subscribe("t", EditorEventPriority::Lowest, [&](const EditorBusEvent&) { count++; });

    bus.suspend();
    bus.resume();
    REQUIRE_FALSE(bus.isSuspended());

    EditorBusEvent ev; ev.topic = "t";
    REQUIRE(bus.post(ev));
    bus.flush();
    REQUIRE(count == 1);
}

TEST_CASE("EditorEventBus clearQueue empties pending events", "[Editor][S21]") {
    EditorEventBus bus;
    EditorBusEvent ev; ev.topic = "t";
    bus.post(ev); bus.post(ev);
    REQUIRE(bus.queueSize() == 2);
    bus.clearQueue();
    REQUIRE(bus.queueSize() == 0);
}

TEST_CASE("EditorEventBus flush returns total dispatch count", "[Editor][S21]") {
    EditorEventBus bus;
    bus.subscribe("x", EditorEventPriority::Lowest, [](const EditorBusEvent&) {});
    bus.subscribe("x", EditorEventPriority::Lowest, [](const EditorBusEvent&) {});

    EditorBusEvent ev; ev.topic = "x";
    bus.post(ev); bus.post(ev);

    // 2 events × 2 subscribers = 4 dispatches
    REQUIRE(bus.flush() == 4);
}
