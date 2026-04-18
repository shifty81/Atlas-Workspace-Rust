#include <catch2/catch_test_macros.hpp>
#include "NF/Pipeline/Pipeline.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// Helper: create a unique temp directory for each test invocation.
static fs::path makeTempDir(std::string_view name) {
    static std::atomic<uint64_t> s_seq{0};
    auto ts  = static_cast<uint64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    auto idx = s_seq.fetch_add(1);
    std::string unique = std::string(name) + "_" +
                         std::to_string(ts)  + "_" +
                         std::to_string(idx);
    auto dir = fs::temp_directory_path() / "nf_broker_tests" / unique;
    std::error_code ec;
    fs::create_directories(dir, ec);
    return dir;
}

static NF::PipelineDirectories makeTempPipeline(std::string_view name) {
    auto root = makeTempDir(name);
    auto dirs = NF::PipelineDirectories::fromRoot(root);
    dirs.ensureCreated();
    return dirs;
}

// ── SA-1: Session management ──────────────────────────────────────────────

TEST_CASE("WorkspaceBroker creates sessions", "[Pipeline][WorkspaceBroker]") {
    NF::WorkspaceBroker broker;
    REQUIRE(broker.sessionCount() == 0);

    auto id = broker.createSession("TestProject");
    REQUIRE_FALSE(id.empty());
    REQUIRE(broker.sessionCount() == 1);

    auto* s = broker.session(id);
    REQUIRE(s != nullptr);
    REQUIRE(s->projectName == "TestProject");
    REQUIRE(s->active);
    REQUIRE(s->createdAt > 0);
}

TEST_CASE("WorkspaceBroker creates multiple independent sessions", "[Pipeline][WorkspaceBroker]") {
    NF::WorkspaceBroker broker;
    auto id1 = broker.createSession("Project1");
    auto id2 = broker.createSession("Project2");

    REQUIRE(id1 != id2);
    REQUIRE(broker.sessionCount() == 2);
    REQUIRE(broker.session(id1)->projectName == "Project1");
    REQUIRE(broker.session(id2)->projectName == "Project2");
}

TEST_CASE("WorkspaceBroker closes and resumes sessions", "[Pipeline][WorkspaceBroker]") {
    NF::WorkspaceBroker broker;
    auto id = broker.createSession("TestProject");

    REQUIRE(broker.closeSession(id));
    REQUIRE_FALSE(broker.session(id)->active);

    REQUIRE(broker.resumeSession(id));
    REQUIRE(broker.session(id)->active);
}

TEST_CASE("WorkspaceBroker close/resume returns false for unknown session", "[Pipeline][WorkspaceBroker]") {
    NF::WorkspaceBroker broker;
    REQUIRE_FALSE(broker.closeSession("nonexistent"));
    REQUIRE_FALSE(broker.resumeSession("nonexistent"));
}

TEST_CASE("WorkspaceBroker lists active sessions", "[Pipeline][WorkspaceBroker]") {
    NF::WorkspaceBroker broker;
    auto id1 = broker.createSession("P1");
    auto id2 = broker.createSession("P2");
    auto id3 = broker.createSession("P3");

    broker.closeSession(id2);

    auto active = broker.activeSessions();
    REQUIRE(active.size() == 2);

    // Check that id1 and id3 are in the active list.
    bool foundId1 = false, foundId3 = false;
    for (const auto& a : active) {
        if (a == id1) foundId1 = true;
        if (a == id3) foundId3 = true;
    }
    REQUIRE(foundId1);
    REQUIRE(foundId3);
}

TEST_CASE("WorkspaceBroker session lookup returns nullptr for unknown id", "[Pipeline][WorkspaceBroker]") {
    NF::WorkspaceBroker broker;
    REQUIRE(broker.session("nonexistent") == nullptr);
}

// ── SA-2: Context indexing ────────────────────────────────────────────────

TEST_CASE("WorkspaceBroker indexes events per session", "[Pipeline][WorkspaceBroker]") {
    NF::WorkspaceBroker broker;
    auto id = broker.createSession("TestProject");

    NF::ChangeEvent ev;
    ev.tool      = "Editor";
    ev.eventType = NF::ChangeEventType::AssetImported;
    ev.path      = "assets/ship.glb";
    ev.timestamp = 1000LL;

    broker.indexEvent(id, ev);

    REQUIRE(broker.totalEventsIndexed() == 1);
    REQUIRE(broker.eventCountForPath("assets/ship.glb") == 1);
    REQUIRE(broker.lastEventTypeForPath("assets/ship.glb") == NF::ChangeEventType::AssetImported);

    auto* s = broker.session(id);
    REQUIRE(s->requests.size() == 1);
    REQUIRE(s->requests[0].event.path == "assets/ship.glb");
}

