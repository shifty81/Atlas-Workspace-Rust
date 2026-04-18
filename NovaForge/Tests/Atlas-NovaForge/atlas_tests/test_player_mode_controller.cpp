/**
 * Tests for PlayerModeController (Phase 23 — Tri-Modal Gameplay):
 *   - PlayerMode enum and name helpers
 *   - IPlayerMode default ownership
 *   - PlayerModeController: registration, transition, tick, callbacks
 */

#include <cassert>
#include <string>
#include <vector>
#include "../cpp_client/include/gameplay/player_mode_controller.h"

using namespace atlas::gameplay;

// ── Minimal concrete mode for testing ────────────────────────────────────────

class TestMode : public IPlayerMode {
public:
    explicit TestMode(PlayerMode m) : m_mode(m) {}

    void OnEnter() override { ++enterCount; }
    void OnExit()  override { ++exitCount;  }
    void Tick(float dt) override { totalDt += dt; ++tickCount; }

    PlayerMode    Mode()      const override { return m_mode; }
    ModeOwnership Ownership() const override {
        return DefaultOwnership(m_mode);
    }

    int   enterCount = 0;
    int   exitCount  = 0;
    int   tickCount  = 0;
    float totalDt    = 0.0f;

private:
    PlayerMode m_mode;
};

// ── PlayerMode enum tests ─────────────────────────────────────────────────────

void test_player_mode_names() {
    assert(std::string(PlayerModeName(PlayerMode::FPS))           == "FPS");
    assert(std::string(PlayerModeName(PlayerMode::FLIGHT))        == "Flight");
    assert(std::string(PlayerModeName(PlayerMode::FLEET_COMMAND)) == "Fleet Command");
}

void test_default_ownership_fps() {
    auto o = DefaultOwnership(PlayerMode::FPS);
    assert(o.ownsInput);
    assert(o.ownsCamera);
    assert(o.ownsUIStack);
    assert(o.timeScale == 1.0f);
    assert(o.playerBodyPhysical == true);
}

void test_default_ownership_flight() {
    auto o = DefaultOwnership(PlayerMode::FLIGHT);
    assert(o.timeScale == 1.0f);
    assert(o.playerBodyPhysical == false);
}

void test_default_ownership_fleet_command() {
    auto o = DefaultOwnership(PlayerMode::FLEET_COMMAND);
    assert(o.timeScale == 0.15f);
    assert(o.playerBodyPhysical == false);
}

// ── PlayerModeController tests ────────────────────────────────────────────────

void test_controller_starts_at_flight() {
    PlayerModeController ctrl;
    assert(ctrl.CurrentMode() == PlayerMode::FLIGHT);
}

void test_controller_has_mode_false_before_register() {
    PlayerModeController ctrl;
    assert(!ctrl.HasMode(PlayerMode::FPS));
}

void test_controller_register_and_has_mode() {
    PlayerModeController ctrl;
    ctrl.RegisterMode(std::make_unique<TestMode>(PlayerMode::FPS));
    assert(ctrl.HasMode(PlayerMode::FPS));
    assert(!ctrl.HasMode(PlayerMode::FLIGHT));
}

void test_controller_transition_calls_enter() {
    PlayerModeController ctrl;
    auto* raw = new TestMode(PlayerMode::FPS);
    ctrl.RegisterMode(std::unique_ptr<TestMode>(raw));

    bool ok = ctrl.TransitionTo(PlayerMode::FPS);
    assert(ok);
    assert(raw->enterCount == 1);
    assert(raw->exitCount  == 0);
    assert(ctrl.CurrentMode() == PlayerMode::FPS);
}

