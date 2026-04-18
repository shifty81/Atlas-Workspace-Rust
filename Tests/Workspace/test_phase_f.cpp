// Phase F — Play-In-Editor (PIE)
//
// F.1 — PIEService lifecycle
//   - Default state: Stopped
//   - enter() → Playing; double-enter is no-op
//   - exit() → Stopped; exit while Stopped is no-op
//   - pause() Playing → Paused; pause while Stopped/Paused no-op
//   - resume() Paused → Playing; resume while Playing/Stopped no-op
//   - step() while Paused increments tick; step while Playing/Stopped no-op
//   - reset() while Playing/Paused restores counters; reset while Stopped no-op
//   - callbacks: onEnter, onExit, onPause, onResume, onStep, onReset
//   - sessionId increments with each enter()
//   - sessionHistory grows after each exit()
//   - PIEState names
//
// F.1 — PIEDiagnostics
//   - pushDiagnostic records event in diagnostics list
//   - diagnosticCount / errorCount / countBySeverity
//   - kMaxDiagnosticHistory cap
//   - session record captures events on exit
//   - onDiagnostic callback invoked per event
//
// F.1 — PIEPerformanceCounters
//   - tickFrame updates fps / entityCount / drawCallCount / memoryBytes
//   - tickFrame is no-op while Paused or Stopped
//   - tickIndex increments per tickFrame call
//
// F.2 — PIEInputRouter
//   - Default mode: Editor
//   - routeToGame / routeToEditor switches mode
//   - isGameMode / isEditorMode
//   - processKey dispatches to the correct sink based on mode
//   - processMouseButton dispatches to correct sink
//   - processMouseMove dispatches to correct sink
//   - isExitKey detects escape (0x1B) in game mode
//   - modeSwitchCount / keyEventCount / mouseEventCount / mouseMoveCount
//   - onModeChange callback invoked on each switch
//
// F.3 — PIEExternalLaunch
//   - default state Idle
//   - hasValidConfig false without config
//   - setConfig validates executable + project path
//   - launch() Idle → Running (stub)
//   - launch() with invalid config returns failure
//   - launch() while Running returns failure
//   - terminate() Running → Exited
//   - simulateExit(0) → Exited; simulateExit(nonzero) → Crashed
//   - launchCount increments
//   - processId is nonzero after launch
//   - lastExitCode reflects exit code
//   - stdoutLines / stdoutLineCount / pushStdoutLine / onStdoutLine callback
//   - onLaunched / onExited callbacks
//   - PIEProcessState names

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "NF/Editor/PIEService.h"
#include "NF/Editor/PIEInputRouter.h"
#include "NF/Editor/PIEExternalLaunch.h"

#include <string>
#include <vector>

using namespace NF;
using Catch::Approx;

// ── F.1 — PIEState names ─────────────────────────────────────────────────────

TEST_CASE("PIEState names are correct", "[phase_f][f1][pie_state]") {
    REQUIRE(std::string(pieStateName(PIEState::Stopped)) == "Stopped");
    REQUIRE(std::string(pieStateName(PIEState::Playing)) == "Playing");
    REQUIRE(std::string(pieStateName(PIEState::Paused))  == "Paused");
}

// ── F.1 — PIEService lifecycle ────────────────────────────────────────────────

TEST_CASE("PIEService default state is Stopped", "[phase_f][f1][lifecycle]") {
    PIEService svc;
    REQUIRE(svc.isStopped());
    REQUIRE_FALSE(svc.isPlaying());
    REQUIRE_FALSE(svc.isPaused());
    REQUIRE(svc.state() == PIEState::Stopped);
    REQUIRE(svc.sessionId() == 0);
}

TEST_CASE("PIEService enter transitions to Playing", "[phase_f][f1][lifecycle]") {
    PIEService svc;
    REQUIRE(svc.enter());
    REQUIRE(svc.isPlaying());
    REQUIRE(svc.sessionId() == 1);
}

TEST_CASE("PIEService enter while already Playing is no-op", "[phase_f][f1][lifecycle]") {
    PIEService svc;
    svc.enter();
    REQUIRE_FALSE(svc.enter());
    REQUIRE(svc.isPlaying());
    REQUIRE(svc.sessionId() == 1);
}

TEST_CASE("PIEService enter while Paused is no-op", "[phase_f][f1][lifecycle]") {
    PIEService svc;
    svc.enter();
    svc.pause();
    REQUIRE_FALSE(svc.enter());
    REQUIRE(svc.isPaused());
}

TEST_CASE("PIEService exit from Playing transitions to Stopped", "[phase_f][f1][lifecycle]") {
    PIEService svc;
    svc.enter();
    REQUIRE(svc.exit());
    REQUIRE(svc.isStopped());
}

