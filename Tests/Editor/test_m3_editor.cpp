#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/EditorApp.h"
#include "NF/Editor/EditorWorldSession.h"
#include "NF/Editor/Legacy/PCGTuning.h"

// ── PlayState enum ───────────────────────────────────────────────

TEST_CASE("PlayState names", "[Editor][M3]") {
    REQUIRE(std::string(NF::playStateName(NF::PlayState::Stopped)) == "Stopped");
    REQUIRE(std::string(NF::playStateName(NF::PlayState::Running)) == "Running");
    REQUIRE(std::string(NF::playStateName(NF::PlayState::Paused))  == "Paused");
}

// ── EditorWorldSnapshot ──────────────────────────────────────────

TEST_CASE("EditorWorldSnapshot capture and invalidate", "[Editor][M3]") {
    NF::EditorWorldSnapshot snap;
    REQUIRE_FALSE(snap.valid);

    NF::NoiseParams params;
    params.frequency = 2.0f;
    std::vector<NF::PlacedEntity> entities;
    NF::PlacedEntity e;
    e.entityId = 1;
    e.templateName = "Rock";
    e.position = {10.f, 0.f, 5.f};
    entities.push_back(e);

    snap.capture("test.nfw", entities, params, {1.f, 2.f, 3.f}, 45.f, -15.f);
    REQUIRE(snap.valid);
    REQUIRE(snap.worldPath == "test.nfw");
    REQUIRE(snap.placedEntities.size() == 1);
    REQUIRE(snap.placedEntities[0].templateName == "Rock");
    REQUIRE(snap.pcgParams.frequency == 2.0f);
    REQUIRE(snap.cameraPosition.x == 1.f);
    REQUIRE(snap.cameraYaw == 45.f);
    REQUIRE(snap.cameraPitch == -15.f);

    snap.invalidate();
    REQUIRE_FALSE(snap.valid);
}

// ── EditorWorldSession ───────────────────────────────────────────

TEST_CASE("EditorWorldSession lifecycle: start → pause → resume → stop", "[Editor][M3]") {
    NF::EditorWorldSession session;
    REQUIRE(session.state() == NF::PlayState::Stopped);
    REQUIRE(session.elapsedTime() == 0.f);
    REQUIRE(session.frameCount() == 0);
    REQUIRE_FALSE(session.hasSnapshot());

    // Start
    REQUIRE(session.start("world.nfw", {}, NF::NoiseParams{}, {}, 0.f, 0.f));
    REQUIRE(session.state() == NF::PlayState::Running);
    REQUIRE(session.hasSnapshot());

    // Can't start again while running
    REQUIRE_FALSE(session.start("other.nfw", {}, NF::NoiseParams{}, {}, 0.f, 0.f));

    // Tick advances time
    session.tick(0.016f);
    session.tick(0.016f);
    REQUIRE(session.elapsedTime() > 0.03f);
    REQUIRE(session.frameCount() == 2);

    // Pause
    REQUIRE(session.pause());
    REQUIRE(session.state() == NF::PlayState::Paused);

    // Tick while paused should not advance
    float t = session.elapsedTime();
    session.tick(1.0f);
    REQUIRE(session.elapsedTime() == t);

    // Resume
    REQUIRE(session.resume());
    REQUIRE(session.state() == NF::PlayState::Running);

    // Can't resume when already running
    REQUIRE_FALSE(session.resume());

    // Stop
    REQUIRE(session.stop());
    REQUIRE(session.state() == NF::PlayState::Stopped);
    REQUIRE(session.hasSnapshot());  // snapshot still valid for restoration

    // Can't stop when already stopped
    REQUIRE_FALSE(session.stop());
}

TEST_CASE("EditorWorldSession pause requires Running", "[Editor][M3]") {
    NF::EditorWorldSession session;
    REQUIRE_FALSE(session.pause());  // can't pause when Stopped
}

TEST_CASE("EditorWorldSession resume requires Paused", "[Editor][M3]") {
    NF::EditorWorldSession session;
    REQUIRE_FALSE(session.resume());  // can't resume when Stopped
}

// ── PlayInEditorSystem ───────────────────────────────────────────

TEST_CASE("PlayInEditorSystem standalone lifecycle", "[Editor][M3]") {
    NF::PlayInEditorSystem pie;
    REQUIRE(pie.isStopped());
    REQUIRE_FALSE(pie.isRunning());
    REQUIRE_FALSE(pie.isPaused());

    REQUIRE(pie.start("test.nfw"));
    REQUIRE(pie.isRunning());
    REQUIRE(pie.state() == NF::PlayState::Running);

    pie.tick(0.5f);
    REQUIRE(pie.elapsedTime() > 0.4f);

    REQUIRE(pie.pause());
    REQUIRE(pie.isPaused());

    REQUIRE(pie.resume());
    REQUIRE(pie.isRunning());

    REQUIRE(pie.stop());
    REQUIRE(pie.isStopped());
}

