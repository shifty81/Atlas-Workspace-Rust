// Tests/Workspace/test_phase34_frame_controller.cpp
// Phase 34 — Workspace Frame Controller
//
// Tests for:
//   1. FrameBudget        — validity, defaults
//   2. FrameResult        — default state, fields
//   3. FrameStatistics    — default state, reset, budgetUtilization
//   4. WorkspaceFrameController — construction, config, frame lifecycle,
//                                 EMA smoothing, skip detection, pacing
//   5. Integration        — simulated frame loop, stats accumulation

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceFrameController.h"
#include <cmath>

using namespace NF;

// Epsilon for float comparisons
static constexpr float kEps = 1e-4f;

// ─────────────────────────────────────────────────────────────────
// 1. FrameBudget
// ─────────────────────────────────────────────────────────────────
TEST_CASE("FrameBudget default is valid", "[FrameBudget]") {
    FrameBudget b;
    CHECK(b.isValid());
    CHECK(b.totalMs  > 0.f);
    CHECK(b.updateMs > 0.f);
    CHECK(b.renderMs > 0.f);
}

TEST_CASE("FrameBudget invalid when totalMs is zero", "[FrameBudget]") {
    FrameBudget b;
    b.totalMs = 0.f;
    CHECK_FALSE(b.isValid());
}

TEST_CASE("FrameBudget invalid when updateMs is zero", "[FrameBudget]") {
    FrameBudget b;
    b.updateMs = 0.f;
    CHECK_FALSE(b.isValid());
}

// ─────────────────────────────────────────────────────────────────
// 2. FrameResult
// ─────────────────────────────────────────────────────────────────
TEST_CASE("FrameResult default state is zeroed", "[FrameResult]") {
    FrameResult fr;
    CHECK(fr.dt          == 0.f);
    CHECK(fr.rawDt       == 0.f);
    CHECK_FALSE(fr.wasSkipped);
    CHECK(fr.frameNumber == 0u);
}

// ─────────────────────────────────────────────────────────────────
// 3. FrameStatistics
// ─────────────────────────────────────────────────────────────────
TEST_CASE("FrameStatistics default state", "[FrameStatistics]") {
    FrameStatistics s;
    CHECK(s.totalFrames   == 0u);
    CHECK(s.fps           == 0.f);
    CHECK(s.avgDtMs       == 0.f);
    CHECK(s.skippedFrames == 0u);
}

TEST_CASE("FrameStatistics reset clears all fields", "[FrameStatistics]") {
    FrameStatistics s;
    s.totalFrames = 42;
    s.fps         = 60.f;
    s.avgDtMs     = 16.f;
    s.reset();
    CHECK(s.totalFrames == 0u);
    CHECK(s.fps         == 0.f);
    CHECK(s.avgDtMs     == 0.f);
}

TEST_CASE("FrameStatistics budgetUtilization returns zero for zero budget", "[FrameStatistics]") {
    FrameStatistics s;
    s.lastUpdateMs = 4.f;
    s.lastRenderMs = 8.f;
    CHECK(s.budgetUtilization(0.f) == 0.f);
}

TEST_CASE("FrameStatistics budgetUtilization correct ratio", "[FrameStatistics]") {
    FrameStatistics s;
    s.lastUpdateMs = 4.f;
    s.lastRenderMs = 8.f;
    float util = s.budgetUtilization(16.f);
    CHECK(std::fabs(util - (12.f / 16.f)) < kEps);
}

// ─────────────────────────────────────────────────────────────────
// 4. WorkspaceFrameController
// ─────────────────────────────────────────────────────────────────
TEST_CASE("WorkspaceFrameController default target is 60 FPS", "[WorkspaceFrameController]") {
    WorkspaceFrameController fc;
    CHECK(std::fabs(fc.targetFPS() - 60.f) < kEps);
}

TEST_CASE("WorkspaceFrameController default maxDtSec is 0.1", "[WorkspaceFrameController]") {
    WorkspaceFrameController fc;
    CHECK(std::fabs(fc.maxDtSec() - 0.1f) < kEps);
}

TEST_CASE("WorkspaceFrameController setTargetFPS updates budget", "[WorkspaceFrameController]") {
    WorkspaceFrameController fc;
    fc.setTargetFPS(30.f);
    CHECK(std::fabs(fc.targetFPS() - 30.f) < kEps);
    CHECK(std::fabs(fc.budget().totalMs - (1000.f / 30.f)) < kEps);
}