TEST_CASE("WorkspaceBroker tracks multiple events on same path", "[Pipeline][WorkspaceBroker]") {
    NF::WorkspaceBroker broker;
    auto id = broker.createSession("TestProject");

    NF::ChangeEvent ev1;
    ev1.tool      = "Editor";
    ev1.eventType = NF::ChangeEventType::ScriptUpdated;
    ev1.path      = "scripts/door.graph";
    ev1.timestamp = 1000LL;

    NF::ChangeEvent ev2;
    ev2.tool      = "ContractScanner";
    ev2.eventType = NF::ChangeEventType::ContractIssue;
    ev2.path      = "scripts/door.graph";
    ev2.timestamp = 2000LL;

    broker.indexEvent(id, ev1);
    broker.indexEvent(id, ev2);

    REQUIRE(broker.eventCountForPath("scripts/door.graph") == 2);
    REQUIRE(broker.lastEventTypeForPath("scripts/door.graph") == NF::ChangeEventType::ContractIssue);
}

TEST_CASE("WorkspaceBroker returns tracked paths sorted", "[Pipeline][WorkspaceBroker]") {
    NF::WorkspaceBroker broker;
    auto id = broker.createSession("TestProject");

    NF::ChangeEvent ev1;
    ev1.tool = "Editor";
    ev1.eventType = NF::ChangeEventType::AssetImported;
    ev1.path = "c.glb";
    ev1.timestamp = 1000LL;

    NF::ChangeEvent ev2 = ev1;
    ev2.path = "a.glb";

    NF::ChangeEvent ev3 = ev1;
    ev3.path = "b.glb";

    broker.indexEvent(id, ev1);
    broker.indexEvent(id, ev2);
    broker.indexEvent(id, ev3);

    auto paths = broker.trackedPaths();
    REQUIRE(paths.size() == 3);
    REQUIRE(paths[0] == "a.glb");
    REQUIRE(paths[1] == "b.glb");
    REQUIRE(paths[2] == "c.glb");
}

TEST_CASE("WorkspaceBroker ignores index for unknown session", "[Pipeline][WorkspaceBroker]") {
    NF::WorkspaceBroker broker;

    NF::ChangeEvent ev;
    ev.tool      = "Editor";
    ev.eventType = NF::ChangeEventType::AssetImported;
    ev.path      = "assets/test.glb";
    ev.timestamp = 1000LL;

    broker.indexEvent("nonexistent", ev);
    REQUIRE(broker.totalEventsIndexed() == 0);
}

TEST_CASE("WorkspaceBroker returns Unknown for untracked path", "[Pipeline][WorkspaceBroker]") {
    NF::WorkspaceBroker broker;
    REQUIRE(broker.eventCountForPath("untracked") == 0);
    REQUIRE(broker.lastEventTypeForPath("untracked") == NF::ChangeEventType::Unknown);
}

// ── SA-3: Analysis requests ───────────────────────────────────────────────

TEST_CASE("WorkspaceBroker analyzes events and returns results", "[Pipeline][WorkspaceBroker]") {
    auto dirs = makeTempPipeline("broker_analyze");
    NF::WorkspaceBroker broker;
    auto id = broker.createSession("TestProject");

    NF::ChangeEvent ev;
    ev.tool      = "BlenderGenerator";
    ev.eventType = NF::ChangeEventType::AssetImported;
    ev.path      = "assets/crate.glb";
    ev.timestamp = 1000LL;
    ev.metadata  = "type=mesh";

    auto result = broker.analyzeEvent(id, ev, dirs);

    REQUIRE(result.success);
    REQUIRE(result.sessionId == id);
    REQUIRE(result.requestPath == "assets/crate.glb");
    REQUIRE(result.sourceEventType == NF::ChangeEventType::AssetImported);
    REQUIRE_FALSE(result.summary.empty());
    REQUIRE_FALSE(result.recommendation.empty());
    REQUIRE(result.analysisTime > 0);

    REQUIRE(broker.totalAnalyses() == 1);
    REQUIRE(broker.totalEventsIndexed() == 1);

    // Session should have the result.
    auto* s = broker.session(id);
    REQUIRE(s->results.size() == 1);
    REQUIRE(s->results[0].requestPath == "assets/crate.glb");
}

TEST_CASE("WorkspaceBroker analyzeEvent fails for unknown session", "[Pipeline][WorkspaceBroker]") {
    auto dirs = makeTempPipeline("broker_fail");
    NF::WorkspaceBroker broker;

    NF::ChangeEvent ev;
    ev.tool      = "Editor";
    ev.eventType = NF::ChangeEventType::WorldChanged;
    ev.path      = "worlds/sector01.json";
    ev.timestamp = 2000LL;

    auto result = broker.analyzeEvent("nonexistent", ev, dirs);

    REQUIRE_FALSE(result.success);
    REQUIRE(result.summary.find("not found") != std::string::npos);
}

