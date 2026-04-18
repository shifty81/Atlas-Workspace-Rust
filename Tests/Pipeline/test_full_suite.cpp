#include <catch2/catch_test_macros.hpp>
#include "NF/Pipeline/Pipeline.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static fs::path makeTempDir(std::string_view name) {
    static std::atomic<uint64_t> s_seq{0};
    auto ts  = static_cast<uint64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    auto idx = s_seq.fetch_add(1);
    std::string unique = std::string(name) + "_" +
                         std::to_string(ts)  + "_" +
                         std::to_string(idx);
    auto dir = fs::temp_directory_path() / "nf_suite_tests" / unique;
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

static void createDummyAsset(const fs::path& fullPath) {
    fs::create_directories(fullPath.parent_path());
    std::ofstream ofs(fullPath, std::ios::binary);
    const char data[] = "glTF\x02\x00\x00\x00";
    ofs.write(data, sizeof(data));
}

// ═══════════════════════════════════════════════════════════════════
// S5: Full Suite Validation
//
// All five tool adapters + BlenderBridge + WorkspaceBroker +
// AtlasAIReasoner active simultaneously.  Events propagate through
// the entire pipeline without interference or feedback loops.
// ═══════════════════════════════════════════════════════════════════

TEST_CASE("S5: All tools active — AssetImported propagates correctly", "[Pipeline][Suite]") {
    auto dirs = makeTempPipeline("suite_asset");

    // Set up all components.
    NF::Manifest manifest;
    NF::BlenderBridge bridge(manifest);

    NF::WorkspaceBroker broker;
    auto sessionId = broker.createSession("SuiteTest");

    NF::AtlasAIReasoner reasoner;
    reasoner.loadDefaultRules();

    NF::ToolRegistry registry;
    registry.registerTool(std::make_unique<NF::BlenderGenAdapter>());
    registry.registerTool(std::make_unique<NF::ContractScannerAdapter>());
    registry.registerTool(std::make_unique<NF::ReplayMinimizerAdapter>());
    registry.registerTool(std::make_unique<NF::AtlasAIAdapter>());
    REQUIRE(registry.toolCount() == 4);

    NF::PipelineWatcher watcher(dirs.changes);
    registry.attach(watcher, dirs);
    broker.attachToWatcher(watcher, sessionId, dirs);
    reasoner.attachToWatcher(watcher, dirs);

    // Create a dummy asset and write an AssetImported event.
    createDummyAsset(dirs.assets / "frigate.glb");

    NF::ChangeEvent ev;
    ev.tool      = "BlenderGenerator";
    ev.eventType = NF::ChangeEventType::AssetImported;
    ev.path      = "pipeline/assets/frigate.glb";
    ev.timestamp = 1000LL;
    ev.metadata  = "type=mesh";
    REQUIRE(ev.writeToFile(dirs.changes));

    // Also import the asset via the bridge.
    auto importResult = bridge.importAsset(ev, dirs);
    REQUIRE(importResult.status == NF::AssetImportStatus::Registered);

    // Poll the watcher — all subscribers fire.
    int found = watcher.poll();
    REQUIRE(found >= 1);

    // Verify each component processed the event.
    // BlenderGenAdapter + AtlasAIAdapter handled it.
    REQUIRE(registry.tool(0)->handledCount() >= 1);  // BlenderGen
    REQUIRE(registry.tool(3)->handledCount() >= 1);  // AtlasAI

    // WorkspaceBroker analyzed it.
    REQUIRE(broker.totalAnalyses() >= 1);

    // AtlasAIReasoner evaluated rules (R020 matches *.glb AssetImported).
    REQUIRE(reasoner.eventsProcessed() >= 1);

    // Bridge registered the asset.
    REQUIRE(bridge.importCount() == 1);
    REQUIRE(manifest.recordCount() == 1);
}

