// Tests/Workspace/test_phase73_pie_viewport.cpp
//
// Phase 73 — PIE Viewport Full Integration
//
// PIEGameSceneProvider:
//   - default mode Stopped; provideScene returns empty
//   - enterGame → Playing; exitGame → Stopped
//   - double-enterGame is no-op
//   - pauseGame / resumeGame transitions
//   - tickGame advances entity positions in Playing mode
//   - tickGame is no-op in Paused/Stopped modes
//   - tickCount increments only while Playing
//   - addEntity / removeEntity / clearEntities
//   - entityCount reflects mutations
//   - provideScene populates entity proxies in Playing/Paused
//   - provideScene returns empty in Stopped
//   - setSelectedId marks correct entity as selected
//   - clearColor default and setter
//   - mode name strings
//
// PIEViewportBridge:
//   - default state: not connected
//   - connect() with invalid handle returns false
//   - connect() with null gameProvider returns false
//   - connect() succeeds and sets isConnected
//   - viewportMode starts Editor after connect
//   - PIE enter → viewportMode becomes Game
//   - PIE exit  → viewportMode reverts to Editor
//   - PIE pause → gameProvider transitions to Paused
//   - PIE resume → gameProvider transitions to Playing
//   - switchCount increments on each mode transition
//   - onSwitchToGame callback fires on PIE enter
//   - onSwitchToEditor callback fires on PIE exit
//   - disconnect clears connection and restores editor provider
//   - disconnect while in game mode calls exitGame on provider
//
// PIEViewportOverlay:
//   - default state Stopped; commandCount may be non-zero (overlay always on)
//   - badgeText returns "PIE: STOPPED" / "PIE: PLAYING" / "PIE: PAUSED"
//   - badgeColor differs per state
//   - statsText formats FPS/entity/tick correctly
//   - updateState with no change does not rebuild commands
//   - updateState with new state rebuilds commands
//   - setVisible(false) clears command list
//   - setVisible(true) restores commands
//   - commands include badge rect, badge text, stats text
//   - Tick bottom-left item only present in Playing/Paused
//   - counters accessor returns the last pushed counters
//
// End-to-end:
//   - E2E: connect bridge → PIE enter → tickGame → provideScene has content
//   - E2E: PIE exit → provideScene returns empty

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "NF/Workspace/PIEGameSceneProvider.h"
#include "NF/Workspace/PIEViewportBridge.h"
#include "NF/Workspace/PIEViewportOverlay.h"
#include "NF/Workspace/WorkspaceViewportManager.h"
#include "NF/Editor/PIEService.h"

using namespace NF;
using Catch::Approx;

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

static PIEGameEntity makeEntity(uint32_t id, float x = 0.f, float y = 0.f, float z = 0.f,
                                float vx = 0.f, float vy = 0.f, float vz = 0.f) {
    PIEGameEntity e;
    e.id = id;
    e.name = "E" + std::to_string(id);
    e.x = x; e.y = y; e.z = z;
    e.vx = vx; e.vy = vy; e.vz = vz;
    e.halfExtent = 0.5f;
    return e;
}

// Allocate a viewport slot in a manager and activate it.
static ViewportHandle setupViewport(WorkspaceViewportManager& mgr,
                                    const std::string& toolId = "test.tool") {
    ViewportHandle h = mgr.requestViewport(toolId, {0.f, 0.f, 320.f, 240.f});
    mgr.activateViewport(h);
    return h;
}

// ═════════════════════════════════════════════════════════════════════════════
// PIEGameSceneProvider tests
// ═════════════════════════════════════════════════════════════════════════════

TEST_CASE("PIEGameSceneProvider: mode name strings", "[phase73][game_provider][names]") {
    CHECK(std::string(pieGameSceneModeName(PIEGameSceneMode::Stopped)) == "Stopped");
    CHECK(std::string(pieGameSceneModeName(PIEGameSceneMode::Playing)) == "Playing");
    CHECK(std::string(pieGameSceneModeName(PIEGameSceneMode::Paused))  == "Paused");
}