TEST_CASE("PIEService exit while Stopped is no-op", "[phase_f][f1][lifecycle]") {
    PIEService svc;
    REQUIRE_FALSE(svc.exit());
    REQUIRE(svc.isStopped());
}

TEST_CASE("PIEService exit from Paused transitions to Stopped", "[phase_f][f1][lifecycle]") {
    PIEService svc;
    svc.enter();
    svc.pause();
    REQUIRE(svc.exit());
    REQUIRE(svc.isStopped());
}

TEST_CASE("PIEService pause from Playing transitions to Paused", "[phase_f][f1][lifecycle]") {
    PIEService svc;
    svc.enter();
    REQUIRE(svc.pause());
    REQUIRE(svc.isPaused());
}

TEST_CASE("PIEService pause while Stopped is no-op", "[phase_f][f1][lifecycle]") {
    PIEService svc;
    REQUIRE_FALSE(svc.pause());
}

TEST_CASE("PIEService pause while already Paused is no-op", "[phase_f][f1][lifecycle]") {
    PIEService svc;
    svc.enter();
    svc.pause();
    REQUIRE_FALSE(svc.pause());
    REQUIRE(svc.isPaused());
}

TEST_CASE("PIEService resume from Paused transitions to Playing", "[phase_f][f1][lifecycle]") {
    PIEService svc;
    svc.enter();
    svc.pause();
    REQUIRE(svc.resume());
    REQUIRE(svc.isPlaying());
}

TEST_CASE("PIEService resume while Playing is no-op", "[phase_f][f1][lifecycle]") {
    PIEService svc;
    svc.enter();
    REQUIRE_FALSE(svc.resume());
    REQUIRE(svc.isPlaying());
}

TEST_CASE("PIEService resume while Stopped is no-op", "[phase_f][f1][lifecycle]") {
    PIEService svc;
    REQUIRE_FALSE(svc.resume());
}

TEST_CASE("PIEService step while Paused increments tickIndex", "[phase_f][f1][lifecycle]") {
    PIEService svc;
    svc.enter();
    svc.pause();
    uint32_t before = svc.counters().tickIndex;
    REQUIRE(svc.step());
    REQUIRE(svc.counters().tickIndex == before + 1);
}

TEST_CASE("PIEService step while Playing is no-op", "[phase_f][f1][lifecycle]") {
    PIEService svc;
    svc.enter();
    REQUIRE_FALSE(svc.step());
}

TEST_CASE("PIEService step while Stopped is no-op", "[phase_f][f1][lifecycle]") {
    PIEService svc;
    REQUIRE_FALSE(svc.step());
}

TEST_CASE("PIEService reset from Playing restores counters and stays Playing", "[phase_f][f1][lifecycle]") {
    PIEService svc;
    svc.enter();
    svc.tickFrame(16.6f, 60.f, 42, 100, 1024*1024);
    svc.tickFrame(16.6f, 60.f, 42, 100, 1024*1024);
    REQUIRE(svc.counters().tickIndex == 2);
    REQUIRE(svc.reset());
    REQUIRE(svc.isPlaying());
    REQUIRE(svc.counters().tickIndex == 0);
}

TEST_CASE("PIEService reset from Paused switches back to Playing", "[phase_f][f1][lifecycle]") {
    PIEService svc;
    svc.enter();
    svc.pause();
    REQUIRE(svc.reset());
    REQUIRE(svc.isPlaying());
}

TEST_CASE("PIEService reset while Stopped is no-op", "[phase_f][f1][lifecycle]") {
    PIEService svc;
    REQUIRE_FALSE(svc.reset());
}

// ── F.1 — PIEService callbacks ────────────────────────────────────────────────

TEST_CASE("PIEService onEnter callback fires on enter()", "[phase_f][f1][callbacks]") {
    PIEService svc;
    bool fired = false;
    svc.setOnEnter([&]{ fired = true; });
    svc.enter();
    REQUIRE(fired);
}

TEST_CASE("PIEService onExit callback fires on exit()", "[phase_f][f1][callbacks]") {
    PIEService svc;
    bool fired = false;
    svc.setOnExit([&]{ fired = true; });
    svc.enter();
    svc.exit();
    REQUIRE(fired);
}

TEST_CASE("PIEService onPause callback fires on pause()", "[phase_f][f1][callbacks]") {
    PIEService svc;
    bool fired = false;
    svc.setOnPause([&]{ fired = true; });
    svc.enter();
    svc.pause();
    REQUIRE(fired);
}

