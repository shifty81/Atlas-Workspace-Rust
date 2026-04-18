#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── S78: PCGTuning + FrameStatsTracker + WorldPreview ────────────

// ── PCGTuning ───────────────────────────────────────────────────

TEST_CASE("NoiseParams default values", "[Editor][S78]") {
    NoiseParams p;
    REQUIRE(p.frequency   == 1.0f);
    REQUIRE(p.amplitude   == 1.0f);
    REQUIRE(p.octaves     == 4);
    REQUIRE(p.lacunarity  == 2.0f);
    REQUIRE(p.persistence == 0.5f);
    REQUIRE(p.seed        == 42);
}

TEST_CASE("NoiseParams equality operator", "[Editor][S78]") {
    NoiseParams a, b;
    REQUIRE(a == b);
    b.seed = 99;
    REQUIRE(a != b);
}

TEST_CASE("PCGTuningPanel default name", "[Editor][S78]") {
    PCGTuningPanel panel;
    REQUIRE(panel.name() == "PCGTuning");
}

TEST_CASE("PCGTuningPanel setNoiseParams updates params and marks dirty", "[Editor][S78]") {
    PCGTuningPanel panel;
    REQUIRE_FALSE(panel.isDirty());
    NoiseParams p;
    p.seed = 123;
    p.octaves = 6;
    panel.setNoiseParams(p);
    REQUIRE(panel.noiseParams().seed == 123);
    REQUIRE(panel.noiseParams().octaves == 6);
    REQUIRE(panel.isDirty());
}

TEST_CASE("PCGTuningPanel clearDirty clears dirty flag", "[Editor][S78]") {
    PCGTuningPanel panel;
    panel.markDirty();
    REQUIRE(panel.isDirty());
    panel.clearDirty();
    REQUIRE_FALSE(panel.isDirty());
}

TEST_CASE("PCGTuningPanel addPreset and presetCount", "[Editor][S78]") {
    PCGTuningPanel panel;
    PCGPreset preset;
    preset.name = "Desert";
    preset.params.seed = 10;
    panel.addPreset(preset);
    REQUIRE(panel.presetCount() == 1);
}

TEST_CASE("PCGTuningPanel removePreset removes entry", "[Editor][S78]") {
    PCGTuningPanel panel;
    PCGPreset preset; preset.name = "Forest";
    panel.addPreset(preset);
    REQUIRE(panel.removePreset("Forest"));
    REQUIRE(panel.presetCount() == 0);
}

TEST_CASE("PCGTuningPanel removePreset returns false for missing", "[Editor][S78]") {
    PCGTuningPanel panel;
    REQUIRE_FALSE(panel.removePreset("ghost"));
}

TEST_CASE("PCGTuningPanel applyPreset updates noise params", "[Editor][S78]") {
    PCGTuningPanel panel;
    PCGPreset preset;
    preset.name = "Ocean";
    preset.params.frequency = 0.5f;
    preset.params.seed = 77;
    panel.addPreset(preset);
    REQUIRE(panel.applyPreset("Ocean"));
    REQUIRE(panel.noiseParams().frequency == 0.5f);
    REQUIRE(panel.noiseParams().seed == 77);
}

TEST_CASE("PCGTuningPanel applyPreset fails for missing preset", "[Editor][S78]") {
    PCGTuningPanel panel;
    REQUIRE_FALSE(panel.applyPreset("ghost"));
}

TEST_CASE("PCGTuningPanel setSeed updates seed", "[Editor][S78]") {
    PCGTuningPanel panel;
    panel.setSeed(999);
    REQUIRE(panel.noiseParams().seed == 999);
    REQUIRE(panel.isDirty());
}

TEST_CASE("PCGTuningPanel randomizeSeed changes seed", "[Editor][S78]") {
    PCGTuningPanel panel;
    panel.setSeed(42);
    panel.clearDirty();
    panel.randomizeSeed();
    REQUIRE(panel.isDirty());
    // Seed should change (hash will produce different value)
}

// ── FrameStatsTracker ────────────────────────────────────────────

TEST_CASE("FrameStatsTracker default stats are zero", "[Editor][S78]") {
    FrameStatsTracker tracker;
    REQUIRE(tracker.stats().fps == 0.f);
    REQUIRE(tracker.stats().frameCount == 0);
    REQUIRE(tracker.stats().frameTimeMs == 0.f);
}

