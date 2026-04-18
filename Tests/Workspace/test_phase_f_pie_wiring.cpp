// Tests/Workspace/test_phase_f_pie_wiring.cpp
//
// Phase F — PIE wiring: toolbar controller + console output bridge
//
// F.PIE_TOOLBAR — PIEToolbarController
//   - canPlay / canPause / canStop / canStep in Stopped state
//   - pressPlay from Stopped starts session (→ Playing)
//   - pressPlay from Paused resumes session (→ Playing)
//   - pressPlay while Playing is no-op (returns false)
//   - pressPause while Playing → Paused
//   - pressPause while Stopped is no-op
//   - pressStop while Playing → Stopped
//   - pressStop while Paused → Stopped
//   - pressStop while Stopped is no-op
//   - pressStep while Paused increments PIE tick
//   - pressStep while Playing is no-op
//   - isReadOnly false when Stopped, true when Playing or Paused
//   - pieState passthrough
//   - onAction callback fires for each successful action
//   - actionCount tracks successful presses
//   - PIEToolbarAction names
//
// F.PIE_CONSOLE — PIEConsoleBridge
//   - default state: not bound, lineCount == 0
//   - bind wires stdout → ConsolePanel messages
//   - pushStdoutLine after bind appends to ConsolePanel
//   - lineCount increments per routed line
//   - multiple lines all arrive in ConsolePanel
//   - unbind disconnects routing (future pushStdoutLine does not reach panel)
//   - rebind to different objects replaces previous binding
//   - messages appear as ConsoleMessageLevel::Info

#include <catch2/catch_test_macros.hpp>

#include "NF/Editor/PIEToolbarController.h"
#include "NF/Editor/PIEConsoleBridge.h"

using namespace NF;

// ─────────────────────────────────────────────────────────────────────────────
// PIEToolbarAction names
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("PIEToolbarAction names are correct", "[phase_f][pie_toolbar][names]") {
    CHECK(std::string(pieToolbarActionName(PIEToolbarAction::Play))  == "Play");
    CHECK(std::string(pieToolbarActionName(PIEToolbarAction::Pause)) == "Pause");
    CHECK(std::string(pieToolbarActionName(PIEToolbarAction::Stop))  == "Stop");
    CHECK(std::string(pieToolbarActionName(PIEToolbarAction::Step))  == "Step");
}

// ─────────────────────────────────────────────────────────────────────────────
// PIEToolbarController — button enable states
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("PIEToolbarController: canPlay/Stop true, canPause/Step false when Stopped",
          "[phase_f][pie_toolbar][enable]") {
    PIEService svc;
    PIEToolbarController tb(svc);

    REQUIRE(svc.isStopped());
    CHECK(tb.canPlay());
    CHECK(!tb.canPause());
    CHECK(!tb.canStop());
    CHECK(!tb.canStep());
}

TEST_CASE("PIEToolbarController: canPause/Stop true, canPlay/Step false when Playing",
          "[phase_f][pie_toolbar][enable]") {
    PIEService svc;
    PIEToolbarController tb(svc);
    svc.enter();

    CHECK(!tb.canPlay());
    CHECK(tb.canPause());
    CHECK(tb.canStop());
    CHECK(!tb.canStep());
}

TEST_CASE("PIEToolbarController: canPlay/Stop/Step true, canPause false when Paused",
          "[phase_f][pie_toolbar][enable]") {
    PIEService svc;
    PIEToolbarController tb(svc);
    svc.enter();
    svc.pause();

    CHECK(tb.canPlay());
    CHECK(!tb.canPause());
    CHECK(tb.canStop());
    CHECK(tb.canStep());
}

// ─────────────────────────────────────────────────────────────────────────────
// PIEToolbarController — pressPlay
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("PIEToolbarController: pressPlay from Stopped enters session",
          "[phase_f][pie_toolbar][play]") {
    PIEService svc;
    PIEToolbarController tb(svc);

    bool acted = tb.pressPlay();
    CHECK(acted);
    CHECK(svc.isPlaying());
}

TEST_CASE("PIEToolbarController: pressPlay from Paused resumes session",
          "[phase_f][pie_toolbar][play]") {
    PIEService svc;
    PIEToolbarController tb(svc);
    svc.enter();
    svc.pause();
    REQUIRE(svc.isPaused());

    bool acted = tb.pressPlay();
    CHECK(acted);
    CHECK(svc.isPlaying());
}

TEST_CASE("PIEToolbarController: pressPlay while Playing is no-op",
          "[phase_f][pie_toolbar][play]") {
    PIEService svc;
    PIEToolbarController tb(svc);
    svc.enter();
    REQUIRE(svc.isPlaying());

    bool acted = tb.pressPlay();
    CHECK(!acted);
    CHECK(svc.isPlaying());
}