TEST_CASE("PIEGameSceneProvider: default state is Stopped", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    REQUIRE(p.isStopped());
    REQUIRE_FALSE(p.isPlaying());
    REQUIRE_FALSE(p.isPaused());
    REQUIRE(p.mode() == PIEGameSceneMode::Stopped);
    REQUIRE(p.entityCount() == 0);
    REQUIRE(p.tickCount() == 0);
}

TEST_CASE("PIEGameSceneProvider: provideScene returns empty when Stopped", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    p.addEntity(makeEntity(1));

    ViewportSlot slot;
    auto state = p.provideScene(1, slot);
    CHECK_FALSE(state.hasContent);
    CHECK(state.entityCount == 0);
    CHECK(state.entities.empty());
}

TEST_CASE("PIEGameSceneProvider: enterGame transitions to Playing", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    REQUIRE(p.enterGame());
    REQUIRE(p.isPlaying());
    REQUIRE_FALSE(p.isStopped());
}

TEST_CASE("PIEGameSceneProvider: double enterGame is no-op", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    p.enterGame();
    REQUIRE_FALSE(p.enterGame()); // already Playing
    REQUIRE(p.isPlaying());
}

TEST_CASE("PIEGameSceneProvider: exitGame → Stopped", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    p.enterGame();
    REQUIRE(p.exitGame());
    REQUIRE(p.isStopped());
}

TEST_CASE("PIEGameSceneProvider: exitGame while Stopped is no-op", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    REQUIRE_FALSE(p.exitGame());
}

TEST_CASE("PIEGameSceneProvider: pauseGame from Playing → Paused", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    p.enterGame();
    REQUIRE(p.pauseGame());
    REQUIRE(p.isPaused());
    REQUIRE_FALSE(p.isPlaying());
}

TEST_CASE("PIEGameSceneProvider: pauseGame while Stopped is no-op", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    REQUIRE_FALSE(p.pauseGame());
}

TEST_CASE("PIEGameSceneProvider: resumeGame from Paused → Playing", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    p.enterGame();
    p.pauseGame();
    REQUIRE(p.resumeGame());
    REQUIRE(p.isPlaying());
}

TEST_CASE("PIEGameSceneProvider: resumeGame while Playing is no-op", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    p.enterGame();
    REQUIRE_FALSE(p.resumeGame());
}

TEST_CASE("PIEGameSceneProvider: addEntity / entityCount", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    REQUIRE(p.addEntity(makeEntity(1)));
    REQUIRE(p.addEntity(makeEntity(2)));
    REQUIRE(p.entityCount() == 2);
}

TEST_CASE("PIEGameSceneProvider: addEntity duplicate id is rejected", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    p.addEntity(makeEntity(42));
    REQUIRE_FALSE(p.addEntity(makeEntity(42)));
    REQUIRE(p.entityCount() == 1);
}

TEST_CASE("PIEGameSceneProvider: removeEntity", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    p.addEntity(makeEntity(1));
    p.addEntity(makeEntity(2));
    REQUIRE(p.removeEntity(1));
    REQUIRE(p.entityCount() == 1);
    REQUIRE_FALSE(p.removeEntity(99));
}

TEST_CASE("PIEGameSceneProvider: clearEntities", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    p.addEntity(makeEntity(1));
    p.addEntity(makeEntity(2));
    p.clearEntities();
    REQUIRE(p.entityCount() == 0);
}

TEST_CASE("PIEGameSceneProvider: tickGame advances positions while Playing", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    p.addEntity(makeEntity(1, 0.f, 0.f, 0.f, 1.f, 2.f, 3.f)); // velocity 1,2,3
    p.enterGame();
    p.tickGame(1.0f); // 1 second
    const auto& e = p.entities().front();
    CHECK(e.x == Approx(1.f));
    CHECK(e.y == Approx(2.f));
    CHECK(e.z == Approx(3.f));
}

TEST_CASE("PIEGameSceneProvider: tickGame is no-op while Paused", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    p.addEntity(makeEntity(1, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f));
    p.enterGame();
    p.pauseGame();
    p.tickGame(10.f); // should not advance
    CHECK(p.entities().front().x == Approx(0.f));
}