TEST_CASE("S5: All tools active — ContractIssue propagates correctly", "[Pipeline][Suite]") {
    auto dirs = makeTempPipeline("suite_contract");

    NF::WorkspaceBroker broker;
    auto sessionId = broker.createSession("SuiteTest");

    NF::AtlasAIReasoner reasoner;
    reasoner.loadDefaultRules();

    NF::ToolRegistry registry;
    registry.registerTool(std::make_unique<NF::BlenderGenAdapter>());
    registry.registerTool(std::make_unique<NF::ContractScannerAdapter>());
    registry.registerTool(std::make_unique<NF::ReplayMinimizerAdapter>());
    registry.registerTool(std::make_unique<NF::AtlasAIAdapter>());

    NF::PipelineWatcher watcher(dirs.changes);
    registry.attach(watcher, dirs);
    broker.attachToWatcher(watcher, sessionId, dirs);
    reasoner.attachToWatcher(watcher, dirs);

    // Write a ContractIssue event.
    NF::ChangeEvent ev;
    ev.tool      = "ContractScanner";
    ev.eventType = NF::ChangeEventType::ContractIssue;
    ev.path      = "src/Entity.cpp";
    ev.timestamp = 2000LL;
    ev.metadata  = "scanned=src/Entity.cpp;violations=3";
    REQUIRE(ev.writeToFile(dirs.changes));

    watcher.poll();

    // AtlasAI handles all events including ContractIssue.
    REQUIRE(registry.tool(3)->handledCount() >= 1);

    // WorkspaceBroker analyzed.
    REQUIRE(broker.totalAnalyses() >= 1);

    // AtlasAIReasoner found violations (R001 matches ContractIssue *.cpp).
    REQUIRE(reasoner.violationCount() >= 1);
    REQUIRE_FALSE(reasoner.passesGate());  // Error-level violations
}

TEST_CASE("S5: All tools active — ScriptUpdated propagates correctly", "[Pipeline][Suite]") {
    auto dirs = makeTempPipeline("suite_script");

    NF::WorkspaceBroker broker;
    auto sessionId = broker.createSession("SuiteTest");

    NF::AtlasAIReasoner reasoner;
    reasoner.loadDefaultRules();

    NF::ToolRegistry registry;
    registry.registerTool(std::make_unique<NF::BlenderGenAdapter>());
    registry.registerTool(std::make_unique<NF::ContractScannerAdapter>());
    registry.registerTool(std::make_unique<NF::ReplayMinimizerAdapter>());
    registry.registerTool(std::make_unique<NF::AtlasAIAdapter>());

    NF::PipelineWatcher watcher(dirs.changes);
    registry.attach(watcher, dirs);
    broker.attachToWatcher(watcher, sessionId, dirs);
    reasoner.attachToWatcher(watcher, dirs);

    NF::ChangeEvent ev;
    ev.tool      = "Editor";
    ev.eventType = NF::ChangeEventType::ScriptUpdated;
    ev.path      = "scripts/door.graph";
    ev.timestamp = 3000LL;
    REQUIRE(ev.writeToFile(dirs.changes));

    watcher.poll();

    // ContractScanner handles ScriptUpdated.
    REQUIRE(registry.tool(1)->handledCount() >= 1);
    // AtlasAI handles everything.
    REQUIRE(registry.tool(3)->handledCount() >= 1);

    REQUIRE(broker.totalAnalyses() >= 1);
    REQUIRE(reasoner.eventsProcessed() >= 1);
}

TEST_CASE("S5: All tools active — WorldChanged propagates correctly", "[Pipeline][Suite]") {
    auto dirs = makeTempPipeline("suite_world");

    NF::WorkspaceBroker broker;
    auto sessionId = broker.createSession("SuiteTest");

    NF::AtlasAIReasoner reasoner;
    reasoner.loadDefaultRules();

    NF::ToolRegistry registry;
    registry.registerTool(std::make_unique<NF::BlenderGenAdapter>());
    registry.registerTool(std::make_unique<NF::ContractScannerAdapter>());
    registry.registerTool(std::make_unique<NF::ReplayMinimizerAdapter>());
    registry.registerTool(std::make_unique<NF::AtlasAIAdapter>());

    NF::PipelineWatcher watcher(dirs.changes);
    registry.attach(watcher, dirs);
    broker.attachToWatcher(watcher, sessionId, dirs);
    reasoner.attachToWatcher(watcher, dirs);

    NF::ChangeEvent ev;
    ev.tool      = "Editor";
    ev.eventType = NF::ChangeEventType::WorldChanged;
    ev.path      = "worlds/sector07.json";
    ev.timestamp = 4000LL;
    REQUIRE(ev.writeToFile(dirs.changes));

    watcher.poll();

    // AtlasAI handles all events including WorldChanged.
    REQUIRE(registry.tool(3)->handledCount() >= 1);

    REQUIRE(broker.totalAnalyses() >= 1);
    REQUIRE(reasoner.violationCount() >= 1);  // R010/R011 match worlds/*
}