TEST_CASE("PIEService onResume callback fires on resume()", "[phase_f][f1][callbacks]") {
    PIEService svc;
    bool fired = false;
    svc.setOnResume([&]{ fired = true; });
    svc.enter();
    svc.pause();
    svc.resume();
    REQUIRE(fired);
}

TEST_CASE("PIEService onStep callback receives tickIndex", "[phase_f][f1][callbacks]") {
    PIEService svc;
    uint32_t receivedTick = 0;
    svc.setOnStep([&](uint32_t t){ receivedTick = t; });
    svc.enter();
    svc.pause();
    svc.step();
    REQUIRE(receivedTick == 1);
    svc.step();
    REQUIRE(receivedTick == 2);
}

TEST_CASE("PIEService onReset callback fires on reset()", "[phase_f][f1][callbacks]") {
    PIEService svc;
    bool fired = false;
    svc.setOnReset([&]{ fired = true; });
    svc.enter();
    svc.reset();
    REQUIRE(fired);
}

// ── F.1 — Session history ─────────────────────────────────────────────────────

TEST_CASE("PIEService sessionId increments with each enter()", "[phase_f][f1][session]") {
    PIEService svc;
    svc.enter(); svc.exit();
    svc.enter(); svc.exit();
    REQUIRE(svc.sessionId() == 2);
}

TEST_CASE("PIEService sessionHistory grows after each exit()", "[phase_f][f1][session]") {
    PIEService svc;
    REQUIRE(svc.sessionCount() == 0);
    svc.enter(); svc.exit();
    REQUIRE(svc.sessionCount() == 1);
    svc.enter(); svc.exit();
    REQUIRE(svc.sessionCount() == 2);
}

TEST_CASE("PIEService session record totalTicks matches tickFrame calls", "[phase_f][f1][session]") {
    PIEService svc;
    svc.enter();
    svc.tickFrame(16.6f, 60.f, 10, 50, 0);
    svc.tickFrame(16.6f, 60.f, 10, 50, 0);
    svc.tickFrame(16.6f, 60.f, 10, 50, 0);
    svc.exit();
    REQUIRE(svc.sessionHistory()[0].totalTicks == 3);
}

TEST_CASE("PIEService session record durationMs uses clock", "[phase_f][f1][session]") {
    PIEService svc;
    svc.enter();
    svc.advanceClock(500);
    svc.exit();
    REQUIRE(svc.sessionHistory()[0].durationMs() == 500);
}

// ── F.1 — Performance counters ────────────────────────────────────────────────

TEST_CASE("PIEService tickFrame updates counters while Playing", "[phase_f][f1][perf]") {
    PIEService svc;
    svc.enter();
    svc.tickFrame(16.6f, 60.f, 42, 100, 2*1024*1024);
    const auto& c = svc.counters();
    REQUIRE(c.fps          == Approx(60.f));
    REQUIRE(c.entityCount  == 42);
    REQUIRE(c.drawCallCount == 100);
    REQUIRE(c.memoryBytes  == 2*1024*1024);
    REQUIRE(c.lastFrameMs  == Approx(16.6f));
    REQUIRE(c.tickIndex    == 1);
}

TEST_CASE("PIEService tickFrame is no-op while Paused", "[phase_f][f1][perf]") {
    PIEService svc;
    svc.enter();
    svc.pause();
    svc.tickFrame(16.6f, 60.f, 10, 50, 0);
    REQUIRE(svc.counters().tickIndex == 0);
}

TEST_CASE("PIEService tickFrame is no-op while Stopped", "[phase_f][f1][perf]") {
    PIEService svc;
    svc.tickFrame(16.6f, 60.f, 10, 50, 0);
    REQUIRE(svc.counters().tickIndex == 0);
}

// ── F.1 — Diagnostics ────────────────────────────────────────────────────────

TEST_CASE("PIEDiagnosticSeverity names are correct", "[phase_f][f1][diag]") {
    REQUIRE(std::string(pieDiagnosticSeverityName(PIEDiagnosticSeverity::Info))     == "Info");
    REQUIRE(std::string(pieDiagnosticSeverityName(PIEDiagnosticSeverity::Warning))  == "Warning");
    REQUIRE(std::string(pieDiagnosticSeverityName(PIEDiagnosticSeverity::Error))    == "Error");
    REQUIRE(std::string(pieDiagnosticSeverityName(PIEDiagnosticSeverity::Critical)) == "Critical");
}