// ─────────────────────────────────────────────────────────────────────────────
// PIEToolbarController — pressPause
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("PIEToolbarController: pressPause while Playing transitions to Paused",
          "[phase_f][pie_toolbar][pause]") {
    PIEService svc;
    PIEToolbarController tb(svc);
    svc.enter();

    bool acted = tb.pressPause();
    CHECK(acted);
    CHECK(svc.isPaused());
}

TEST_CASE("PIEToolbarController: pressPause while Stopped is no-op",
          "[phase_f][pie_toolbar][pause]") {
    PIEService svc;
    PIEToolbarController tb(svc);

    bool acted = tb.pressPause();
    CHECK(!acted);
    CHECK(svc.isStopped());
}

// ─────────────────────────────────────────────────────────────────────────────
// PIEToolbarController — pressStop
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("PIEToolbarController: pressStop while Playing stops session",
          "[phase_f][pie_toolbar][stop]") {
    PIEService svc;
    PIEToolbarController tb(svc);
    svc.enter();

    bool acted = tb.pressStop();
    CHECK(acted);
    CHECK(svc.isStopped());
}

TEST_CASE("PIEToolbarController: pressStop while Paused stops session",
          "[phase_f][pie_toolbar][stop]") {
    PIEService svc;
    PIEToolbarController tb(svc);
    svc.enter();
    svc.pause();

    bool acted = tb.pressStop();
    CHECK(acted);
    CHECK(svc.isStopped());
}

TEST_CASE("PIEToolbarController: pressStop while Stopped is no-op",
          "[phase_f][pie_toolbar][stop]") {
    PIEService svc;
    PIEToolbarController tb(svc);

    bool acted = tb.pressStop();
    CHECK(!acted);
}

// ─────────────────────────────────────────────────────────────────────────────
// PIEToolbarController — pressStep
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("PIEToolbarController: pressStep while Paused increments tick",
          "[phase_f][pie_toolbar][step]") {
    PIEService svc;
    PIEToolbarController tb(svc);
    svc.enter();
    svc.pause();
    uint32_t tickBefore = svc.counters().tickIndex;

    bool acted = tb.pressStep();
    CHECK(acted);
    CHECK(svc.counters().tickIndex == tickBefore + 1);
}

TEST_CASE("PIEToolbarController: pressStep while Playing is no-op",
          "[phase_f][pie_toolbar][step]") {
    PIEService svc;
    PIEToolbarController tb(svc);
    svc.enter();

    bool acted = tb.pressStep();
    CHECK(!acted);
}

// ─────────────────────────────────────────────────────────────────────────────
// PIEToolbarController — isReadOnly
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("PIEToolbarController: isReadOnly false when Stopped",
          "[phase_f][pie_toolbar][readonly]") {
    PIEService svc;
    PIEToolbarController tb(svc);
    CHECK(!tb.isReadOnly());
}

TEST_CASE("PIEToolbarController: isReadOnly true when Playing",
          "[phase_f][pie_toolbar][readonly]") {
    PIEService svc;
    PIEToolbarController tb(svc);
    svc.enter();
    CHECK(tb.isReadOnly());
}

TEST_CASE("PIEToolbarController: isReadOnly true when Paused",
          "[phase_f][pie_toolbar][readonly]") {
    PIEService svc;
    PIEToolbarController tb(svc);
    svc.enter();
    svc.pause();
    CHECK(tb.isReadOnly());
}

TEST_CASE("PIEToolbarController: isReadOnly false again after Stop",
          "[phase_f][pie_toolbar][readonly]") {
    PIEService svc;
    PIEToolbarController tb(svc);
    svc.enter();
    svc.exit();
    CHECK(!tb.isReadOnly());
}

// ─────────────────────────────────────────────────────────────────────────────
// PIEToolbarController — pieState passthrough
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("PIEToolbarController: pieState passthrough matches PIEService",
          "[phase_f][pie_toolbar][state]") {
    PIEService svc;
    PIEToolbarController tb(svc);

    CHECK(tb.pieState() == PIEState::Stopped);
    svc.enter();
    CHECK(tb.pieState() == PIEState::Playing);
    svc.pause();
    CHECK(tb.pieState() == PIEState::Paused);
    svc.exit();
    CHECK(tb.pieState() == PIEState::Stopped);
}

// ─────────────────────────────────────────────────────────────────────────────
// PIEToolbarController — callbacks
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("PIEToolbarController: onAction fires on each successful press",
          "[phase_f][pie_toolbar][callback]") {
    PIEService svc;
    PIEToolbarController tb(svc);

    std::vector<PIEToolbarAction> actions;
    tb.setOnAction([&](PIEToolbarAction a) { actions.push_back(a); });

    tb.pressPlay();   // Stopped → Playing
    tb.pressPause();  // Playing → Paused
    tb.pressStep();   // Paused step
    tb.pressPlay();   // Paused → Playing
    tb.pressStop();   // Playing → Stopped

    REQUIRE(actions.size() == 5);
    CHECK(actions[0] == PIEToolbarAction::Play);
    CHECK(actions[1] == PIEToolbarAction::Pause);
    CHECK(actions[2] == PIEToolbarAction::Step);
    CHECK(actions[3] == PIEToolbarAction::Play);
    CHECK(actions[4] == PIEToolbarAction::Stop);
}