TEST_CASE("WorkspaceBroker generates recommendations per event type", "[Pipeline][WorkspaceBroker]") {
    auto dirs = makeTempPipeline("broker_recommendations");
    NF::WorkspaceBroker broker;
    auto id = broker.createSession("TestProject");

    // Test different event types produce different recommendations.
    auto makeEvent = [](NF::ChangeEventType type, const std::string& path) {
        NF::ChangeEvent ev;
        ev.tool      = "Editor";
        ev.eventType = type;
        ev.path      = path;
        ev.timestamp = 1000LL;
        return ev;
    };

    auto r1 = broker.analyzeEvent(id, makeEvent(NF::ChangeEventType::AssetImported, "a.glb"), dirs);
    auto r2 = broker.analyzeEvent(id, makeEvent(NF::ChangeEventType::ScriptUpdated, "s.cpp"), dirs);
    auto r3 = broker.analyzeEvent(id, makeEvent(NF::ChangeEventType::ContractIssue, "c.cpp"), dirs);

    REQUIRE(r1.recommendation.find("Content Browser") != std::string::npos);
    REQUIRE(r2.recommendation.find("ContractScanner") != std::string::npos);
    REQUIRE(r3.recommendation.find("AtlasAI") != std::string::npos);
}

TEST_CASE("WorkspaceBroker analyzeEvent writes response to pipeline", "[Pipeline][WorkspaceBroker]") {
    auto dirs = makeTempPipeline("broker_emit");
    NF::WorkspaceBroker broker;
    auto id = broker.createSession("TestProject");

    NF::ChangeEvent ev;
    ev.tool      = "Editor";
    ev.eventType = NF::ChangeEventType::WorldChanged;
    ev.path      = "worlds/sector05.json";
    ev.timestamp = 3000LL;

    broker.analyzeEvent(id, ev, dirs);

    // Check that a .change.json response was written.
    int fileCount = 0;
    for (const auto& entry : fs::directory_iterator(dirs.changes)) {
        if (entry.path().extension() == ".json") ++fileCount;
    }
    REQUIRE(fileCount >= 1);
}

// ── SA-5: Full pipeline integration ───────────────────────────────────────

TEST_CASE("WorkspaceBroker attaches to PipelineWatcher and processes events", "[Pipeline][WorkspaceBroker]") {
    auto dirs = makeTempPipeline("broker_attach");
    NF::WorkspaceBroker broker;
    auto id = broker.createSession("TestProject");

    NF::PipelineWatcher watcher(dirs.changes);
    broker.attachToWatcher(watcher, id, dirs);

    // Write a .change.json event.
    NF::ChangeEvent ev;
    ev.tool      = "Editor";
    ev.eventType = NF::ChangeEventType::AssetImported;
    ev.path      = "assets/barrel.glb";
    ev.timestamp = 5000LL;
    ev.metadata  = "type=mesh";
    REQUIRE(ev.writeToFile(dirs.changes));

    // Poll the watcher.
    int found = watcher.poll();
    REQUIRE(found >= 1);

    // The broker should have analyzed the event.
    REQUIRE(broker.totalAnalyses() >= 1);
    REQUIRE(broker.totalEventsIndexed() >= 1);
    REQUIRE(broker.eventCountForPath("assets/barrel.glb") >= 1);
}

TEST_CASE("WorkspaceBroker skips AtlasAI events to avoid feedback loops", "[Pipeline][WorkspaceBroker]") {
    auto dirs = makeTempPipeline("broker_noloop");
    NF::WorkspaceBroker broker;
    auto id = broker.createSession("TestProject");

    NF::PipelineWatcher watcher(dirs.changes);
    broker.attachToWatcher(watcher, id, dirs);

    // Write an AtlasAI event (should be skipped — broker's own output).
    NF::ChangeEvent ev;
    ev.tool      = "AtlasAI";
    ev.eventType = NF::ChangeEventType::AIAnalysis;
    ev.path      = "analysis/result.json";
    ev.timestamp = 6000LL;
    REQUIRE(ev.writeToFile(dirs.changes));

    watcher.poll();

    // Broker should NOT have processed this event (feedback loop prevention).
    REQUIRE(broker.totalAnalyses() == 0);
}

TEST_CASE("WorkspaceBroker end-to-end with ToolRegistry", "[Pipeline][WorkspaceBroker]") {
    auto dirs = makeTempPipeline("broker_e2e");
    NF::WorkspaceBroker broker;
    auto id = broker.createSession("TestProject");

    // Set up registry with all tools.
    NF::ToolRegistry registry;
    registry.registerTool(std::make_unique<NF::BlenderGenAdapter>());
    registry.registerTool(std::make_unique<NF::AtlasAIAdapter>());

    NF::PipelineWatcher watcher(dirs.changes);
    registry.attach(watcher, dirs);
    broker.attachToWatcher(watcher, id, dirs);

    // Write an AssetImported event.
    NF::ChangeEvent ev;
    ev.tool      = "Editor";
    ev.eventType = NF::ChangeEventType::AssetImported;
    ev.path      = "assets/turret.glb";
    ev.timestamp = 7000LL;
    ev.metadata  = "type=mesh";
    REQUIRE(ev.writeToFile(dirs.changes));

    watcher.poll();

    // Both the registry and broker should have processed the event.
    REQUIRE(registry.tool(0)->handledCount() >= 1);  // BlenderGenAdapter
    REQUIRE(registry.tool(1)->handledCount() >= 1);  // AtlasAIAdapter
    REQUIRE(broker.totalAnalyses() >= 1);
}
