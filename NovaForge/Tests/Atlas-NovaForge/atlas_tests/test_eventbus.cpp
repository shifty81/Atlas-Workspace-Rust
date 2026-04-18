#include <cassert>
#include <string>
#include <vector>
#include <atomic>
#include "../engine/core/EventBus.h"

// ── Helpers ──────────────────────────────────────────────────────────

static void ok(const char* name) {
    // Placeholder — real tracking happens via RUN_TEST() in main.cpp.
    (void)name;
}

// ── Tests ────────────────────────────────────────────────────────────

void test_eventbus_initial_state() {
    atlas::EventBus bus;
    assert(bus.SubscriptionCount() == 0);
    assert(bus.QueueSize() == 0);
    assert(bus.TotalPublished() == 0);
    ok("test_eventbus_initial_state");
}

void test_eventbus_subscribe_and_count() {
    atlas::EventBus bus;
    auto id1 = bus.Subscribe("DAMAGE", [](const atlas::Event&) {});
    auto id2 = bus.Subscribe("SHIELD_DOWN", [](const atlas::Event&) {});
    assert(id1 != 0);
    assert(id2 != 0);
    assert(id1 != id2);
    assert(bus.SubscriptionCount() == 2);
    ok("test_eventbus_subscribe_and_count");
}

void test_eventbus_subscribe_null_callback() {
    atlas::EventBus bus;
    auto id = bus.Subscribe("X", nullptr);
    assert(id == 0);
    assert(bus.SubscriptionCount() == 0);
    ok("test_eventbus_subscribe_null_callback");
}

void test_eventbus_unsubscribe() {
    atlas::EventBus bus;
    auto id = bus.Subscribe("A", [](const atlas::Event&) {});
    assert(bus.SubscriptionCount() == 1);
    bus.Unsubscribe(id);
    assert(bus.SubscriptionCount() == 0);
    ok("test_eventbus_unsubscribe");
}

void test_eventbus_unsubscribe_invalid_noop() {
    atlas::EventBus bus;
    bus.Subscribe("A", [](const atlas::Event&) {});
    bus.Unsubscribe(0);       // invalid
    bus.Unsubscribe(999);     // non-existent
    assert(bus.SubscriptionCount() == 1);
    ok("test_eventbus_unsubscribe_invalid_noop");
}

void test_eventbus_publish_invokes_callback() {
    atlas::EventBus bus;
    int called = 0;
    double receivedDamage = 0.0;
    bus.Subscribe("DAMAGE", [&](const atlas::Event& e) {
        ++called;
        receivedDamage = e.floatParam;
    });
    bus.Publish({"DAMAGE", 42, 0, 100.5});
    assert(called == 1);
    assert(receivedDamage == 100.5);
    ok("test_eventbus_publish_invokes_callback");
}

void test_eventbus_publish_only_matching_type() {
    atlas::EventBus bus;
    int damageCalls = 0;
    int healCalls = 0;
    bus.Subscribe("DAMAGE", [&](const atlas::Event&) { ++damageCalls; });
    bus.Subscribe("HEAL", [&](const atlas::Event&) { ++healCalls; });

    bus.Publish({"DAMAGE", 1});
    assert(damageCalls == 1);
    assert(healCalls == 0);

    bus.Publish({"HEAL", 2});
    assert(damageCalls == 1);
    assert(healCalls == 1);
    ok("test_eventbus_publish_only_matching_type");
}

void test_eventbus_multiple_subscribers_same_type() {
    atlas::EventBus bus;
    int a = 0, b = 0;
    bus.Subscribe("X", [&](const atlas::Event&) { ++a; });
    bus.Subscribe("X", [&](const atlas::Event&) { ++b; });
    bus.Publish({"X"});
    assert(a == 1);
    assert(b == 1);
    ok("test_eventbus_multiple_subscribers_same_type");
}

void test_eventbus_wildcard_receives_all() {
    atlas::EventBus bus;
    std::vector<std::string> received;
    bus.Subscribe("*", [&](const atlas::Event& e) {
        received.push_back(e.type);
    });
    bus.Publish({"ALPHA"});
    bus.Publish({"BETA"});
    bus.Publish({"GAMMA"});
    assert(received.size() == 3);
    assert(received[0] == "ALPHA");
    assert(received[1] == "BETA");
    assert(received[2] == "GAMMA");
    ok("test_eventbus_wildcard_receives_all");
}

