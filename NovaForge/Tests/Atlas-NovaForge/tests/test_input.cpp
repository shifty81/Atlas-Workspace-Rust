#include "../engine/input/InputManager.h"
#include <iostream>
#include <cassert>

using namespace atlas::input;

void test_input_bind_action() {
    InputManager input;
    input.Init();

    input.BindAction(InputAction::MoveForward, InputDevice::Keyboard, 87, "W");
    assert(input.HasBinding(InputAction::MoveForward));
    assert(input.BindingCount() == 1);

    auto* binding = input.GetBinding(InputAction::MoveForward);
    assert(binding != nullptr);
    assert(binding->keyCode == 87);
    assert(binding->name == "W");

    std::cout << "[PASS] test_input_bind_action" << std::endl;
}

void test_input_unbind_action() {
    InputManager input;
    input.Init();

    input.BindAction(InputAction::Jump, InputDevice::Keyboard, 32, "Space");
    assert(input.HasBinding(InputAction::Jump));

    input.UnbindAction(InputAction::Jump);
    assert(!input.HasBinding(InputAction::Jump));
    assert(input.BindingCount() == 0);

    std::cout << "[PASS] test_input_unbind_action" << std::endl;
}

void test_input_press_release() {
    InputManager input;
    input.Init();

    input.BindAction(InputAction::Jump, InputDevice::Keyboard, 32, "Space");

    input.InjectPress(InputAction::Jump);
    assert(input.IsPressed(InputAction::Jump));
    assert(input.IsHeld(InputAction::Jump));

    input.Update();
    assert(!input.IsPressed(InputAction::Jump));
    assert(input.IsHeld(InputAction::Jump));

    input.InjectRelease(InputAction::Jump);
    assert(!input.IsHeld(InputAction::Jump));

    std::cout << "[PASS] test_input_press_release" << std::endl;
}

void test_input_axis() {
    InputManager input;
    input.Init();

    input.BindAction(InputAction::MoveForward, InputDevice::Gamepad, 0, "LeftStickY");

    input.InjectAxis(InputAction::MoveForward, 0.75f);
    assert(input.GetAxis(InputAction::MoveForward) > 0.7f);
    assert(input.GetAxis(InputAction::MoveForward) < 0.8f);

    std::cout << "[PASS] test_input_axis" << std::endl;
}

void test_input_callback() {
    InputManager input;
    input.Init();

    input.BindAction(InputAction::Interact, InputDevice::Keyboard, 69, "E");

    bool callbackFired = false;
    input.SetCallback(InputAction::Interact, [&](const InputState& state) {
        callbackFired = true;
        (void)state;
    });

    input.InjectPress(InputAction::Interact);
    input.Update();
    assert(callbackFired);

    std::cout << "[PASS] test_input_callback" << std::endl;
}

void test_input_unbound_action() {
    InputManager input;
    input.Init();

    assert(!input.IsPressed(InputAction::Sprint));
    assert(!input.IsHeld(InputAction::Sprint));
    assert(input.GetAxis(InputAction::Sprint) == 0.0f);

    std::cout << "[PASS] test_input_unbound_action" << std::endl;
}
