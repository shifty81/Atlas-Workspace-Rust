// S156 editor tests: ProfilerViewV1, MemoryTrackerV1, DiagnosticPanelV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;
using Catch::Approx;

// ── ProfilerViewV1 ────────────────────────────────────────────────────────────

TEST_CASE("PvvSample validity and overBudget", "[Editor][S156]") {
    PvvSample s;
    REQUIRE(!s.isValid());
    s.id = 1; s.label = "CPU";
    REQUIRE(s.isValid());
    REQUIRE(!s.overBudget());
    s.budgetMs = 16.f; s.valueMs = 20.f;
    REQUIRE(s.overBudget());
}

TEST_CASE("PvvFrame validity and totalMs", "[Editor][S156]") {
    PvvFrame f;
    REQUIRE(!f.isValid());
    f.frameId = 1;
    REQUIRE(f.isValid());
    PvvSample s; s.id = 1; s.label = "X"; s.valueMs = 5.f;
    f.samples.push_back(s);
    REQUIRE(f.totalMs() == Approx(5.f));
}

TEST_CASE("ProfilerViewV1 beginFrame and endFrame", "[Editor][S156]") {
    ProfilerViewV1 pv;
    REQUIRE(pv.beginFrame(1));
    REQUIRE(pv.endFrame());
    REQUIRE(pv.frameCount() == 1);
}

TEST_CASE("ProfilerViewV1 cannot begin two frames", "[Editor][S156]") {
    ProfilerViewV1 pv;
    REQUIRE(pv.beginFrame(1));
    REQUIRE(!pv.beginFrame(2));
    pv.endFrame();
}

TEST_CASE("ProfilerViewV1 endFrame without beginFrame fails", "[Editor][S156]") {
    ProfilerViewV1 pv;
    REQUIRE(!pv.endFrame());
}

TEST_CASE("ProfilerViewV1 addSample inside frame", "[Editor][S156]") {
    ProfilerViewV1 pv;
    pv.beginFrame(1);
    PvvSample s; s.id = 1; s.label = "Physics"; s.valueMs = 3.f;
    REQUIRE(pv.addSample(s));
    pv.endFrame();
    auto* f = pv.getFrame(1);
    REQUIRE(f != nullptr);
    REQUIRE(f->samples.size() == 1);
}

TEST_CASE("ProfilerViewV1 addSample outside frame fails", "[Editor][S156]") {
    ProfilerViewV1 pv;
    PvvSample s; s.id = 1; s.label = "X";
    REQUIRE(!pv.addSample(s));
}

TEST_CASE("ProfilerViewV1 getFrame by id", "[Editor][S156]") {
    ProfilerViewV1 pv;
    pv.beginFrame(42);
    pv.endFrame();
    REQUIRE(pv.getFrame(42) != nullptr);
    REQUIRE(pv.getFrame(99) == nullptr);
}

TEST_CASE("ProfilerViewV1 clearHistory", "[Editor][S156]") {
    ProfilerViewV1 pv;
    pv.beginFrame(1); pv.endFrame();
    pv.beginFrame(2); pv.endFrame();
    REQUIRE(pv.frameCount() == 2);
    pv.clearHistory();
    REQUIRE(pv.frameCount() == 0);
}

TEST_CASE("ProfilerViewV1 averageFrameMs", "[Editor][S156]") {
    ProfilerViewV1 pv;
    for (int i = 0; i < 4; ++i) {
        pv.beginFrame(i + 1);
        PvvSample s; s.id = static_cast<uint64_t>(i + 1); s.label = "L"; s.valueMs = 10.f;
        pv.addSample(s);
        pv.endFrame();
    }
    REQUIRE(pv.averageFrameMs() == Approx(10.f));
}

TEST_CASE("ProfilerViewV1 onFrame callback fires on endFrame", "[Editor][S156]") {
    ProfilerViewV1 pv;
    uint32_t cbFrame = 0;
    pv.setOnFrame([&](const PvvFrame& f){ cbFrame = f.frameId; });
    pv.beginFrame(7);
    pv.endFrame();
    REQUIRE(cbFrame == 7);
}

// ── MemoryTrackerV1 ───────────────────────────────────────────────────────────