TEST_CASE("PIEGameSceneProvider: tickGame is no-op while Stopped", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    p.addEntity(makeEntity(1, 5.f, 0.f, 0.f, 1.f, 0.f, 0.f));
    p.tickGame(10.f);
    CHECK(p.entities().front().x == Approx(5.f));
}

TEST_CASE("PIEGameSceneProvider: tickCount increments only while Playing", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    p.tickGame(1.f); // Stopped → no change
    REQUIRE(p.tickCount() == 0);

    p.enterGame();
    p.tickGame(1.f);
    p.tickGame(1.f);
    REQUIRE(p.tickCount() == 2);

    p.pauseGame();
    p.tickGame(1.f); // Paused → no change
    REQUIRE(p.tickCount() == 2);
}

TEST_CASE("PIEGameSceneProvider: provideScene returns entity proxies while Playing", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    p.addEntity(makeEntity(1, 1.f, 2.f, 3.f));
    p.enterGame();

    ViewportSlot slot;
    auto st = p.provideScene(1, slot);
    REQUIRE(st.hasContent);
    REQUIRE(st.entityCount == 1);
    REQUIRE(st.entities.size() == 1);
    CHECK(st.entities[0].x == Approx(1.f));
    CHECK(st.entities[0].y == Approx(2.f));
    CHECK(st.entities[0].z == Approx(3.f));
}

TEST_CASE("PIEGameSceneProvider: provideScene holds last snapshot while Paused", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    p.addEntity(makeEntity(1));
    p.enterGame();
    p.pauseGame();

    ViewportSlot slot;
    auto st = p.provideScene(1, slot);
    CHECK(st.hasContent);
    CHECK(st.entityCount == 1);
}

TEST_CASE("PIEGameSceneProvider: setSelectedId marks entity as selected", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    p.addEntity(makeEntity(1));
    p.addEntity(makeEntity(2));
    p.setSelectedId(2);
    p.enterGame();
    ViewportSlot slot;
    auto st = p.provideScene(1, slot);
    bool foundSelected = false;
    for (const auto& proxy : st.entities)
        if (proxy.selected) { foundSelected = true; break; }
    CHECK(foundSelected);
}

TEST_CASE("PIEGameSceneProvider: clearColor default and setter", "[phase73][game_provider]") {
    PIEGameSceneProvider p;
    CHECK(p.clearColor() == 0x0D1B2AFFu);
    p.setClearColor(0xFF000000u);
    CHECK(p.clearColor() == 0xFF000000u);
}

// ═════════════════════════════════════════════════════════════════════════════
// PIEViewportBridge tests
// ═════════════════════════════════════════════════════════════════════════════

TEST_CASE("PIEViewportBridge: default state not connected", "[phase73][bridge]") {
    PIEViewportBridge bridge;
    CHECK_FALSE(bridge.isConnected());
    CHECK(bridge.handle() == kInvalidViewportHandle);
    CHECK(bridge.switchCount() == 0);
}

TEST_CASE("PIEViewportBridge: connect with invalid handle returns false", "[phase73][bridge]") {
    PIEService svc;
    WorkspaceViewportManager mgr;
    PIEGameSceneProvider game;

    PIEViewportBridge bridge;
    CHECK_FALSE(bridge.connect(svc, mgr, kInvalidViewportHandle, nullptr, &game));
    CHECK_FALSE(bridge.isConnected());
}

TEST_CASE("PIEViewportBridge: connect with null game provider returns false", "[phase73][bridge]") {
    PIEService svc;
    WorkspaceViewportManager mgr;
    ViewportHandle h = setupViewport(mgr);

    PIEViewportBridge bridge;
    CHECK_FALSE(bridge.connect(svc, mgr, h, nullptr, nullptr));
    CHECK_FALSE(bridge.isConnected());
}

TEST_CASE("PIEViewportBridge: connect succeeds and reports isConnected", "[phase73][bridge]") {
    PIEService svc;
    WorkspaceViewportManager mgr;
    PIEGameSceneProvider game;
    ViewportHandle h = setupViewport(mgr);

    PIEViewportBridge bridge;
    REQUIRE(bridge.connect(svc, mgr, h, nullptr, &game));
    CHECK(bridge.isConnected());
    CHECK(bridge.handle() == h);
    CHECK(bridge.viewportMode() == PIEViewportMode::Editor);
    CHECK(bridge.isEditorMode());
}