TEST_CASE("WorkspaceFrameController setTargetFPS ignores non-positive", "[WorkspaceFrameController]") {
    WorkspaceFrameController fc;
    float before = fc.targetFPS();
    fc.setTargetFPS(0.f);
    CHECK(std::fabs(fc.targetFPS() - before) < kEps);
    fc.setTargetFPS(-1.f);
    CHECK(std::fabs(fc.targetFPS() - before) < kEps);
}

TEST_CASE("WorkspaceFrameController setMaxDeltaTime updates maxDtSec", "[WorkspaceFrameController]") {
    WorkspaceFrameController fc;
    fc.setMaxDeltaTime(0.05f);
    CHECK(std::fabs(fc.maxDtSec() - 0.05f) < kEps);
}

TEST_CASE("WorkspaceFrameController setEMAAlpha within valid range", "[WorkspaceFrameController]") {
    WorkspaceFrameController fc;
    fc.setEMAAlpha(0.5f);
    CHECK(std::fabs(fc.emaAlpha() - 0.5f) < kEps);
}

TEST_CASE("WorkspaceFrameController setEMAAlpha ignores out-of-range", "[WorkspaceFrameController]") {
    WorkspaceFrameController fc;
    float before = fc.emaAlpha();
    fc.setEMAAlpha(0.f);
    CHECK(std::fabs(fc.emaAlpha() - before) < kEps);
    fc.setEMAAlpha(1.5f);
    CHECK(std::fabs(fc.emaAlpha() - before) < kEps);
}

TEST_CASE("WorkspaceFrameController setBudget accepts valid budget", "[WorkspaceFrameController]") {
    WorkspaceFrameController fc;
    FrameBudget b;
    b.totalMs  = 33.33f; // ~30 FPS
    b.updateMs = 8.f;
    b.renderMs = 25.f;
    fc.setBudget(b);
    CHECK(std::fabs(fc.budget().totalMs - 33.33f) < kEps);
}

TEST_CASE("WorkspaceFrameController setBudget ignores invalid budget", "[WorkspaceFrameController]") {
    WorkspaceFrameController fc;
    float before = fc.budget().totalMs;
    FrameBudget bad;
    bad.totalMs = 0.f;
    fc.setBudget(bad);
    CHECK(std::fabs(fc.budget().totalMs - before) < kEps);
}

TEST_CASE("WorkspaceFrameController beginFrame increments frameNumber", "[WorkspaceFrameController]") {
    WorkspaceFrameController fc;
    auto r1 = fc.beginFrame(0.016f);
    CHECK(r1.frameNumber == 1u);
    fc.endFrame();
    auto r2 = fc.beginFrame(0.016f);
    CHECK(r2.frameNumber == 2u);
}

TEST_CASE("WorkspaceFrameController beginFrame clamps negative dt", "[WorkspaceFrameController]") {
    WorkspaceFrameController fc;
    auto r = fc.beginFrame(-1.f);
    CHECK(r.rawDt > 0.f);
}

TEST_CASE("WorkspaceFrameController beginFrame clamps dt above maxDtSec", "[WorkspaceFrameController]") {
    WorkspaceFrameController fc;
    fc.setMaxDeltaTime(0.05f);
    auto r = fc.beginFrame(1.0f); // huge spike
    CHECK(r.rawDt <= 0.05f + kEps);
}

TEST_CASE("WorkspaceFrameController first frame seeds EMA with clamped dt", "[WorkspaceFrameController]") {
    WorkspaceFrameController fc;
    fc.setEMAAlpha(1.0f); // alpha=1 → EMA equals rawDt immediately
    auto r = fc.beginFrame(0.016f);
    CHECK(std::fabs(r.dt - 0.016f) < kEps);
}

TEST_CASE("WorkspaceFrameController markUpdateDone and markRenderDone record timing", "[WorkspaceFrameController]") {
    WorkspaceFrameController fc;
    fc.beginFrame(0.016f);
    fc.markUpdateDone(3.f);
    fc.markRenderDone(10.f);
    fc.endFrame();
    CHECK(std::fabs(fc.stats().lastUpdateMs - 3.f) < kEps);
    CHECK(std::fabs(fc.stats().lastRenderMs - 10.f) < kEps);
}