TEST_CASE("MtvAlloc validity", "[Editor][S156]") {
    MtvAlloc a;
    REQUIRE(!a.isValid());
    a.id = 1; a.bytes = 64;
    REQUIRE(a.isValid());
}

TEST_CASE("MemoryTrackerV1 trackAlloc and allocCount", "[Editor][S156]") {
    MemoryTrackerV1 mt;
    MtvAlloc a; a.id = 1; a.bytes = 128;
    REQUIRE(mt.trackAlloc(a));
    REQUIRE(mt.allocCount() == 1);
}

TEST_CASE("MemoryTrackerV1 reject duplicate alloc id", "[Editor][S156]") {
    MemoryTrackerV1 mt;
    MtvAlloc a; a.id = 2; a.bytes = 64;
    REQUIRE(mt.trackAlloc(a));
    REQUIRE(!mt.trackAlloc(a));
}

TEST_CASE("MemoryTrackerV1 trackFree marks freed", "[Editor][S156]") {
    MemoryTrackerV1 mt;
    MtvAlloc a; a.id = 3; a.bytes = 256;
    mt.trackAlloc(a);
    REQUIRE(mt.liveAllocCount() == 1);
    REQUIRE(mt.trackFree(3));
    REQUIRE(mt.liveAllocCount() == 0);
}

TEST_CASE("MemoryTrackerV1 totalLiveBytes", "[Editor][S156]") {
    MemoryTrackerV1 mt;
    MtvAlloc a; a.id = 1; a.bytes = 100;
    MtvAlloc b; b.id = 2; b.bytes = 200;
    mt.trackAlloc(a); mt.trackAlloc(b);
    REQUIRE(mt.totalLiveBytes() == 300);
    mt.trackFree(1);
    REQUIRE(mt.totalLiveBytes() == 200);
}

TEST_CASE("MemoryTrackerV1 peakBytes", "[Editor][S156]") {
    MemoryTrackerV1 mt;
    MtvAlloc a; a.id = 1; a.bytes = 1000;
    mt.trackAlloc(a);
    mt.trackFree(1);
    REQUIRE(mt.peakBytes() == 1000);
}

TEST_CASE("MemoryTrackerV1 byTag", "[Editor][S156]") {
    MemoryTrackerV1 mt;
    MtvAlloc a; a.id = 1; a.bytes = 512; a.tag = "Texture";
    MtvAlloc b; b.id = 2; b.bytes = 256; b.tag = "Mesh";
    mt.trackAlloc(a); mt.trackAlloc(b);
    REQUIRE(mt.byTag("Texture") == 512);
    REQUIRE(mt.byTag("Mesh") == 256);
    REQUIRE(mt.byTag("Audio") == 0);
}

TEST_CASE("MemoryTrackerV1 clearFreed", "[Editor][S156]") {
    MemoryTrackerV1 mt;
    MtvAlloc a; a.id = 1; a.bytes = 64;
    MtvAlloc b; b.id = 2; b.bytes = 64;
    mt.trackAlloc(a); mt.trackAlloc(b);
    mt.trackFree(1);
    mt.clearFreed();
    REQUIRE(mt.allocCount() == 1);
}

TEST_CASE("MemoryTrackerV1 checkLeaks fires callback for live allocs", "[Editor][S156]") {
    MemoryTrackerV1 mt;
    MtvAlloc a; a.id = 1; a.bytes = 64; a.tag = "leak";
    mt.trackAlloc(a);
    int leakCount = 0;
    mt.setOnLeak([&](const MtvAlloc&){ ++leakCount; });
    mt.checkLeaks();
    REQUIRE(leakCount == 1);
}

// ── DiagnosticPanelV1 ─────────────────────────────────────────────────────────

TEST_CASE("DpvEntry validity", "[Editor][S156]") {
    DpvEntry e;
    REQUIRE(!e.isValid());
    e.id = 1; e.message = "Low FPS";
    REQUIRE(e.isValid());
}

TEST_CASE("DiagnosticPanelV1 addEntry and entryCount", "[Editor][S156]") {
    DiagnosticPanelV1 dp;
    DpvEntry e; e.id = 1; e.message = "Slow frame"; e.severity = DpvSeverity::Warning;
    REQUIRE(dp.addEntry(e));
    REQUIRE(dp.entryCount() == 1);
}