TEST_CASE("PIEViewportBridge: PIE enter switches viewport to Game mode", "[phase73][bridge]") {
    PIEService svc;
    WorkspaceViewportManager mgr;
    PIEGameSceneProvider game;
    ViewportHandle h = setupViewport(mgr);

    PIEViewportBridge bridge;
    bridge.connect(svc, mgr, h, nullptr, &game);

    svc.enter(); // triggers onEnter callback

    CHECK(bridge.isGameMode());
    CHECK(game.isPlaying());
    CHECK(bridge.switchCount() == 1);
}

TEST_CASE("PIEViewportBridge: PIE exit reverts viewport to Editor mode", "[phase73][bridge]") {
    PIEService svc;
    WorkspaceViewportManager mgr;
    PIEGameSceneProvider game;
    ViewportHandle h = setupViewport(mgr);

    PIEViewportBridge bridge;
    bridge.connect(svc, mgr, h, nullptr, &game);

    svc.enter();
    CHECK(bridge.isGameMode());

    svc.exit();
    CHECK(bridge.isEditorMode());
    CHECK(game.isStopped());
    CHECK(bridge.switchCount() == 2);
}

TEST_CASE("PIEViewportBridge: PIE pause transitions game provider to Paused", "[phase73][bridge]") {
    PIEService svc;
    WorkspaceViewportManager mgr;
    PIEGameSceneProvider game;
    ViewportHandle h = setupViewport(mgr);

    PIEViewportBridge bridge;
    bridge.connect(svc, mgr, h, nullptr, &game);

    svc.enter();
    svc.pause();

    CHECK(game.isPaused());
    CHECK(bridge.isGameMode()); // still game mode, just paused
}

TEST_CASE("PIEViewportBridge: PIE resume transitions game provider back to Playing", "[phase73][bridge]") {
    PIEService svc;
    WorkspaceViewportManager mgr;
    PIEGameSceneProvider game;
    ViewportHandle h = setupViewport(mgr);

    PIEViewportBridge bridge;
    bridge.connect(svc, mgr, h, nullptr, &game);

    svc.enter();
    svc.pause();
    svc.resume();

    CHECK(game.isPlaying());
    CHECK(bridge.isGameMode());
}

TEST_CASE("PIEViewportBridge: onSwitchToGame callback fires on PIE enter", "[phase73][bridge]") {
    PIEService svc;
    WorkspaceViewportManager mgr;
    PIEGameSceneProvider game;
    ViewportHandle h = setupViewport(mgr);

    int callCount = 0;
    PIEViewportBridge bridge;
    bridge.setOnSwitchToGame([&]() { ++callCount; });
    bridge.connect(svc, mgr, h, nullptr, &game);

    svc.enter();
    CHECK(callCount == 1);
    svc.exit();
    CHECK(callCount == 1); // no extra fire on exit
}

TEST_CASE("PIEViewportBridge: onSwitchToEditor callback fires on PIE exit", "[phase73][bridge]") {
    PIEService svc;
    WorkspaceViewportManager mgr;
    PIEGameSceneProvider game;
    ViewportHandle h = setupViewport(mgr);

    int callCount = 0;
    PIEViewportBridge bridge;
    bridge.setOnSwitchToEditor([&]() { ++callCount; });
    bridge.connect(svc, mgr, h, nullptr, &game);

    svc.enter();
    CHECK(callCount == 0);
    svc.exit();
    CHECK(callCount == 1);
}

TEST_CASE("PIEViewportBridge: disconnect clears connection", "[phase73][bridge]") {
    PIEService svc;
    WorkspaceViewportManager mgr;
    PIEGameSceneProvider game;
    ViewportHandle h = setupViewport(mgr);

    PIEViewportBridge bridge;
    bridge.connect(svc, mgr, h, nullptr, &game);
    bridge.disconnect();

    CHECK_FALSE(bridge.isConnected());
    CHECK(bridge.handle() == kInvalidViewportHandle);
}