TEST_CASE("PlayInEditorSystem togglePlay cycles states", "[Editor][M3]") {
    NF::PlayInEditorSystem pie;
    REQUIRE(pie.isStopped());

    pie.togglePlay();  // Stopped → Running
    REQUIRE(pie.isRunning());

    pie.togglePlay();  // Running → Paused
    REQUIRE(pie.isPaused());

    pie.togglePlay();  // Paused → Running
    REQUIRE(pie.isRunning());

    pie.stop();
    REQUIRE(pie.isStopped());
}

TEST_CASE("PlayInEditorSystem snapshots and restores entities", "[Editor][M3]") {
    NF::EntityPlacementTool tool;
    tool.setActiveTemplate("Pillar");
    NF::EntityID id = tool.placeEntity({10.f, 0.f, 5.f});
    REQUIRE(tool.placedCount() == 1);

    NF::PlayInEditorSystem pie;
    pie.setPlacementTool(&tool);

    // Start captures snapshot
    REQUIRE(pie.start("test.nfw"));
    REQUIRE(pie.session().snapshot().placedEntities.size() == 1);
    REQUIRE(pie.session().snapshot().placedEntities[0].entityId == id);

    // Modify during play
    tool.placeEntity({20.f, 0.f, 0.f});
    REQUIRE(tool.placedCount() == 2);

    // Stop should restore
    REQUIRE(pie.stop());
    REQUIRE(tool.placedCount() == 1);
    REQUIRE(tool.placedEntities()[0].entityId == id);
}

TEST_CASE("PlayInEditorSystem snapshots and restores PCG params", "[Editor][M3]") {
    NF::PCGTuningPanel pcg;
    NF::NoiseParams original;
    original.frequency = 1.5f;
    original.seed = 99;
    pcg.setNoiseParams(original);

    NF::PlayInEditorSystem pie;
    pie.setPCGTuningPanel(&pcg);

    REQUIRE(pie.start());

    // Modify PCG during play
    NF::NoiseParams modified;
    modified.frequency = 5.0f;
    modified.seed = 0;
    pcg.setNoiseParams(modified);
    REQUIRE(pcg.noiseParams().frequency == 5.0f);

    // Stop restores original
    REQUIRE(pie.stop());
    REQUIRE(pcg.noiseParams().frequency == 1.5f);
    REQUIRE(pcg.noiseParams().seed == 99);
}

TEST_CASE("PlayInEditorSystem snapshots and restores camera", "[Editor][M3]") {
    NF::ViewportPanel vp;
    vp.setCameraPosition({5.f, 10.f, 15.f});
    vp.setCameraYaw(45.f);
    vp.setCameraPitch(-30.f);

    NF::PlayInEditorSystem pie;
    pie.setViewportPanel(&vp);

    REQUIRE(pie.start());

    // Move camera during play
    vp.setCameraPosition({100.f, 200.f, 300.f});
    vp.setCameraYaw(90.f);

    REQUIRE(pie.stop());
    REQUIRE(vp.cameraPosition().x == 5.f);
    REQUIRE(vp.cameraPosition().y == 10.f);
    REQUIRE(vp.cameraYaw() == 45.f);
    REQUIRE(vp.cameraPitch() == -30.f);
}

// ── EditorApp M3 Integration ─────────────────────────────────────

TEST_CASE("EditorApp M3 Play-in-Editor accessible", "[Editor][M3]") {
    NF::EditorApp app;
    app.init(800, 600);

    auto& pie = app.playInEditor();
    REQUIRE(pie.isStopped());

    // Play command
    REQUIRE(app.commands().executeCommand("play.start"));
    REQUIRE(pie.isRunning());

    // Pause command
    REQUIRE(app.commands().executeCommand("play.pause"));
    REQUIRE(pie.isPaused());

    // Stop command
    REQUIRE(app.commands().executeCommand("play.stop"));
    REQUIRE(pie.isStopped());

    app.shutdown();
}

TEST_CASE("EditorApp Play/Pause/Stop enabled checks", "[Editor][M3]") {
    NF::EditorApp app;
    app.init(800, 600);

    // Pause and Stop should be disabled when stopped
    REQUIRE_FALSE(app.commands().isCommandEnabled("play.pause"));
    REQUIRE_FALSE(app.commands().isCommandEnabled("play.stop"));

    // Start play
    app.commands().executeCommand("play.start");
    REQUIRE(app.commands().isCommandEnabled("play.pause"));
    REQUIRE(app.commands().isCommandEnabled("play.stop"));

    // Pause
    app.commands().executeCommand("play.pause");
    REQUIRE_FALSE(app.commands().isCommandEnabled("play.pause"));
    REQUIRE(app.commands().isCommandEnabled("play.stop"));

    app.shutdown();
}

TEST_CASE("EditorApp panel count includes PCGTuning", "[Editor][M3]") {
    NF::EditorApp app;
    app.init(800, 600);

    // Should have 7 panels: Viewport, Inspector, Hierarchy, Console, ContentBrowser,
    // GraphEditor, PCGTuning
    REQUIRE(app.editorPanels().size() == 7);

    app.shutdown();
}