TEST_CASE("PIEService pushDiagnostic records event", "[phase_f][f1][diag]") {
    PIEService svc;
    svc.enter();
    PIEDiagnosticEvent evt;
    evt.severity = PIEDiagnosticSeverity::Warning;
    evt.message  = "Low memory";
    evt.source   = "Runtime";
    svc.pushDiagnostic(evt);
    REQUIRE(svc.diagnosticCount() == 1);
    REQUIRE(svc.diagnostics()[0].message == "Low memory");
}

TEST_CASE("PIEService errorCount counts Error and Critical", "[phase_f][f1][diag]") {
    PIEService svc;
    svc.enter();
    PIEDiagnosticEvent e1; e1.severity = PIEDiagnosticSeverity::Error;   e1.message = "E";
    PIEDiagnosticEvent e2; e2.severity = PIEDiagnosticSeverity::Critical; e2.message = "C";
    PIEDiagnosticEvent e3; e3.severity = PIEDiagnosticSeverity::Warning;  e3.message = "W";
    svc.pushDiagnostic(e1);
    svc.pushDiagnostic(e2);
    svc.pushDiagnostic(e3);
    REQUIRE(svc.errorCount() == 2);
}

TEST_CASE("PIEService countBySeverity filters correctly", "[phase_f][f1][diag]") {
    PIEService svc;
    svc.enter();
    for (int i = 0; i < 3; ++i) {
        PIEDiagnosticEvent ev; ev.severity = PIEDiagnosticSeverity::Info; ev.message = "i";
        svc.pushDiagnostic(ev);
    }
    PIEDiagnosticEvent w; w.severity = PIEDiagnosticSeverity::Warning; w.message = "w";
    svc.pushDiagnostic(w);
    REQUIRE(svc.countBySeverity(PIEDiagnosticSeverity::Info)    == 3);
    REQUIRE(svc.countBySeverity(PIEDiagnosticSeverity::Warning) == 1);
    REQUIRE(svc.countBySeverity(PIEDiagnosticSeverity::Error)   == 0);
}

TEST_CASE("PIEService onDiagnostic callback invoked per event", "[phase_f][f1][diag]") {
    PIEService svc;
    svc.enter();
    std::vector<std::string> received;
    svc.setOnDiagnostic([&](const PIEDiagnosticEvent& e){ received.push_back(e.message); });
    PIEDiagnosticEvent e1; e1.message = "A"; svc.pushDiagnostic(e1);
    PIEDiagnosticEvent e2; e2.message = "B"; svc.pushDiagnostic(e2);
    REQUIRE(received.size() == 2);
    REQUIRE(received[0] == "A");
    REQUIRE(received[1] == "B");
}

TEST_CASE("PIEService diagnostic list cleared on enter", "[phase_f][f1][diag]") {
    PIEService svc;
    svc.enter();
    PIEDiagnosticEvent ev; ev.message = "old"; svc.pushDiagnostic(ev);
    REQUIRE(svc.diagnosticCount() == 1);
    svc.exit();
    svc.enter(); // new session
    REQUIRE(svc.diagnosticCount() == 0);
}

TEST_CASE("PIEService session record captures events on exit", "[phase_f][f1][diag]") {
    PIEService svc;
    svc.enter();
    PIEDiagnosticEvent e1; e1.message = "runtime error"; e1.severity = PIEDiagnosticSeverity::Error;
    PIEDiagnosticEvent e2; e2.message = "warning"; e2.severity = PIEDiagnosticSeverity::Warning;
    svc.pushDiagnostic(e1);
    svc.pushDiagnostic(e2);
    svc.exit();
    REQUIRE(svc.sessionHistory()[0].events.size() == 2);
    REQUIRE(svc.sessionHistory()[0].errorCount == 1);
}

// ── F.2 — PIEInputRouter ─────────────────────────────────────────────────────

TEST_CASE("PIEInputMode names are correct", "[phase_f][f2][input_router]") {
    REQUIRE(std::string(pieInputModeName(PIEInputMode::Editor)) == "Editor");
    REQUIRE(std::string(pieInputModeName(PIEInputMode::Game))   == "Game");
}

TEST_CASE("PIEInputRouter default mode is Editor", "[phase_f][f2][input_router]") {
    PIEInputRouter r;
    REQUIRE(r.isEditorMode());
    REQUIRE_FALSE(r.isGameMode());
    REQUIRE(r.mode() == PIEInputMode::Editor);
}

TEST_CASE("PIEInputRouter routeToGame switches to Game mode", "[phase_f][f2][input_router]") {
    PIEInputRouter r;
    r.routeToGame();
    REQUIRE(r.isGameMode());
    REQUIRE(r.modeSwitchCount() == 1);
}

