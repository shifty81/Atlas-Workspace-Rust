#include <catch2/catch_test_macros.hpp>
#include "NF/Input/Input.h"

// ── Key state ────────────────────────────────────────────────────

TEST_CASE("InputSystem key down and up", "[Input]") {
    NF::InputSystem input;
    input.init();

    REQUIRE_FALSE(input.isKeyDown(NF::KeyCode::W));

    input.setKeyDown(NF::KeyCode::W);
    REQUIRE(input.isKeyDown(NF::KeyCode::W));

    input.setKeyUp(NF::KeyCode::W);
    REQUIRE_FALSE(input.isKeyDown(NF::KeyCode::W));

    input.shutdown();
}

TEST_CASE("InputSystem unknown key is safe", "[Input]") {
    NF::InputSystem input;
    input.init();

    REQUIRE_FALSE(input.isKeyDown(NF::KeyCode::Unknown));
    input.setKeyDown(NF::KeyCode::Unknown);
    REQUIRE_FALSE(input.isKeyDown(NF::KeyCode::Unknown));

    input.shutdown();
}

// ── Mouse ────────────────────────────────────────────────────────

TEST_CASE("InputSystem mouse position and delta", "[Input]") {
    NF::InputSystem input;
    input.init();

    input.setMousePosition(100.f, 200.f);
    REQUIRE(input.state().mouse.x == 100.f);
    REQUIRE(input.state().mouse.y == 200.f);

    input.setMousePosition(110.f, 205.f);
    REQUIRE(input.state().mouse.deltaX == 10.f);
    REQUIRE(input.state().mouse.deltaY == 5.f);

    input.shutdown();
}

TEST_CASE("InputSystem scroll delta", "[Input]") {
    NF::InputSystem input;
    input.init();

    input.setScrollDelta(3.f);
    REQUIRE(input.state().mouse.scrollDelta == 3.f);

    input.shutdown();
}

// ── Gamepad ──────────────────────────────────────────────────────

TEST_CASE("InputSystem gamepad state", "[Input]") {
    NF::InputSystem input;
    input.init();

    REQUIRE_FALSE(input.state().gamepad.connected);

    input.setGamepadConnected(true);
    REQUIRE(input.state().gamepad.connected);

    input.setGamepadAxis(0.5f, -0.3f, 1.0f, -1.0f);
    REQUIRE(input.state().gamepad.leftStickX == 0.5f);
    REQUIRE(input.state().gamepad.rightStickY == -1.0f);

    input.setGamepadTriggers(0.7f, 0.9f);
    REQUIRE(input.state().gamepad.leftTrigger == 0.7f);
    REQUIRE(input.state().gamepad.rightTrigger == 0.9f);

    input.shutdown();
}

// ── Action bindings ──────────────────────────────────────────────

TEST_CASE("InputSystem action pressed event", "[Input][Actions]") {
    NF::InputSystem input;
    input.init();

    NF::StringID jumpAction("Jump");
    input.bindAction({jumpAction, NF::KeyCode::Space});

    // Frame 1: press Space
    input.setKeyDown(NF::KeyCode::Space);
    input.update();

    auto& events = input.actionEvents();
    REQUIRE(events.size() == 1);
    REQUIRE(events[0].actionName == jumpAction);
    REQUIRE(events[0].phase == NF::ActionPhase::Pressed);

    // Frame 2: still held
    input.update();
    auto& events2 = input.actionEvents();
    REQUIRE(events2.size() == 1);
    REQUIRE(events2[0].phase == NF::ActionPhase::Held);

    // Frame 3: released
    input.setKeyUp(NF::KeyCode::Space);
    input.update();
    auto& events3 = input.actionEvents();
    REQUIRE(events3.size() == 1);
    REQUIRE(events3[0].phase == NF::ActionPhase::Released);

    // Frame 4: no events
    input.update();
    REQUIRE(input.actionEvents().empty());

    input.shutdown();
}

TEST_CASE("InputSystem isActionActive", "[Input][Actions]") {
    NF::InputSystem input;
    input.init();

    NF::StringID fire("Fire");
    input.bindAction({fire, NF::KeyCode::Mouse1});

    input.setKeyDown(NF::KeyCode::Mouse1);
    input.update();
    REQUIRE(input.isActionActive(fire));

    input.setKeyUp(NF::KeyCode::Mouse1);
    input.update();
    REQUIRE_FALSE(input.isActionActive(fire));

    input.shutdown();
}

TEST_CASE("InputSystem modifier key binding", "[Input][Actions]") {
    NF::InputSystem input;
    input.init();

    NF::StringID save("Save");
    NF::ActionBinding saveBinding;
    saveBinding.actionName = save;
    saveBinding.key = NF::KeyCode::S;
    saveBinding.ctrl = true;
    input.bindAction(saveBinding);

    // Press S without Ctrl - should NOT fire
    input.setKeyDown(NF::KeyCode::S);
    input.update();
    REQUIRE(input.actionEvents().empty());

    // Press S + LCtrl - should fire
    input.setKeyDown(NF::KeyCode::LCtrl);
    input.update();
    REQUIRE(input.isActionActive(save));

    input.shutdown();
}

TEST_CASE("InputSystem clearBindings removes all actions", "[Input][Actions]") {
    NF::InputSystem input;
    input.init();

    input.bindAction({NF::StringID("Test"), NF::KeyCode::A});
    input.setKeyDown(NF::KeyCode::A);
    input.update();
    REQUIRE_FALSE(input.actionEvents().empty());

    input.clearBindings();
    input.update();
    REQUIRE(input.actionEvents().empty());

    input.shutdown();
}

TEST_CASE("InputSystem multiple bindings", "[Input][Actions]") {
    NF::InputSystem input;
    input.init();

    NF::StringID moveForward("MoveForward");
    NF::StringID jump("Jump");
    input.bindAction({moveForward, NF::KeyCode::W});
    input.bindAction({jump, NF::KeyCode::Space});

    input.setKeyDown(NF::KeyCode::W);
    input.setKeyDown(NF::KeyCode::Space);
    input.update();

    REQUIRE(input.isActionActive(moveForward));
    REQUIRE(input.isActionActive(jump));
    REQUIRE(input.actionEvents().size() == 2);

    input.shutdown();
}