void test_controller_transition_calls_exit_on_prev() {
    PlayerModeController ctrl;
    auto* fps    = new TestMode(PlayerMode::FPS);
    auto* flight = new TestMode(PlayerMode::FLIGHT);
    ctrl.RegisterMode(std::unique_ptr<TestMode>(fps));
    ctrl.RegisterMode(std::unique_ptr<TestMode>(flight));

    ctrl.TransitionTo(PlayerMode::FPS);
    assert(fps->enterCount == 1);

    ctrl.TransitionTo(PlayerMode::FLIGHT);
    assert(fps->exitCount    == 1);
    assert(flight->enterCount == 1);
    assert(ctrl.CurrentMode() == PlayerMode::FLIGHT);
}

void test_controller_same_mode_no_retransition() {
    PlayerModeController ctrl;
    auto* fps = new TestMode(PlayerMode::FPS);
    ctrl.RegisterMode(std::unique_ptr<TestMode>(fps));

    ctrl.TransitionTo(PlayerMode::FPS);
    assert(fps->enterCount == 1);

    bool again = ctrl.TransitionTo(PlayerMode::FPS);
    assert(!again);
    assert(fps->enterCount == 1);  // not called again
}

void test_controller_transition_to_unregistered_fails() {
    PlayerModeController ctrl;
    bool ok = ctrl.TransitionTo(PlayerMode::FPS);
    assert(!ok);
}

void test_controller_tick_dispatches_to_active_mode() {
    PlayerModeController ctrl;
    auto* fps = new TestMode(PlayerMode::FPS);
    ctrl.RegisterMode(std::unique_ptr<TestMode>(fps));
    ctrl.TransitionTo(PlayerMode::FPS);

    ctrl.Tick(0.016f);
    ctrl.Tick(0.016f);
    assert(fps->tickCount == 2);
}

void test_controller_tick_before_transition_does_nothing() {
    PlayerModeController ctrl;
    auto* fps = new TestMode(PlayerMode::FPS);
    ctrl.RegisterMode(std::unique_ptr<TestMode>(fps));
    ctrl.Tick(0.016f);  // no active mode yet
    assert(fps->tickCount == 0);
}

void test_controller_transition_callback_fired() {
    PlayerModeController ctrl;
    auto* fps    = new TestMode(PlayerMode::FPS);
    auto* flight = new TestMode(PlayerMode::FLIGHT);
    ctrl.RegisterMode(std::unique_ptr<TestMode>(fps));
    ctrl.RegisterMode(std::unique_ptr<TestMode>(flight));

    std::vector<ModeTransition> fired;
    ctrl.SetTransitionCallback([&](const ModeTransition& t) {
        fired.push_back(t);
    });

    ctrl.TransitionTo(PlayerMode::FPS);
    ctrl.TransitionTo(PlayerMode::FLIGHT);

    assert(fired.size() == 1);  // only second transition fires callback (first has no prev mode)
    assert(fired[0].from == PlayerMode::FPS);
    assert(fired[0].to   == PlayerMode::FLIGHT);
}

void test_controller_time_scale_fleet_command() {
    PlayerModeController ctrl;
    ctrl.RegisterMode(std::make_unique<TestMode>(PlayerMode::FLEET_COMMAND));
    ctrl.TransitionTo(PlayerMode::FLEET_COMMAND);
    assert(ctrl.CurrentTimeScale() == 0.15f);
}

void test_controller_transition_count() {
    PlayerModeController ctrl;
    ctrl.RegisterMode(std::make_unique<TestMode>(PlayerMode::FPS));
    ctrl.RegisterMode(std::make_unique<TestMode>(PlayerMode::FLIGHT));

    ctrl.TransitionTo(PlayerMode::FPS);
    ctrl.TransitionTo(PlayerMode::FLIGHT);
    assert(ctrl.TransitionCount() == 2);
}

void test_controller_not_transitioning_after_done() {
    PlayerModeController ctrl;
    ctrl.RegisterMode(std::make_unique<TestMode>(PlayerMode::FPS));
    ctrl.TransitionTo(PlayerMode::FPS);
    assert(!ctrl.IsTransitioning());
}