TEST_CASE("PIEInputRouter routeToEditor switches back to Editor mode", "[phase_f][f2][input_router]") {
    PIEInputRouter r;
    r.routeToGame();
    r.routeToEditor();
    REQUIRE(r.isEditorMode());
    REQUIRE(r.modeSwitchCount() == 2);
}

TEST_CASE("PIEInputRouter routeToGame idempotent — no extra switch count", "[phase_f][f2][input_router]") {
    PIEInputRouter r;
    r.routeToGame();
    r.routeToGame();
    REQUIRE(r.modeSwitchCount() == 1);
}

TEST_CASE("PIEInputRouter onModeChange callback fires on switch", "[phase_f][f2][input_router]") {
    PIEInputRouter r;
    PIEInputMode received = PIEInputMode::Editor;
    r.setOnModeChange([&](PIEInputMode m){ received = m; });
    r.routeToGame();
    REQUIRE(received == PIEInputMode::Game);
    r.routeToEditor();
    REQUIRE(received == PIEInputMode::Editor);
}

TEST_CASE("PIEInputRouter processKey dispatches to gameKeySink in Game mode", "[phase_f][f2][input_router]") {
    PIEInputRouter r;
    r.routeToGame();
    uint32_t receivedKey = 0;
    r.setGameKeySink([&](const PIEKeyEvent& e){ receivedKey = e.keyCode; });
    PIEKeyEvent evt; evt.keyCode = 0x41; // 'A'
    REQUIRE(r.processKey(evt));
    REQUIRE(receivedKey == 0x41);
}

TEST_CASE("PIEInputRouter processKey dispatches to editorKeySink in Editor mode", "[phase_f][f2][input_router]") {
    PIEInputRouter r;
    uint32_t receivedKey = 0;
    r.setEditorKeySink([&](const PIEKeyEvent& e){ receivedKey = e.keyCode; });
    PIEKeyEvent evt; evt.keyCode = 0x44; // 'D'
    REQUIRE(r.processKey(evt));
    REQUIRE(receivedKey == 0x44);
}

TEST_CASE("PIEInputRouter processKey returns false with no sink", "[phase_f][f2][input_router]") {
    PIEInputRouter r;
    PIEKeyEvent evt; evt.keyCode = 0x41;
    REQUIRE_FALSE(r.processKey(evt)); // no editor sink registered
}

TEST_CASE("PIEInputRouter processKey keyEventCount increments always", "[phase_f][f2][input_router]") {
    PIEInputRouter r;
    PIEKeyEvent evt;
    r.processKey(evt);
    r.processKey(evt);
    REQUIRE(r.keyEventCount() == 2);
}

TEST_CASE("PIEInputRouter processMouseButton dispatches to correct sink", "[phase_f][f2][input_router]") {
    PIEInputRouter r;
    r.routeToGame();
    uint8_t receivedBtn = 99;
    r.setGameMouseSink([&](const PIEMouseEvent& e){ receivedBtn = e.button; });
    PIEMouseEvent evt; evt.button = 1;
    REQUIRE(r.processMouseButton(evt));
    REQUIRE(receivedBtn == 1);
    REQUIRE(r.mouseEventCount() == 1);
}

TEST_CASE("PIEInputRouter processMouseMove dispatches to correct sink", "[phase_f][f2][input_router]") {
    PIEInputRouter r;
    float recvDx = 0.f;
    r.setEditorMouseMoveSink([&](const PIEMouseMoveEvent& e){ recvDx = e.dx; });
    PIEMouseMoveEvent evt; evt.dx = 12.5f;
    REQUIRE(r.processMouseMove(evt));
    REQUIRE(recvDx == Approx(12.5f));
    REQUIRE(r.mouseMoveCount() == 1);
}

TEST_CASE("PIEInputRouter isExitKey detects escape in Game mode", "[phase_f][f2][input_router]") {
    PIEInputRouter r;
    r.routeToGame();
    PIEKeyEvent esc; esc.keyCode = 0x1B; esc.pressed = true;
    REQUIRE(r.isExitKey(esc));
}

TEST_CASE("PIEInputRouter isExitKey returns false in Editor mode", "[phase_f][f2][input_router]") {
    PIEInputRouter r;
    PIEKeyEvent esc; esc.keyCode = 0x1B; esc.pressed = true;
    REQUIRE_FALSE(r.isExitKey(esc));
}

TEST_CASE("PIEInputRouter custom exitKeyCode respected", "[phase_f][f2][input_router]") {
    PIEInputRouter r;
    r.routeToGame();
    r.setExitKeyCode(0xF1); // custom exit key
    PIEKeyEvent k; k.keyCode = 0xF1; k.pressed = true;
    REQUIRE(r.isExitKey(k));
    PIEKeyEvent esc; esc.keyCode = 0x1B; esc.pressed = true;
    REQUIRE_FALSE(r.isExitKey(esc));
}

