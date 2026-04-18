#include "../engine/ui/UICommandBus.h"
#include <iostream>
#include <cassert>

using namespace atlas::ui;

void test_command_bus_enqueue() {
    UICommandBus bus;
    assert(bus.PendingCount() == 0);

    UICommand cmd;
    cmd.type = UICommandType::ButtonPress;
    cmd.targetWidgetId = 42;
    cmd.tick = 100;
    bus.Enqueue(cmd);

    assert(bus.PendingCount() == 1);
    std::cout << "[PASS] test_command_bus_enqueue" << std::endl;
}

void test_command_bus_drain() {
    UICommandBus bus;

    UICommand cmd1;
    cmd1.type = UICommandType::ButtonPress;
    cmd1.targetWidgetId = 1;
    bus.Enqueue(cmd1);

    UICommand cmd2;
    cmd2.type = UICommandType::TextInput;
    cmd2.targetWidgetId = 2;
    cmd2.valueString = "hello";
    bus.Enqueue(cmd2);

    std::vector<UICommand> out;
    bus.Drain(out);

    assert(out.size() == 2);
    assert(out[0].type == UICommandType::ButtonPress);
    assert(out[0].targetWidgetId == 1);
    assert(out[1].type == UICommandType::TextInput);
    assert(out[1].valueString == "hello");
    assert(bus.PendingCount() == 0);
    std::cout << "[PASS] test_command_bus_drain" << std::endl;
}

void test_command_bus_clear() {
    UICommandBus bus;

    UICommand cmd;
    cmd.type = UICommandType::ButtonPress;
    bus.Enqueue(cmd);
    bus.Enqueue(cmd);
    assert(bus.PendingCount() == 2);

    bus.Clear();
    assert(bus.PendingCount() == 0);
    std::cout << "[PASS] test_command_bus_clear" << std::endl;
}

void test_command_bus_dispatch() {
    UICommandBus bus;
    int pressCount = 0;

    bus.RegisterHandler(UICommandType::ButtonPress, [&](const UICommand& cmd) {
        pressCount++;
        assert(cmd.targetWidgetId == 10);
    });

    UICommand cmd;
    cmd.type = UICommandType::ButtonPress;
    cmd.targetWidgetId = 10;
    bus.Enqueue(cmd);
    bus.Enqueue(cmd);

    bus.Dispatch();
    assert(pressCount == 2);
    assert(bus.PendingCount() == 0);
    std::cout << "[PASS] test_command_bus_dispatch" << std::endl;
}

void test_command_bus_dispatch_unhandled() {
    UICommandBus bus;
    int callCount = 0;

    bus.RegisterHandler(UICommandType::ButtonPress, [&](const UICommand&) {
        callCount++;
    });

    UICommand cmd;
    cmd.type = UICommandType::TextInput;  // No handler registered for this
    bus.Enqueue(cmd);

    bus.Dispatch();
    assert(callCount == 0);  // Handler for ButtonPress should not fire
    std::cout << "[PASS] test_command_bus_dispatch_unhandled" << std::endl;
}

void test_command_bus_move_enqueue() {
    UICommandBus bus;

    UICommand cmd;
    cmd.type = UICommandType::TextInput;
    cmd.valueString = "test_value";
    bus.Enqueue(std::move(cmd));

    std::vector<UICommand> out;
    bus.Drain(out);
    assert(out.size() == 1);
    assert(out[0].valueString == "test_value");
    std::cout << "[PASS] test_command_bus_move_enqueue" << std::endl;
}