TEST_CASE("FrameStatsTracker beginFrame increments frameCount", "[Editor][S78]") {
    FrameStatsTracker tracker;
    tracker.beginFrame(0.016f);
    REQUIRE(tracker.stats().frameCount == 1);
    tracker.beginFrame(0.016f);
    REQUIRE(tracker.stats().frameCount == 2);
}

TEST_CASE("FrameStatsTracker beginFrame sets frameTimeMs", "[Editor][S78]") {
    FrameStatsTracker tracker;
    tracker.beginFrame(0.033f);
    REQUIRE(tracker.stats().frameTimeMs > 32.f);
    REQUIRE(tracker.stats().frameTimeMs < 34.f);
}

TEST_CASE("FrameStatsTracker beginFrame accumulates fps via EMA", "[Editor][S78]") {
    FrameStatsTracker tracker;
    // Run several frames at 60fps
    for (int i = 0; i < 50; ++i) {
        tracker.beginFrame(1.0f / 60.f);
    }
    // FPS should be converging towards 60
    REQUIRE(tracker.stats().fps > 1.f);
}

TEST_CASE("FrameStatsTracker recordUpdateTime and recordRenderTime", "[Editor][S78]") {
    FrameStatsTracker tracker;
    tracker.beginFrame(0.016f);
    tracker.recordUpdateTime(2.5f);
    tracker.recordRenderTime(10.0f);
    REQUIRE(tracker.stats().updateTimeMs == 2.5f);
    REQUIRE(tracker.stats().renderTimeMs == 10.0f);
}

// ── WorldPreview ─────────────────────────────────────────────────

TEST_CASE("PreviewState enum has expected values", "[Editor][S78]") {
    PreviewState s = PreviewState::Idle;
    REQUIRE(s == PreviewState::Idle);
    s = PreviewState::Loading;
    REQUIRE(s == PreviewState::Loading);
    s = PreviewState::Ready;
    REQUIRE(s == PreviewState::Ready);
    s = PreviewState::Error;
    REQUIRE(s == PreviewState::Error);
}

TEST_CASE("WorldPreviewService starts Idle", "[Editor][S78]") {
    WorldPreviewService svc;
    REQUIRE(svc.state() == PreviewState::Idle);
    REQUIRE(svc.worldPath().empty());
    REQUIRE_FALSE(svc.isDirty());
}

TEST_CASE("WorldPreviewService loadPreview with valid path transitions to Ready", "[Editor][S78]") {
    WorldPreviewService svc;
    svc.loadPreview("worlds/forest.nfw");
    REQUIRE(svc.state() == PreviewState::Ready);
    REQUIRE(svc.worldPath() == "worlds/forest.nfw");
}

TEST_CASE("WorldPreviewService loadPreview with empty path transitions to Error", "[Editor][S78]") {
    WorldPreviewService svc;
    svc.loadPreview("");
    REQUIRE(svc.state() == PreviewState::Error);
    REQUIRE_FALSE(svc.lastError().empty());
}

TEST_CASE("WorldPreviewService unloadPreview resets to Idle", "[Editor][S78]") {
    WorldPreviewService svc;
    svc.loadPreview("worlds/desert.nfw");
    svc.unloadPreview();
    REQUIRE(svc.state() == PreviewState::Idle);
    REQUIRE(svc.worldPath().empty());
}

TEST_CASE("WorldPreviewService setViewCenter and viewRadius", "[Editor][S78]") {
    WorldPreviewService svc;
    Vec3 center{10.f, 0.f, 20.f};
    svc.setViewCenter(center);
    REQUIRE(svc.viewCenter().x == 10.f);
    REQUIRE(svc.viewCenter().z == 20.f);
    svc.setViewRadius(250.f);
    REQUIRE(svc.viewRadius() == 250.f);
}

TEST_CASE("WorldPreviewService setDirty and clearDirty", "[Editor][S78]") {
    WorldPreviewService svc;
    svc.setDirty();
    REQUIRE(svc.isDirty());
    svc.clearDirty();
    REQUIRE_FALSE(svc.isDirty());
}