TEST_CASE("DiagnosticPanelV1 removeEntry", "[Editor][S156]") {
    DiagnosticPanelV1 dp;
    DpvEntry e; e.id = 2; e.message = "GPU stall";
    dp.addEntry(e);
    REQUIRE(dp.removeEntry(2));
    REQUIRE(dp.entryCount() == 0);
    REQUIRE(!dp.removeEntry(2));
}

TEST_CASE("DiagnosticPanelV1 dismiss", "[Editor][S156]") {
    DiagnosticPanelV1 dp;
    DpvEntry e; e.id = 3; e.message = "Audio glitch"; e.category = DpvCategory::Audio;
    dp.addEntry(e);
    REQUIRE(dp.dismiss(3));
    REQUIRE(dp.countByCategory(DpvCategory::Audio) == 0);
}

TEST_CASE("DiagnosticPanelV1 dismissAll by category", "[Editor][S156]") {
    DiagnosticPanelV1 dp;
    DpvEntry e1; e1.id = 1; e1.message = "A"; e1.category = DpvCategory::Memory;
    DpvEntry e2; e2.id = 2; e2.message = "B"; e2.category = DpvCategory::Memory;
    DpvEntry e3; e3.id = 3; e3.message = "C"; e3.category = DpvCategory::Network;
    dp.addEntry(e1); dp.addEntry(e2); dp.addEntry(e3);
    dp.dismissAll(DpvCategory::Memory);
    REQUIRE(dp.countByCategory(DpvCategory::Memory) == 0);
    REQUIRE(dp.countByCategory(DpvCategory::Network) == 1);
}

TEST_CASE("DiagnosticPanelV1 countBySeverity", "[Editor][S156]") {
    DiagnosticPanelV1 dp;
    DpvEntry e1; e1.id = 1; e1.message = "A"; e1.severity = DpvSeverity::Error;
    DpvEntry e2; e2.id = 2; e2.message = "B"; e2.severity = DpvSeverity::Warning;
    DpvEntry e3; e3.id = 3; e3.message = "C"; e3.severity = DpvSeverity::Error;
    dp.addEntry(e1); dp.addEntry(e2); dp.addEntry(e3);
    REQUIRE(dp.countBySeverity(DpvSeverity::Error) == 2);
    REQUIRE(dp.countBySeverity(DpvSeverity::Warning) == 1);
    REQUIRE(dp.countBySeverity(DpvSeverity::Fatal) == 0);
}

TEST_CASE("DiagnosticPanelV1 clearDismissed", "[Editor][S156]") {
    DiagnosticPanelV1 dp;
    DpvEntry e1; e1.id = 1; e1.message = "A";
    DpvEntry e2; e2.id = 2; e2.message = "B";
    dp.addEntry(e1); dp.addEntry(e2);
    dp.dismiss(1);
    dp.clearDismissed();
    REQUIRE(dp.entryCount() == 1);
}

TEST_CASE("DiagnosticPanelV1 filterBySeverity", "[Editor][S156]") {
    DiagnosticPanelV1 dp;
    DpvEntry e1; e1.id = 1; e1.message = "A"; e1.severity = DpvSeverity::Info;
    DpvEntry e2; e2.id = 2; e2.message = "B"; e2.severity = DpvSeverity::Error;
    DpvEntry e3; e3.id = 3; e3.message = "C"; e3.severity = DpvSeverity::Fatal;
    dp.addEntry(e1); dp.addEntry(e2); dp.addEntry(e3);
    auto filtered = dp.filterBySeverity(DpvSeverity::Error);
    REQUIRE(filtered.size() == 2);
}

TEST_CASE("DiagnosticPanelV1 onEntry callback fires on addEntry", "[Editor][S156]") {
    DiagnosticPanelV1 dp;
    uint32_t cbId = 0;
    dp.setOnEntry([&](const DpvEntry& e){ cbId = e.id; });
    DpvEntry e; e.id = 99; e.message = "Perf drop";
    dp.addEntry(e);
    REQUIRE(cbId == 99);
}