TEST_CASE("S5: All tools active — ReplayExported propagates correctly", "[Pipeline][Suite]") {
    auto dirs = makeTempPipeline("suite_replay");

    NF::WorkspaceBroker broker;
    auto sessionId = broker.createSession("SuiteTest");

    NF::AtlasAIReasoner reasoner;
    reasoner.loadDefaultRules();

    NF::ToolRegistry registry;
    registry.registerTool(std::make_unique<NF::BlenderGenAdapter>());
    registry.registerTool(std::make_unique<NF::ContractScannerAdapter>());
    registry.registerTool(std::make_unique<NF::ReplayMinimizerAdapter>());
    registry.registerTool(std::make_unique<NF::AtlasAIAdapter>());

    NF::PipelineWatcher watcher(dirs.changes);
    registry.attach(watcher, dirs);
    broker.attachToWatcher(watcher, sessionId, dirs);
    reasoner.attachToWatcher(watcher, dirs);

    NF::ChangeEvent ev;
    ev.tool      = "GameRuntime";
    ev.eventType = NF::ChangeEventType::ReplayExported;
    ev.path      = "replays/match42.replay.json";
    ev.timestamp = 5000LL;
    REQUIRE(ev.writeToFile(dirs.changes));

    watcher.poll();

    // ReplayMinimizer handles ReplayExported.
    REQUIRE(registry.tool(2)->handledCount() >= 1);
    // AtlasAI handles everything.
    REQUIRE(registry.tool(3)->handledCount() >= 1);

    REQUIRE(broker.totalAnalyses() >= 1);
    REQUIRE(reasoner.eventsProcessed() >= 1);
}

TEST_CASE("S5: Full event matrix — all event types through all tools", "[Pipeline][Suite]") {
    auto dirs = makeTempPipeline("suite_full_matrix");

    NF::Manifest manifest;
    NF::BlenderBridge bridge(manifest);

    NF::WorkspaceBroker broker;
    auto sessionId = broker.createSession("FullMatrixTest");

    NF::AtlasAIReasoner reasoner;
    reasoner.loadDefaultRules();

    NF::ToolRegistry registry;
    registry.registerTool(std::make_unique<NF::BlenderGenAdapter>());
    registry.registerTool(std::make_unique<NF::ContractScannerAdapter>());
    registry.registerTool(std::make_unique<NF::ReplayMinimizerAdapter>());
    registry.registerTool(std::make_unique<NF::AtlasAIAdapter>());

    // Dispatch all 6 active event types through the registry.
    struct TestEvent {
        NF::ChangeEventType type;
        std::string path;
        int expectedHandlers;  // How many adapters should handle this
    };
    std::vector<TestEvent> events = {
        {NF::ChangeEventType::AssetImported,     "assets/a.glb",           2},  // BlenderGen + AtlasAI
        {NF::ChangeEventType::ScriptUpdated,     "scripts/b.graph",       2},  // ContractScanner + AtlasAI
        {NF::ChangeEventType::ReplayExported,    "replays/c.replay.json", 2},  // ReplayMinimizer + AtlasAI
        {NF::ChangeEventType::ContractIssue,     "src/d.cpp",             1},  // AtlasAI only
        {NF::ChangeEventType::WorldChanged,      "worlds/e.json",         1},  // AtlasAI only
        {NF::ChangeEventType::AnimationExported, "anims/f.glb",           1},  // AtlasAI only
    };

    for (const auto& te : events) {
        NF::ChangeEvent ev;
        ev.tool      = "TestHarness";
        ev.eventType = te.type;
        ev.path      = te.path;
        ev.timestamp = 1000LL;

        int handled = registry.dispatch(ev, dirs);
        REQUIRE(handled == te.expectedHandlers);

        // Also route through the broker and reasoner.
        broker.analyzeEvent(sessionId, ev, dirs);
        reasoner.processEvent(ev, dirs);
    }

    // All 6 events analyzed by the broker.
    REQUIRE(broker.totalAnalyses() == 6);

    // All 6 events processed by the reasoner.
    REQUIRE(reasoner.eventsProcessed() == 6);

    // The reasoner should have found violations for several event types.
    REQUIRE(reasoner.violationCount() >= 1);
}