TEST_CASE("PIEInputRouter release of exit key is not an exit", "[phase_f][f2][input_router]") {
    PIEInputRouter r;
    r.routeToGame();
    PIEKeyEvent esc; esc.keyCode = 0x1B; esc.pressed = false; // released
    REQUIRE_FALSE(r.isExitKey(esc));
}

// ── F.3 — PIEExternalLaunch ───────────────────────────────────────────────────

TEST_CASE("PIEProcessState names are correct", "[phase_f][f3][external_launch]") {
    REQUIRE(std::string(pieProcessStateName(PIEProcessState::Idle))      == "Idle");
    REQUIRE(std::string(pieProcessStateName(PIEProcessState::Launching)) == "Launching");
    REQUIRE(std::string(pieProcessStateName(PIEProcessState::Running))   == "Running");
    REQUIRE(std::string(pieProcessStateName(PIEProcessState::Exited))    == "Exited");
    REQUIRE(std::string(pieProcessStateName(PIEProcessState::Crashed))   == "Crashed");
}

TEST_CASE("PIEExternalLaunch default state is Idle", "[phase_f][f3][external_launch]") {
    PIEExternalLaunch el;
    REQUIRE(el.state() == PIEProcessState::Idle);
    REQUIRE_FALSE(el.isRunning());
    REQUIRE_FALSE(el.hasExited());
    REQUIRE(el.launchCount() == 0);
    REQUIRE(el.processId() == 0);
}

TEST_CASE("PIEExternalLaunch hasValidConfig false by default", "[phase_f][f3][external_launch]") {
    PIEExternalLaunch el;
    REQUIRE_FALSE(el.hasValidConfig());
}

TEST_CASE("PIEExternalLaunch hasValidConfig true with executable and project", "[phase_f][f3][external_launch]") {
    PIEExternalLaunch el;
    PIELaunchConfig cfg;
    cfg.executablePath  = "bin/NovaForgeGame.exe";
    cfg.projectFilePath = "MyProject.atlas";
    el.setConfig(cfg);
    REQUIRE(el.hasValidConfig());
}

TEST_CASE("PIEExternalLaunch launch with invalid config returns failure", "[phase_f][f3][external_launch]") {
    PIEExternalLaunch el;
    auto result = el.launch();
    REQUIRE(result.failed());
    REQUIRE_FALSE(result.errorMsg.empty());
    REQUIRE(el.state() == PIEProcessState::Idle);
}

TEST_CASE("PIEExternalLaunch launch with valid config succeeds", "[phase_f][f3][external_launch]") {
    PIEExternalLaunch el;
    PIELaunchConfig cfg;
    cfg.executablePath  = "bin/NovaForgeGame.exe";
    cfg.projectFilePath = "MyProject.atlas";
    el.setConfig(cfg);
    auto result = el.launch();
    REQUIRE(result.ok());
    REQUIRE(el.isRunning());
    REQUIRE(el.launchCount() == 1);
    REQUIRE(el.processId() != 0);
}

TEST_CASE("PIEExternalLaunch launch while already Running returns failure", "[phase_f][f3][external_launch]") {
    PIEExternalLaunch el;
    PIELaunchConfig cfg;
    cfg.executablePath = "bin/game.exe"; cfg.projectFilePath = "p.atlas";
    el.setConfig(cfg);
    el.launch();
    auto result = el.launch(); // second launch
    REQUIRE(result.failed());
    REQUIRE(el.launchCount() == 1);
}

TEST_CASE("PIEExternalLaunch terminate stops running process", "[phase_f][f3][external_launch]") {
    PIEExternalLaunch el;
    PIELaunchConfig cfg;
    cfg.executablePath = "bin/game.exe"; cfg.projectFilePath = "p.atlas";
    el.setConfig(cfg);
    el.launch();
    REQUIRE(el.terminate());
    REQUIRE(el.hasExited());
    REQUIRE(el.lastExitCode() == -1);
}

TEST_CASE("PIEExternalLaunch terminate while Idle returns false", "[phase_f][f3][external_launch]") {
    PIEExternalLaunch el;
    REQUIRE_FALSE(el.terminate());
}

TEST_CASE("PIEExternalLaunch simulateExit(0) transitions to Exited", "[phase_f][f3][external_launch]") {
    PIEExternalLaunch el;
    PIELaunchConfig cfg;
    cfg.executablePath = "bin/game.exe"; cfg.projectFilePath = "p.atlas";
    el.setConfig(cfg);
    el.launch();
    el.simulateExit(0);
    REQUIRE(el.hasExited());
    REQUIRE_FALSE(el.hasCrashed());
    REQUIRE(el.lastExitCode() == 0);
}

