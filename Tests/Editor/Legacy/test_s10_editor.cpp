#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── S10 Tests: Performance Profiler ────────────────────────────

TEST_CASE("ProfileMetricType names", "[Editor][S10]") {
    REQUIRE(std::string(profileMetricTypeName(ProfileMetricType::FrameTime))      == "FrameTime");
    REQUIRE(std::string(profileMetricTypeName(ProfileMetricType::CpuUsage))       == "CpuUsage");
    REQUIRE(std::string(profileMetricTypeName(ProfileMetricType::GpuUsage))       == "GpuUsage");
    REQUIRE(std::string(profileMetricTypeName(ProfileMetricType::MemoryAlloc))    == "MemoryAlloc");
    REQUIRE(std::string(profileMetricTypeName(ProfileMetricType::DrawCalls))       == "DrawCalls");
    REQUIRE(std::string(profileMetricTypeName(ProfileMetricType::TriangleCount))  == "TriangleCount");
    REQUIRE(std::string(profileMetricTypeName(ProfileMetricType::ScriptTime))     == "ScriptTime");
    REQUIRE(std::string(profileMetricTypeName(ProfileMetricType::NetworkLatency)) == "NetworkLatency");
}

TEST_CASE("ProfileSample defaults", "[Editor][S10]") {
    ProfileSample s;
    REQUIRE(s.type == ProfileMetricType::FrameTime);
    REQUIRE(s.value == 0.f);
    REQUIRE(s.timestamp == 0.0);
    REQUIRE_FALSE(s.hasTag());
}

TEST_CASE("ProfileSample hasTag", "[Editor][S10]") {
    ProfileSample s;
    s.tag = "render";
    REQUIRE(s.hasTag());
}

TEST_CASE("ProfileSession start/stop/duration", "[Editor][S10]") {
    ProfileSession sess;
    REQUIRE_FALSE(sess.isActive());
    sess.start(10.0);
    REQUIRE(sess.isActive());
    REQUIRE(sess.duration() == 0.0); // active, returns 0

    sess.stop(15.0);
    REQUIRE_FALSE(sess.isActive());
    REQUIRE(sess.duration() == Catch::Approx(5.0));
}

TEST_CASE("FrameProfiler beginFrame/endFrame", "[Editor][S10]") {
    FrameProfiler fp;
    REQUIRE(fp.totalFrames() == 0);

    fp.beginFrame(0.0);
    fp.endFrame(0.016);
    REQUIRE(fp.totalFrames() == 1);
    REQUIRE(fp.sampleCount() == 1);
    REQUIRE(fp.averageFrameTime() == Catch::Approx(0.016f));
    REQUIRE(fp.peakFrameTime() == Catch::Approx(0.016f));
}

TEST_CASE("FrameProfiler multiple frames + peak tracking", "[Editor][S10]") {
    FrameProfiler fp;
    fp.beginFrame(0.0);
    fp.endFrame(0.010);  // 10ms
    fp.beginFrame(0.010);
    fp.endFrame(0.060);  // 50ms
    fp.beginFrame(0.060);
    fp.endFrame(0.076);  // 16ms

    REQUIRE(fp.totalFrames() == 3);
    REQUIRE(fp.peakFrameTime() == Catch::Approx(0.050f));
    REQUIRE(fp.averageFrameTime() == Catch::Approx(0.076f / 3.f));
}

TEST_CASE("FrameProfiler recordMetric", "[Editor][S10]") {
    FrameProfiler fp;
    fp.recordMetric(ProfileMetricType::DrawCalls, 150.f, 1.0, "scene");
    fp.recordMetric(ProfileMetricType::TriangleCount, 50000.f, 1.0);
    REQUIRE(fp.sampleCount() == 2);
    REQUIRE(fp.samplesByType(ProfileMetricType::DrawCalls).size() == 1);
    REQUIRE(fp.samplesByType(ProfileMetricType::TriangleCount).size() == 1);
}

TEST_CASE("FrameProfiler endFrame without beginFrame ignored", "[Editor][S10]") {
    FrameProfiler fp;
    fp.endFrame(1.0); // no beginFrame
    REQUIRE(fp.totalFrames() == 0);
    REQUIRE(fp.sampleCount() == 0);
}

TEST_CASE("MemoryProfiler track alloc/free", "[Editor][S10]") {
    MemoryProfiler mp;
    mp.trackAllocation(1024);
    REQUIRE(mp.currentUsage() == 1024);
    REQUIRE(mp.allocationCount() == 1);
    REQUIRE(mp.peakUsage() == 1024);

    mp.trackAllocation(2048);
    REQUIRE(mp.currentUsage() == 3072);
    REQUIRE(mp.peakUsage() == 3072);

    mp.trackFree(1024);
    REQUIRE(mp.currentUsage() == 2048);
    REQUIRE(mp.peakUsage() == 3072); // peak unchanged
    REQUIRE(mp.freeCount() == 1);
}