TEST_CASE("WorkspaceFrameController endFrame increments totalFrames", "[WorkspaceFrameController]") {
    WorkspaceFrameController fc;
    fc.beginFrame(0.016f);
    fc.endFrame();
    CHECK(fc.stats().totalFrames == 1u);
    fc.beginFrame(0.016f);
    fc.endFrame();
    CHECK(fc.stats().totalFrames == 2u);
}

TEST_CASE("WorkspaceFrameController skippedFrames when over budget", "[WorkspaceFrameController]") {
    WorkspaceFrameController fc;
    fc.setTargetFPS(60.f); // budget ~16.67 ms
    fc.beginFrame(0.016f);
    fc.markUpdateDone(5.f);
    fc.markRenderDone(20.f); // 25 ms total > 16.67 ms
    fc.endFrame(25.f);
    CHECK(fc.stats().skippedFrames == 1u);
}

TEST_CASE("WorkspaceFrameController wasSkipped set on next frame after budget overrun", "[WorkspaceFrameController]") {
    WorkspaceFrameController fc;
    fc.beginFrame(0.016f);
    fc.endFrame(100.f); // way over budget
    auto r = fc.beginFrame(0.016f);
    CHECK(r.wasSkipped);
}

TEST_CASE("WorkspaceFrameController shouldSleep when under budget", "[WorkspaceFrameController]") {
    WorkspaceFrameController fc;
    fc.setTargetFPS(60.f); // budget ~16.67 ms
    CHECK(fc.shouldSleep(10.f));
    CHECK_FALSE(fc.shouldSleep(20.f));
}

TEST_CASE("WorkspaceFrameController sleepMs returns correct slack", "[WorkspaceFrameController]") {
    WorkspaceFrameController fc;
    fc.setTargetFPS(60.f); // budget ~16.67 ms
    float slack = fc.sleepMs(10.f);
    CHECK(slack > 0.f);
    CHECK(slack < fc.budget().totalMs);
}

TEST_CASE("WorkspaceFrameController resetStats clears statistics", "[WorkspaceFrameController]") {
    WorkspaceFrameController fc;
    fc.beginFrame(0.016f);
    fc.endFrame();
    REQUIRE(fc.stats().totalFrames == 1u);
    fc.resetStats();
    CHECK(fc.stats().totalFrames == 0u);
    CHECK(fc.frameNumber() == 0u);
}

// ─────────────────────────────────────────────────────────────────
// 5. Integration — simulated frame loop
// ─────────────────────────────────────────────────────────────────
TEST_CASE("FrameController integration: 10-frame loop accumulates stats", "[FrameControllerIntegration]") {
    WorkspaceFrameController fc;
    fc.setTargetFPS(60.f);

    const int N = 10;
    for (int i = 0; i < N; ++i) {
        fc.beginFrame(0.016f);
        fc.markUpdateDone(2.f);
        fc.markRenderDone(10.f);
        fc.endFrame(12.f);
    }

    CHECK(fc.stats().totalFrames == static_cast<uint64_t>(N));
    CHECK(fc.stats().skippedFrames == 0u); // 12 ms < 16.67 ms
    CHECK(fc.stats().fps > 0.f);
}

TEST_CASE("FrameController integration: budget overrun is detected", "[FrameControllerIntegration]") {
    WorkspaceFrameController fc;
    fc.setTargetFPS(60.f);

    // Frame 1: under budget
    fc.beginFrame(0.016f);
    fc.markUpdateDone(4.f);
    fc.markRenderDone(8.f);
    fc.endFrame(12.f);
    CHECK(fc.stats().skippedFrames == 0u);

    // Frame 2: over budget
    fc.beginFrame(0.016f);
    fc.markUpdateDone(5.f);
    fc.markRenderDone(20.f);
    fc.endFrame(25.f);
    CHECK(fc.stats().skippedFrames == 1u);
}

TEST_CASE("FrameController integration: EMA converges toward steady-state dt", "[FrameControllerIntegration]") {
    WorkspaceFrameController fc;
    fc.setEMAAlpha(0.5f);
    // Simulate 20 frames at 0.016 s each – EMA should converge toward 0.016
    for (int i = 0; i < 20; ++i) {
        fc.beginFrame(0.016f);
        fc.endFrame();
    }
    // After 20 frames with alpha=0.5, residual error < 0.001 s
    CHECK(std::fabs(fc.emaDtSec() - 0.016f) < 0.001f);
}