TEST_CASE("PIEViewportBridge: disconnect while in game mode exits game provider", "[phase73][bridge]") {
    PIEService svc;
    WorkspaceViewportManager mgr;
    PIEGameSceneProvider game;
    ViewportHandle h = setupViewport(mgr);

    PIEViewportBridge bridge;
    bridge.connect(svc, mgr, h, nullptr, &game);
    svc.enter();
    REQUIRE(bridge.isGameMode());

    bridge.disconnect();
    CHECK(game.isStopped());
}

TEST_CASE("PIEViewportBridge: mode name strings", "[phase73][bridge][names]") {
    CHECK(std::string(pieViewportModeName(PIEViewportMode::Editor)) == "Editor");
    CHECK(std::string(pieViewportModeName(PIEViewportMode::Game))   == "Game");
}

// ═════════════════════════════════════════════════════════════════════════════
// PIEViewportOverlay tests
// ═════════════════════════════════════════════════════════════════════════════

TEST_CASE("PIEViewportOverlay: default state is Stopped with commands visible", "[phase73][overlay]") {
    PIEViewportOverlay ov;
    CHECK(ov.state() == PIEState::Stopped);
    CHECK(ov.isVisible());
    // Overlay is always-on by default: badge rect + badge text + stats text = 3 commands.
    CHECK(ov.commandCount() == 3);
}

TEST_CASE("PIEViewportOverlay: badgeText per state", "[phase73][overlay]") {
    PIEViewportOverlay ov;

    ov.updateState(PIEState::Stopped,  {});
    CHECK(ov.badgeText() == "PIE: STOPPED");

    ov.updateState(PIEState::Playing,  {});
    CHECK(ov.badgeText() == "PIE: PLAYING");

    ov.updateState(PIEState::Paused,   {});
    CHECK(ov.badgeText() == "PIE: PAUSED");
}

TEST_CASE("PIEViewportOverlay: badgeColor differs per state", "[phase73][overlay]") {
    PIEViewportOverlay ov;
    ov.updateState(PIEState::Playing, {});
    uint32_t playColor = ov.badgeColor();

    ov.updateState(PIEState::Paused, {});
    uint32_t pauseColor = ov.badgeColor();

    ov.updateState(PIEState::Stopped, {});
    uint32_t stopColor = ov.badgeColor();

    CHECK(playColor  != pauseColor);
    CHECK(playColor  != stopColor);
    CHECK(pauseColor != stopColor);
}

TEST_CASE("PIEViewportOverlay: statsText formats counters correctly", "[phase73][overlay]") {
    PIEViewportOverlay ov;
    PIEPerformanceCounters c;
    c.fps         = 60.f;
    c.entityCount = 12;
    c.tickIndex   = 314;
    ov.updateState(PIEState::Playing, c);
    std::string txt = ov.statsText();
    CHECK(txt.find("FPS:")  != std::string::npos);
    CHECK(txt.find("60.0")  != std::string::npos);
    CHECK(txt.find("12")    != std::string::npos);
    CHECK(txt.find("314")   != std::string::npos);
}

TEST_CASE("PIEViewportOverlay: updateState with same values does not rebuild", "[phase73][overlay]") {
    PIEViewportOverlay ov;
    PIEPerformanceCounters c;
    c.fps = 30.f;
    ov.updateState(PIEState::Playing, c);
    uint32_t firstCount = ov.commandCount();
    ov.updateState(PIEState::Playing, c); // same — no rebuild
    CHECK(ov.commandCount() == firstCount);
}

TEST_CASE("PIEViewportOverlay: commands list is non-empty when visible and Playing", "[phase73][overlay]") {
    PIEViewportOverlay ov;
    ov.updateState(PIEState::Playing, {});
    CHECK(ov.commandCount() > 0);
}

TEST_CASE("PIEViewportOverlay: setVisible false clears commands", "[phase73][overlay]") {
    PIEViewportOverlay ov;
    ov.updateState(PIEState::Playing, {});
    ov.setVisible(false);
    CHECK(ov.commandCount() == 0);
}

TEST_CASE("PIEViewportOverlay: setVisible true restores commands", "[phase73][overlay]") {
    PIEViewportOverlay ov;
    ov.updateState(PIEState::Playing, {});
    ov.setVisible(false);
    ov.setVisible(true);
    CHECK(ov.commandCount() > 0);
}