TEST_CASE("S5: No feedback loops — tool-emitted events don't cause infinite chains", "[Pipeline][Suite]") {
    auto dirs = makeTempPipeline("suite_noloops");

    NF::WorkspaceBroker broker;
    auto sessionId = broker.createSession("NoLoopTest");

    NF::AtlasAIReasoner reasoner;
    reasoner.loadDefaultRules();

    NF::PipelineWatcher watcher(dirs.changes);
    broker.attachToWatcher(watcher, sessionId, dirs);
    reasoner.attachToWatcher(watcher, dirs);

    // Write an AtlasAI AIAnalysis event — broker should skip it (its own tool);
    // feedback loop prevention keeps the broker from re-processing its own output.
    NF::ChangeEvent ev1;
    ev1.tool      = "AtlasAI";
    ev1.eventType = NF::ChangeEventType::AIAnalysis;
    ev1.path      = "analysis/x.json";
    ev1.timestamp = 1000LL;
    ev1.writeToFile(dirs.changes);

    // Write an Editor event — broker should process it normally.
    NF::ChangeEvent ev2;
    ev2.tool      = "Editor";
    ev2.eventType = NF::ChangeEventType::WorldChanged;
    ev2.path      = "worlds/sector01.json";
    ev2.timestamp = 2000LL;
    ev2.writeToFile(dirs.changes);

    watcher.poll();

    // Broker skips AtlasAI events but processes Editor events.
    REQUIRE(broker.totalAnalyses() == 1);
    // Reasoner skips AtlasAI events but processes Editor events.
    REQUIRE(reasoner.eventsProcessed() == 1);

    // Key invariant: AtlasAI-emitted events are not re-processed by the broker,
    // preventing infinite feedback loops.
}

TEST_CASE("S5: Manifest persistence after full suite run", "[Pipeline][Suite]") {
    auto dirs = makeTempPipeline("suite_persist");

    NF::Manifest manifest;
    manifest.projectName = "SuiteProject";
    NF::BlenderBridge bridge(manifest);

    // Import several assets.
    createDummyAsset(dirs.assets / "hull.glb");
    createDummyAsset(dirs.animations / "walk.glb");
    createDummyAsset(dirs.assets / "turret.glb");

    auto makeEvent = [](NF::ChangeEventType type, const std::string& path,
                        const std::string& meta) {
        NF::ChangeEvent ev;
        ev.tool      = "BlenderGenerator";
        ev.eventType = type;
        ev.path      = path;
        ev.timestamp = 1000LL;
        ev.metadata  = meta;
        return ev;
    };

    bridge.importAsset(makeEvent(NF::ChangeEventType::AssetImported,
                                  "pipeline/assets/hull.glb", "type=mesh"), dirs);
    bridge.importAsset(makeEvent(NF::ChangeEventType::AnimationExported,
                                  "pipeline/animations/walk.glb", "type=clip"), dirs);
    bridge.importAsset(makeEvent(NF::ChangeEventType::AssetImported,
                                  "pipeline/assets/turret.glb", "type=mesh"), dirs);

    REQUIRE(manifest.recordCount() == 3);

    // Save and reload the manifest.
    REQUIRE(manifest.save(dirs.manifestFile));

    NF::Manifest loaded;
    REQUIRE(loaded.load(dirs.manifestFile));
    REQUIRE(loaded.recordCount() == 3);
    REQUIRE(loaded.projectName == "SuiteProject");
}

TEST_CASE("S5: AtlasAIReasoner CI gate summary after full suite", "[Pipeline][Suite]") {
    auto dirs = makeTempPipeline("suite_gate");

    NF::AtlasAIReasoner reasoner;
    reasoner.loadDefaultRules();

    // Process a mix of events.
    auto makeEvent = [](NF::ChangeEventType type, const std::string& path) {
        NF::ChangeEvent ev;
        ev.tool      = "Editor";
        ev.eventType = type;
        ev.path      = path;
        ev.timestamp = 1000LL;
        return ev;
    };

    reasoner.processEvent(makeEvent(NF::ChangeEventType::AssetImported,
                                     "assets/x.glb"), dirs);
    reasoner.processEvent(makeEvent(NF::ChangeEventType::WorldChanged,
                                     "worlds/sector01.json"), dirs);
    reasoner.processEvent(makeEvent(NF::ChangeEventType::ContractIssue,
                                     "src/Main.cpp"), dirs);

    REQUIRE(reasoner.eventsProcessed() == 3);
    REQUIRE(reasoner.violationCount() >= 1);

    auto summary = reasoner.summary();
    REQUIRE(summary.find("AtlasAI:") != std::string::npos);
    REQUIRE(summary.find("Events processed: 3") != std::string::npos);
}