TEST_CASE("PIEToolbarController: onAction not fired on no-op presses",
          "[phase_f][pie_toolbar][callback]") {
    PIEService svc;
    PIEToolbarController tb(svc);

    uint32_t fires = 0;
    tb.setOnAction([&](PIEToolbarAction) { ++fires; });

    tb.pressPause(); // Stopped → no-op
    tb.pressStop();  // Stopped → no-op
    tb.pressStep();  // Stopped → no-op
    CHECK(fires == 0);
}

TEST_CASE("PIEToolbarController: actionCount tracks successful presses",
          "[phase_f][pie_toolbar][callback]") {
    PIEService svc;
    PIEToolbarController tb(svc);

    CHECK(tb.actionCount() == 0);
    tb.pressPlay();
    CHECK(tb.actionCount() == 1);
    tb.pressPause();
    CHECK(tb.actionCount() == 2);
    tb.pressPlay();
    CHECK(tb.actionCount() == 3);
    tb.pressStop();
    CHECK(tb.actionCount() == 4);

    // No-ops do not increment
    tb.pressStop();
    CHECK(tb.actionCount() == 4);
}

// ─────────────────────────────────────────────────────────────────────────────
// PIEConsoleBridge — default state
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("PIEConsoleBridge: default state is unbound with zero lines",
          "[phase_f][pie_console]") {
    PIEConsoleBridge bridge;
    CHECK(!bridge.isBound());
    CHECK(bridge.lineCount() == 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// PIEConsoleBridge — bind / route
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("PIEConsoleBridge: bind marks bridge as bound",
          "[phase_f][pie_console]") {
    PIEExternalLaunch launch;
    ConsolePanel console;
    PIEConsoleBridge bridge;

    bridge.bind(launch, console);
    CHECK(bridge.isBound());
}

TEST_CASE("PIEConsoleBridge: pushStdoutLine after bind appends to ConsolePanel",
          "[phase_f][pie_console]") {
    PIEExternalLaunch launch;
    ConsolePanel console;
    PIEConsoleBridge bridge;
    bridge.bind(launch, console);

    launch.pushStdoutLine("Hello from game process");

    REQUIRE(console.messageCount() == 1);
    CHECK(console.messages()[0].text == "Hello from game process");
    CHECK(console.messages()[0].level == ConsoleMessageLevel::Info);
}

TEST_CASE("PIEConsoleBridge: lineCount increments per routed line",
          "[phase_f][pie_console]") {
    PIEExternalLaunch launch;
    ConsolePanel console;
    PIEConsoleBridge bridge;
    bridge.bind(launch, console);

    launch.pushStdoutLine("line 1");
    launch.pushStdoutLine("line 2");
    launch.pushStdoutLine("line 3");

    CHECK(bridge.lineCount() == 3);
    CHECK(console.messageCount() == 3);
}

TEST_CASE("PIEConsoleBridge: multiple lines all arrive in ConsolePanel in order",
          "[phase_f][pie_console]") {
    PIEExternalLaunch launch;
    ConsolePanel console;
    PIEConsoleBridge bridge;
    bridge.bind(launch, console);

    launch.pushStdoutLine("alpha");
    launch.pushStdoutLine("beta");
    launch.pushStdoutLine("gamma");

    const auto& msgs = console.messages();
    REQUIRE(msgs.size() == 3);
    CHECK(msgs[0].text == "alpha");
    CHECK(msgs[1].text == "beta");
    CHECK(msgs[2].text == "gamma");
}

// ─────────────────────────────────────────────────────────────────────────────
// PIEConsoleBridge — unbind
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("PIEConsoleBridge: unbind disconnects routing",
          "[phase_f][pie_console]") {
    PIEExternalLaunch launch;
    ConsolePanel console;
    PIEConsoleBridge bridge;
    bridge.bind(launch, console);

    launch.pushStdoutLine("before unbind");
    bridge.unbind();

    launch.pushStdoutLine("after unbind");

    // Only the pre-unbind line should be present
    CHECK(console.messageCount() == 1);
    CHECK(console.messages()[0].text == "before unbind");
    CHECK(!bridge.isBound());
}

TEST_CASE("PIEConsoleBridge: rebind replaces previous binding",
          "[phase_f][pie_console]") {
    PIEExternalLaunch launch;
    ConsolePanel console1;
    ConsolePanel console2;
    PIEConsoleBridge bridge;

    bridge.bind(launch, console1);
    launch.pushStdoutLine("to console1");

    bridge.bind(launch, console2);
    launch.pushStdoutLine("to console2");

    CHECK(console1.messageCount() == 1);
    CHECK(console2.messageCount() == 1);
    CHECK(bridge.isBound());
    // lineCount reset on rebind
    CHECK(bridge.lineCount() == 1);
}