TEST_CASE("PIEViewportOverlay: commands include badge rect and text", "[phase73][overlay]") {
    PIEViewportOverlay ov;
    ov.updateState(PIEState::Playing, {});

    bool hasRect = false, hasText = false;
    for (const auto& cmd : ov.commands()) {
        if (cmd.kind == PIEOverlayDrawCommand::Kind::Rect) hasRect = true;
        if (cmd.kind == PIEOverlayDrawCommand::Kind::Text &&
            cmd.text.find("PIE:") != std::string::npos)    hasText = true;
    }
    CHECK(hasRect);
    CHECK(hasText);
}

TEST_CASE("PIEViewportOverlay: Tick label present when Playing", "[phase73][overlay]") {
    PIEViewportOverlay ov;
    PIEPerformanceCounters c;
    c.tickIndex = 42;
    ov.updateState(PIEState::Playing, c);

    bool hasTick = false;
    for (const auto& cmd : ov.commands())
        if (cmd.kind == PIEOverlayDrawCommand::Kind::Text &&
            cmd.anchor == PIEOverlayTextAnchor::BottomLeft &&
            cmd.text.find("Tick:") != std::string::npos)
            hasTick = true;

    CHECK(hasTick);
}

TEST_CASE("PIEViewportOverlay: Tick label absent when Stopped", "[phase73][overlay]") {
    PIEViewportOverlay ov;
    ov.updateState(PIEState::Stopped, {});

    bool hasTick = false;
    for (const auto& cmd : ov.commands())
        if (cmd.kind == PIEOverlayDrawCommand::Kind::Text &&
            cmd.anchor == PIEOverlayTextAnchor::BottomLeft &&
            cmd.text.find("Tick:") != std::string::npos)
            hasTick = true;

    CHECK_FALSE(hasTick);
}

TEST_CASE("PIEViewportOverlay: counters accessor returns last pushed counters", "[phase73][overlay]") {
    PIEViewportOverlay ov;
    PIEPerformanceCounters c;
    c.fps = 144.f;
    c.entityCount = 99;
    ov.updateState(PIEState::Playing, c);
    CHECK(ov.counters().fps         == Approx(144.f));
    CHECK(ov.counters().entityCount == 99);
}

// ═════════════════════════════════════════════════════════════════════════════
// End-to-end tests
// ═════════════════════════════════════════════════════════════════════════════

TEST_CASE("E2E: connect bridge, enter PIE, tick game, provideScene has content", "[phase73][e2e]") {
    PIEService svc;
    WorkspaceViewportManager mgr;
    PIEGameSceneProvider game;
    ViewportHandle h = setupViewport(mgr);

    // Add a moving entity to the game scene.
    game.addEntity(makeEntity(1, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f));

    PIEViewportBridge bridge;
    REQUIRE(bridge.connect(svc, mgr, h, nullptr, &game));

    // Enter PIE.
    svc.enter();
    REQUIRE(bridge.isGameMode());
    REQUIRE(game.isPlaying());

    // Tick the game.
    game.tickGame(2.f);
    REQUIRE(game.tickCount() == 1);

    // provideScene should report content.
    ViewportSlot slot;
    auto st = game.provideScene(h, slot);
    REQUIRE(st.hasContent);
    REQUIRE(st.entityCount == 1);
    CHECK(st.entities[0].x == Approx(2.f));
}

TEST_CASE("E2E: PIE exit clears game provider and provideScene returns empty", "[phase73][e2e]") {
    PIEService svc;
    WorkspaceViewportManager mgr;
    PIEGameSceneProvider game;
    ViewportHandle h = setupViewport(mgr);

    game.addEntity(makeEntity(1));

    PIEViewportBridge bridge;
    bridge.connect(svc, mgr, h, nullptr, &game);

    svc.enter();
    svc.exit();

    CHECK(bridge.isEditorMode());
    CHECK(game.isStopped());

    ViewportSlot slot;
    auto st = game.provideScene(h, slot);
    CHECK_FALSE(st.hasContent);
}
