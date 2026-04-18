#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── FrameStats defaults ─────────────────────────────────────────

TEST_CASE("FrameStats default values are all zero", "[Editor][S46]") {
    FrameStats fs;
    REQUIRE(fs.fps == 0.f);
    REQUIRE(fs.frameTimeMs == 0.f);
    REQUIRE(fs.updateTimeMs == 0.f);
    REQUIRE(fs.renderTimeMs == 0.f);
    REQUIRE(fs.frameCount == 0);
}

// ── FrameStatsTracker ───────────────────────────────────────────

TEST_CASE("FrameStatsTracker initial stats are zero", "[Editor][S46]") {
    FrameStatsTracker tracker;
    const auto& s = tracker.stats();
    REQUIRE(s.fps == 0.f);
    REQUIRE(s.frameTimeMs == 0.f);
    REQUIRE(s.frameCount == 0);
}

TEST_CASE("FrameStatsTracker beginFrame increments frameCount", "[Editor][S46]") {
    FrameStatsTracker tracker;
    tracker.beginFrame(0.016f);
    REQUIRE(tracker.stats().frameCount == 1);

    tracker.beginFrame(0.016f);
    REQUIRE(tracker.stats().frameCount == 2);

    tracker.beginFrame(0.033f);
    REQUIRE(tracker.stats().frameCount == 3);
}

TEST_CASE("FrameStatsTracker beginFrame sets frameTimeMs", "[Editor][S46]") {
    FrameStatsTracker tracker;
    tracker.beginFrame(0.016f);
    REQUIRE(tracker.stats().frameTimeMs == Catch::Approx(16.f).margin(0.01f));

    tracker.beginFrame(0.033f);
    REQUIRE(tracker.stats().frameTimeMs == Catch::Approx(33.f).margin(0.01f));
}

TEST_CASE("FrameStatsTracker FPS uses EMA", "[Editor][S46]") {
    FrameStatsTracker tracker;
    // First frame: fps = 0 * 0.9 + (1/0.016) * 0.1 = 6.25
    tracker.beginFrame(0.016f);
    float expected = 0.f * 0.9f + (1.f / 0.016f) * 0.1f;
    REQUIRE(tracker.stats().fps == Catch::Approx(expected).margin(0.1f));

    // Second frame continues the EMA
    float prev = tracker.stats().fps;
    tracker.beginFrame(0.016f);
    float expected2 = prev * 0.9f + (1.f / 0.016f) * 0.1f;
    REQUIRE(tracker.stats().fps == Catch::Approx(expected2).margin(0.1f));
}

TEST_CASE("FrameStatsTracker FPS with zero dt produces zero contribution", "[Editor][S46]") {
    FrameStatsTracker tracker;
    tracker.beginFrame(0.f);
    // newFps = 0 when dt == 0, so fps = 0 * 0.9 + 0 * 0.1 = 0
    REQUIRE(tracker.stats().fps == 0.f);
}

TEST_CASE("FrameStatsTracker recordUpdateTime sets updateTimeMs", "[Editor][S46]") {
    FrameStatsTracker tracker;
    tracker.beginFrame(0.016f);
    tracker.recordUpdateTime(3.5f);
    REQUIRE(tracker.stats().updateTimeMs == 3.5f);
}

TEST_CASE("FrameStatsTracker recordRenderTime sets renderTimeMs", "[Editor][S46]") {
    FrameStatsTracker tracker;
    tracker.beginFrame(0.016f);
    tracker.recordRenderTime(7.2f);
    REQUIRE(tracker.stats().renderTimeMs == 7.2f);
}

TEST_CASE("FrameStatsTracker multiple frames with record times", "[Editor][S46]") {
    FrameStatsTracker tracker;

    tracker.beginFrame(0.016f);
    tracker.recordUpdateTime(2.0f);
    tracker.recordRenderTime(8.0f);

    tracker.beginFrame(0.033f);
    tracker.recordUpdateTime(5.0f);
    tracker.recordRenderTime(12.0f);

    const auto& s = tracker.stats();
    REQUIRE(s.frameCount == 2);
    REQUIRE(s.updateTimeMs == 5.0f);
    REQUIRE(s.renderTimeMs == 12.0f);
    REQUIRE(s.frameTimeMs == Catch::Approx(33.f).margin(0.01f));
}

TEST_CASE("FrameStatsTracker FPS converges toward steady rate", "[Editor][S46]") {
    FrameStatsTracker tracker;
    // Run many frames at ~60 FPS (dt = 1/60)
    for (int i = 0; i < 100; ++i)
        tracker.beginFrame(1.f / 60.f);

    // After 100 frames at 60fps, EMA should converge near 60
    REQUIRE(tracker.stats().fps == Catch::Approx(60.f).margin(1.f));
}