void test_eventbus_enqueue_and_flush() {
    atlas::EventBus bus;
    int calls = 0;
    bus.Subscribe("TICK", [&](const atlas::Event&) { ++calls; });

    bus.Enqueue({"TICK"});
    bus.Enqueue({"TICK"});
    assert(calls == 0);        // not delivered yet
    assert(bus.QueueSize() == 2);

    bus.Flush();
    assert(calls == 2);        // now delivered
    assert(bus.QueueSize() == 0);
    ok("test_eventbus_enqueue_and_flush");
}

void test_eventbus_flush_fifo_order() {
    atlas::EventBus bus;
    std::vector<int> order;
    bus.Subscribe("SEQ", [&](const atlas::Event& e) {
        order.push_back(static_cast<int>(e.intParam));
    });
    bus.Enqueue({"SEQ", 0, 1});
    bus.Enqueue({"SEQ", 0, 2});
    bus.Enqueue({"SEQ", 0, 3});
    bus.Flush();
    assert(order.size() == 3);
    assert(order[0] == 1);
    assert(order[1] == 2);
    assert(order[2] == 3);
    ok("test_eventbus_flush_fifo_order");
}

void test_eventbus_enqueue_during_flush_deferred() {
    atlas::EventBus bus;
    int round = 0;
    bus.Subscribe("CHAIN", [&](const atlas::Event&) {
        ++round;
        if (round == 1) {
            bus.Enqueue({"CHAIN"});  // Should NOT fire in this Flush
        }
    });
    bus.Enqueue({"CHAIN"});
    bus.Flush();
    assert(round == 1);        // Only the first event fires
    assert(bus.QueueSize() == 1); // Chained event is still queued

    bus.Flush();
    assert(round == 2);        // Now the chained event fires
    assert(bus.QueueSize() == 0);
    ok("test_eventbus_enqueue_during_flush_deferred");
}

void test_eventbus_total_published() {
    atlas::EventBus bus;
    bus.Subscribe("A", [](const atlas::Event&) {});
    bus.Publish({"A"});
    bus.Publish({"A"});
    bus.Enqueue({"A"});
    bus.Flush();
    assert(bus.TotalPublished() == 3);
    ok("test_eventbus_total_published");
}

void test_eventbus_reset() {
    atlas::EventBus bus;
    bus.Subscribe("X", [](const atlas::Event&) {});
    bus.Enqueue({"X"});
    bus.Publish({"X"});
    assert(bus.SubscriptionCount() == 1);
    assert(bus.QueueSize() == 1);
    assert(bus.TotalPublished() > 0);

    bus.Reset();
    assert(bus.SubscriptionCount() == 0);
    assert(bus.QueueSize() == 0);
    assert(bus.TotalPublished() == 0);
    ok("test_eventbus_reset");
}

void test_eventbus_sender_id_and_str_param() {
    atlas::EventBus bus;
    uint32_t gotSender = 0;
    std::string gotStr;
    bus.Subscribe("MSG", [&](const atlas::Event& e) {
        gotSender = e.senderId;
        gotStr = e.strParam;
    });
    bus.Publish({"MSG", 77, 0, 0.0, "hello"});
    assert(gotSender == 77);
    assert(gotStr == "hello");
    ok("test_eventbus_sender_id_and_str_param");
}

void test_eventbus_no_subscribers_no_crash() {
    atlas::EventBus bus;
    bus.Publish({"UNKNOWN_EVENT", 1, 2, 3.0, "data"});
    bus.Enqueue({"ANOTHER"});
    bus.Flush();
    assert(bus.TotalPublished() == 2);
    ok("test_eventbus_no_subscribers_no_crash");
}

void test_eventbus_unsubscribe_wildcard() {
    atlas::EventBus bus;
    int calls = 0;
    auto id = bus.Subscribe("*", [&](const atlas::Event&) { ++calls; });
    bus.Publish({"ANY"});
    assert(calls == 1);

    bus.Unsubscribe(id);
    bus.Publish({"ANY"});
    assert(calls == 1); // no longer subscribed
    assert(bus.SubscriptionCount() == 0);
    ok("test_eventbus_unsubscribe_wildcard");
}