TEST_CASE("PIEExternalLaunch simulateExit(nonzero) transitions to Crashed", "[phase_f][f3][external_launch]") {
    PIEExternalLaunch el;
    PIELaunchConfig cfg;
    cfg.executablePath = "bin/game.exe"; cfg.projectFilePath = "p.atlas";
    el.setConfig(cfg);
    el.launch();
    el.simulateExit(139); // SIGSEGV
    REQUIRE(el.hasCrashed());
    REQUIRE(el.lastExitCode() == 139);
}

TEST_CASE("PIEExternalLaunch launchCount increments per launch", "[phase_f][f3][external_launch]") {
    PIEExternalLaunch el;
    PIELaunchConfig cfg;
    cfg.executablePath = "bin/game.exe"; cfg.projectFilePath = "p.atlas";
    el.setConfig(cfg);
    el.launch(); el.terminate();
    el.launch(); el.terminate();
    REQUIRE(el.launchCount() == 2);
}

TEST_CASE("PIEExternalLaunch onLaunched callback fires with PID", "[phase_f][f3][external_launch]") {
    PIEExternalLaunch el;
    PIELaunchConfig cfg;
    cfg.executablePath = "bin/game.exe"; cfg.projectFilePath = "p.atlas";
    el.setConfig(cfg);
    uint32_t receivedPid = 0;
    el.setOnLaunched([&](uint32_t pid){ receivedPid = pid; });
    el.launch();
    REQUIRE(receivedPid == el.processId());
}

TEST_CASE("PIEExternalLaunch onExited callback fires with exit code", "[phase_f][f3][external_launch]") {
    PIEExternalLaunch el;
    PIELaunchConfig cfg;
    cfg.executablePath = "bin/game.exe"; cfg.projectFilePath = "p.atlas";
    el.setConfig(cfg);
    int32_t receivedCode = 999;
    el.setOnExited([&](int32_t code){ receivedCode = code; });
    el.launch();
    el.simulateExit(42);
    REQUIRE(receivedCode == 42);
}

TEST_CASE("PIEExternalLaunch pushStdoutLine routes to sink", "[phase_f][f3][external_launch]") {
    PIEExternalLaunch el;
    std::vector<std::string> lines;
    el.setOnStdoutLine([&](const std::string& l){ lines.push_back(l); });
    el.pushStdoutLine("Game started");
    el.pushStdoutLine("Loading level...");
    REQUIRE(el.stdoutLineCount() == 2);
    REQUIRE(lines.size() == 2);
    REQUIRE(lines[0] == "Game started");
}

TEST_CASE("PIEExternalLaunch stdoutLines cleared on new launch", "[phase_f][f3][external_launch]") {
    PIEExternalLaunch el;
    PIELaunchConfig cfg;
    cfg.executablePath = "bin/game.exe"; cfg.projectFilePath = "p.atlas";
    el.setConfig(cfg);
    el.launch();
    el.pushStdoutLine("Old message");
    el.terminate();
    el.launch();
    REQUIRE(el.stdoutLineCount() == 0);
}

TEST_CASE("PIELaunchConfig isValid requires both paths", "[phase_f][f3][external_launch]") {
    PIELaunchConfig cfg;
    REQUIRE_FALSE(cfg.isValid());
    cfg.executablePath = "game.exe";
    REQUIRE_FALSE(cfg.isValid()); // missing project
    cfg.projectFilePath = "proj.atlas";
    REQUIRE(cfg.isValid());
}

// ─────────────────────────────────────────────────────────────────────────────
// PIEExternalLaunch — POSIX real process launch integration tests
//
// These tests require a real POSIX environment and run the system 'echo'
// binary as the child process, verifying that fork/execv, stdout piping,
// exit detection, and terminate() all work correctly against a live process.
// ─────────────────────────────────────────────────────────────────────────────

#ifndef _WIN32
#include <chrono>
#include <thread>

static bool waitForCondition(std::function<bool()> pred, int timeoutMs = 2000) {
    const int stepMs = 20;
    for (int elapsed = 0; elapsed < timeoutMs; elapsed += stepMs) {
        if (pred()) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(stepMs));
    }
    return pred();
}

TEST_CASE("PIEExternalLaunch POSIX: real launch of /bin/echo succeeds",
          "[phase_f][f3][posix_launch]") {
    PIEExternalLaunch el;
    PIELaunchConfig cfg;
    cfg.executablePath  = "/bin/echo";
    cfg.projectFilePath = "test.atlas";
    el.setConfig(cfg);

    auto result = el.launch();
    REQUIRE(result.ok());
    REQUIRE(el.isRunning());
    // Real PID — not a synthetic stub value (stub uses 1000+launchCount).
    REQUIRE(el.processId() > 0);
}

