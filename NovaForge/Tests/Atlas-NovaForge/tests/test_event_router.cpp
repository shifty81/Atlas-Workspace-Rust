#include "../engine/ui/UIEventRouter.h"
#include <iostream>
#include <cassert>

using namespace atlas::ui;

/// Simple test target that tracks events.
class TestTarget : public UIEventTarget {
public:
    UIRect bounds;
    int eventCount = 0;
    bool consumeEvents = true;

    TestTarget(UIRect b, int z = 0) : bounds(b) { zOrder = z; }

    bool HitTest(int32_t x, int32_t y) const override {
        return x >= bounds.x && x < bounds.x + bounds.w &&
               y >= bounds.y && y < bounds.y + bounds.h;
    }

    bool OnEvent(const UIEvent& /*event*/) override {
        eventCount++;
        return consumeEvents;
    }
};

void test_event_router_empty() {
    UIEventRouter router;
    assert(router.TargetCount() == 0);

    UIEvent e;
    e.type = UIEvent::Type::MouseDown;
    e.x = 100; e.y = 100;
    assert(!router.Dispatch(e));
    std::cout << "[PASS] test_event_router_empty" << std::endl;
}

void test_event_router_register_dispatch() {
    UIEventRouter router;
    TestTarget target({0, 0, 200, 200}, 0);
    router.Register(&target);
    assert(router.TargetCount() == 1);

    UIEvent e;
    e.type = UIEvent::Type::MouseDown;
    e.x = 50; e.y = 50;
    assert(router.Dispatch(e));
    assert(target.eventCount == 1);
    std::cout << "[PASS] test_event_router_register_dispatch" << std::endl;
}

void test_event_router_miss() {
    UIEventRouter router;
    TestTarget target({0, 0, 100, 100}, 0);
    router.Register(&target);

    UIEvent e;
    e.type = UIEvent::Type::MouseDown;
    e.x = 200; e.y = 200; // outside
    assert(!router.Dispatch(e));
    assert(target.eventCount == 0);
    std::cout << "[PASS] test_event_router_miss" << std::endl;
}

void test_event_router_z_order() {
    UIEventRouter router;
    TestTarget low({0, 0, 200, 200}, 0);
    TestTarget high({0, 0, 200, 200}, 10);

    router.Register(&low);
    router.Register(&high);

    UIEvent e;
    e.type = UIEvent::Type::MouseDown;
    e.x = 50; e.y = 50;
    router.Dispatch(e);

    // High z-order target should get the event first and consume it
    assert(high.eventCount == 1);
    assert(low.eventCount == 0);
    std::cout << "[PASS] test_event_router_z_order" << std::endl;
}

void test_event_router_unregister() {
    UIEventRouter router;
    TestTarget target({0, 0, 200, 200}, 0);
    router.Register(&target);
    assert(router.TargetCount() == 1);

    router.Unregister(&target);
    assert(router.TargetCount() == 0);

    UIEvent e;
    e.type = UIEvent::Type::MouseDown;
    e.x = 50; e.y = 50;
    assert(!router.Dispatch(e));
    std::cout << "[PASS] test_event_router_unregister" << std::endl;
}

void test_event_router_hover_tracking() {
    UIEventRouter router;
    TestTarget target({0, 0, 200, 200}, 0);
    target.consumeEvents = false; // don't consume
    router.Register(&target);

    UIEvent e;
    e.type = UIEvent::Type::MouseMove;
    e.x = 50; e.y = 50;
    router.Dispatch(e);

    assert(router.GetFocus().hovered == &target);
    std::cout << "[PASS] test_event_router_hover_tracking" << std::endl;
}

void test_event_router_clear() {
    UIEventRouter router;
    TestTarget target({0, 0, 200, 200}, 0);
    router.Register(&target);
    router.Clear();
    assert(router.TargetCount() == 0);
    std::cout << "[PASS] test_event_router_clear" << std::endl;
}