TEST_CASE("MemoryProfiler tagged tracking", "[Editor][S10]") {
    MemoryProfiler mp;
    mp.trackAllocation(512, "textures");
    mp.trackAllocation(256, "meshes");
    REQUIRE(mp.taggedUsage("textures") == 512);
    REQUIRE(mp.taggedUsage("meshes") == 256);
    REQUIRE(mp.taggedUsage("sounds") == 0);

    mp.trackFree(128, "textures");
    REQUIRE(mp.taggedUsage("textures") == 384);
}

TEST_CASE("MemoryProfiler free more than allocated clamps to zero", "[Editor][S10]") {
    MemoryProfiler mp;
    mp.trackAllocation(100);
    mp.trackFree(200);
    REQUIRE(mp.currentUsage() == 0);
}

TEST_CASE("ProfilerTimeline addMarker + markersInRange", "[Editor][S10]") {
    ProfilerTimeline tl;
    tl.addMarker("Frame", 1.0, 0.016f, "Render");
    tl.addMarker("Physics", 2.0, 0.005f, "Simulation");
    tl.addMarker("Audio", 5.0, 0.002f, "Audio");
    REQUIRE(tl.markerCount() == 3);

    auto range = tl.markersInRange(0.5, 3.0);
    REQUIRE(range.size() == 2);
}

TEST_CASE("ProfilerTimeline markersByCategory", "[Editor][S10]") {
    ProfilerTimeline tl;
    tl.addMarker("A", 1.0, 0.01f, "Render");
    tl.addMarker("B", 2.0, 0.01f, "Render");
    tl.addMarker("C", 3.0, 0.01f, "Physics");
    REQUIRE(tl.markersByCategory("Render").size() == 2);
    REQUIRE(tl.markersByCategory("Physics").size() == 1);
}

TEST_CASE("ProfileMarker endTime", "[Editor][S10]") {
    ProfileMarker m;
    m.timestamp = 10.0;
    m.duration = 0.5f;
    REQUIRE(m.endTime() == Catch::Approx(10.5));
}

TEST_CASE("PerformanceProfiler init/shutdown", "[Editor][S10]") {
    PerformanceProfiler pp;
    REQUIRE_FALSE(pp.isInitialized());
    pp.init();
    REQUIRE(pp.isInitialized());
    pp.shutdown();
    REQUIRE_FALSE(pp.isInitialized());
}

TEST_CASE("PerformanceProfiler session management", "[Editor][S10]") {
    PerformanceProfiler pp;
    pp.init();
    pp.startSession("TestRun", 0.0);
    REQUIRE(pp.session().isActive());
    REQUIRE(pp.sessionCount() == 1);

    pp.stopSession(10.0);
    REQUIRE_FALSE(pp.session().isActive());
    REQUIRE(pp.session().duration() == Catch::Approx(10.0));
}

TEST_CASE("PerformanceProfiler frame + memory + timeline", "[Editor][S10]") {
    PerformanceProfiler pp;
    pp.init();

    pp.beginFrame(0.0);
    pp.endFrame(0.016);
    REQUIRE(pp.frameSampleCount() == 1);

    pp.trackAllocation(4096, "test");
    REQUIRE(pp.memoryPeakBytes() == 4096);

    pp.addTimelineMarker("Marker1", 0.0, 0.01f, "Test");
    REQUIRE(pp.timelineMarkerCount() == 1);
}

TEST_CASE("PerformanceProfiler tick", "[Editor][S10]") {
    PerformanceProfiler pp;
    pp.init();
    pp.tick(0.016f);
    pp.tick(0.016f);
    REQUIRE(pp.tickCount() == 2);
}

TEST_CASE("PerformanceProfiler ignores calls when not initialized", "[Editor][S10]") {
    PerformanceProfiler pp;
    pp.beginFrame(0.0);
    pp.endFrame(0.016);
    pp.trackAllocation(100);
    pp.tick(1.f);
    REQUIRE(pp.frameSampleCount() == 0);
    REQUIRE(pp.memoryPeakBytes() == 0);
    REQUIRE(pp.tickCount() == 0);
}

TEST_CASE("FrameProfiler clear resets all state", "[Editor][S10]") {
    FrameProfiler fp;
    fp.beginFrame(0.0);
    fp.endFrame(0.016);
    fp.recordMetric(ProfileMetricType::DrawCalls, 100.f, 1.0);
    REQUIRE(fp.totalFrames() == 1);
    REQUIRE(fp.sampleCount() == 2);

    fp.clear();
    REQUIRE(fp.totalFrames() == 0);
    REQUIRE(fp.sampleCount() == 0);
    REQUIRE(fp.peakFrameTime() == 0.f);
    REQUIRE(fp.averageFrameTime() == 0.f);
}

TEST_CASE("PerformanceProfilerConfig defaults", "[Editor][S10]") {
    PerformanceProfilerConfig cfg;
    REQUIRE(cfg.autoCapture);
    REQUIRE(cfg.maxFrameSamples == 4096);
    REQUIRE(cfg.maxTimelineMarkers == 2048);
    REQUIRE(cfg.warningFrameTimeMs == Catch::Approx(33.33f));
    REQUIRE(cfg.criticalFrameTimeMs == Catch::Approx(50.f));
}