TEST_CASE("PIEExternalLaunch POSIX: process exits naturally with code 0",
          "[phase_f][f3][posix_launch]") {
    PIEExternalLaunch el;
    PIELaunchConfig cfg;
    cfg.executablePath  = "/bin/echo";
    cfg.projectFilePath = "test.atlas";
    el.setConfig(cfg);

    int32_t receivedCode = -999;
    el.setOnExited([&](int32_t code) { receivedCode = code; });

    el.launch();

    REQUIRE(waitForCondition([&] { return el.hasExited(); }));
    CHECK(el.hasExited());
    CHECK_FALSE(el.hasCrashed());
    CHECK(el.lastExitCode() == 0);
    CHECK(receivedCode == 0);
}

TEST_CASE("PIEExternalLaunch POSIX: stdout lines captured from child process",
          "[phase_f][f3][posix_launch]") {
    PIEExternalLaunch el;
    PIELaunchConfig cfg;
    // echo outputs its arguments followed by a newline.
    cfg.executablePath  = "/bin/echo";
    cfg.projectFilePath = "hello";
    el.setConfig(cfg);

    std::vector<std::string> captured;
    el.setOnStdoutLine([&](const std::string& line) { captured.push_back(line); });

    el.launch();

    REQUIRE(waitForCondition([&] { return el.hasExited(); }));
    // echo should have produced at least one output line.
    REQUIRE(el.stdoutLineCount() >= 1);
    REQUIRE(captured.size() >= 1);
    // The first token in the output must contain "hello" (or "test.atlas hello" etc.)
    CHECK(captured[0].find("hello") != std::string::npos);
}

TEST_CASE("PIEExternalLaunch POSIX: launchCount increments for real launches",
          "[phase_f][f3][posix_launch]") {
    PIEExternalLaunch el;
    PIELaunchConfig cfg;
    cfg.executablePath  = "/bin/echo";
    cfg.projectFilePath = "p.atlas";
    el.setConfig(cfg);

    el.launch();
    REQUIRE(waitForCondition([&] { return el.hasExited(); }));
    el.launch();
    REQUIRE(waitForCondition([&] { return el.hasExited(); }));

    CHECK(el.launchCount() == 2);
}

TEST_CASE("PIEExternalLaunch POSIX: onExited fires exactly once per launch",
          "[phase_f][f3][posix_launch]") {
    PIEExternalLaunch el;
    PIELaunchConfig cfg;
    cfg.executablePath  = "/bin/echo";
    cfg.projectFilePath = "p.atlas";
    el.setConfig(cfg);

    int fireCount = 0;
    el.setOnExited([&](int32_t) { ++fireCount; });

    el.launch();
    REQUIRE(waitForCondition([&] { return el.hasExited(); }));

    CHECK(fireCount == 1);
}

TEST_CASE("PIEExternalLaunch POSIX: terminate() kills long-running process",
          "[phase_f][f3][posix_launch]") {
    PIEExternalLaunch el;
    PIELaunchConfig cfg;
    // 'sleep' runs indefinitely until killed.
    cfg.executablePath  = "/bin/sleep";
    cfg.projectFilePath = "60";   // treated as argv[1] so: sleep 60
    el.setConfig(cfg);

    int32_t receivedCode = -999;
    el.setOnExited([&](int32_t code) { receivedCode = code; });

    el.launch();
    REQUIRE(el.isRunning());

    bool ok = el.terminate();
    CHECK(ok);
    CHECK(el.hasExited());
    CHECK(el.lastExitCode() == -1);
    CHECK(receivedCode == -1);
}

TEST_CASE("PIEExternalLaunch POSIX: stdout cleared between launches",
          "[phase_f][f3][posix_launch]") {
    PIEExternalLaunch el;
    PIELaunchConfig cfg;
    cfg.executablePath  = "/bin/echo";
    cfg.projectFilePath = "first";
    el.setConfig(cfg);

    el.launch();
    REQUIRE(waitForCondition([&] { return el.hasExited(); }));
    uint32_t afterFirst = el.stdoutLineCount();
    CHECK(afterFirst >= 1);

    el.launch();
    // Immediately after the second launch stdoutLines should be cleared.
    CHECK(el.stdoutLineCount() == 0);

    REQUIRE(waitForCondition([&] { return el.hasExited(); }));
}

#endif // !_WIN32
