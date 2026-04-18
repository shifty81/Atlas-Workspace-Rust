#include <iostream>
#include <cassert>
#include <cstdio>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>

#include "../engine/ai/BehaviorGraph.h"
#include "../engine/sim/SaveSystem.h"
#include "../engine/sim/StateHasher.h"
#include "../engine/sim/ReplayDivergenceInspector.h"
#include "../engine/sim/ReplayRecorder.h"
#include "../editor/panels/SaveFileBrowserPanel.h"
#include "../editor/panels/StateHashDiffPanel.h"
#include "../editor/panels/ReplayTimelinePanel.h"

// --- Helper: simple BehaviorNode for testing ---
namespace {

class TestPassthroughNode : public atlas::ai::BehaviorNode {
public:
    const char* GetName() const override { return "Passthrough"; }
    const char* GetCategory() const override { return "Test"; }
    std::vector<atlas::ai::BehaviorPort> Inputs() const override {
        return { {"in", atlas::ai::BehaviorPinType::Float} };
    }
    std::vector<atlas::ai::BehaviorPort> Outputs() const override {
        return { {"out", atlas::ai::BehaviorPinType::Float} };
    }
    void Evaluate(const atlas::ai::AIContext&,
                  const std::vector<atlas::ai::BehaviorValue>& inputs,
                  std::vector<atlas::ai::BehaviorValue>& outputs) const override {
        if (!inputs.empty() && !inputs[0].data.empty()) {
            outputs[0].type = atlas::ai::BehaviorPinType::Float;
            outputs[0].data = inputs[0].data;
        } else {
            outputs[0].type = atlas::ai::BehaviorPinType::Float;
            outputs[0].data = {42.0f};
        }
    }
};

} // anonymous namespace

// ============================================================
// Task 1: BehaviorGraph Serialization
// ============================================================

void test_behaviorgraph_serialize_state() {
    atlas::ai::BehaviorGraph graph;
    auto id1 = graph.AddNode(std::make_unique<TestPassthroughNode>());
    auto id2 = graph.AddNode(std::make_unique<TestPassthroughNode>());
    atlas::ai::BehaviorEdge edge{id1, 0, id2, 0};
    graph.AddEdge(edge);
    assert(graph.Compile());

    atlas::ai::AIContext ctx{0.5f, 1.0f, 1.0f, 1.0f, 1};
    assert(graph.Execute(ctx));

    auto data = graph.SerializeState();
    assert(!data.empty());

    // Create a new graph with same structure
    atlas::ai::BehaviorGraph graph2;
    auto id1b = graph2.AddNode(std::make_unique<TestPassthroughNode>());
    auto id2b = graph2.AddNode(std::make_unique<TestPassthroughNode>());
    (void)id1b; (void)id2b;

    assert(graph2.DeserializeState(data));
    assert(graph2.IsCompiled());

    // Check output restored
    auto* out = graph2.GetOutput(id1, 0);
    assert(out != nullptr);
    assert(out->data.size() == 1);
    assert(out->data[0] == 42.0f);

    std::cout << "[PASS] test_behaviorgraph_serialize_state" << std::endl;
}

void test_behaviorgraph_serialize_empty() {
    atlas::ai::BehaviorGraph graph;
    auto data = graph.SerializeState();
    assert(!data.empty());

    atlas::ai::BehaviorGraph graph2;
    assert(graph2.DeserializeState(data));
    assert(!graph2.IsCompiled());

    std::cout << "[PASS] test_behaviorgraph_serialize_empty" << std::endl;
}

// ============================================================
// Task 2: Partial-World Saves
// ============================================================

void test_partial_save_and_load() {
    const std::string path = "/tmp/atlas_test_partial.aspw";

    std::vector<atlas::sim::ChunkSaveEntry> chunks;
    atlas::sim::ChunkSaveEntry c1;
    c1.x = 1; c1.y = 2; c1.z = 3;
    c1.data = {0x10, 0x20, 0x30, 0x40};
    chunks.push_back(c1);

    atlas::sim::ChunkSaveEntry c2;
    c2.x = -5; c2.y = 0; c2.z = 10;
    c2.data = {0xAA, 0xBB};
    chunks.push_back(c2);

    atlas::sim::SaveSystem sys;
    auto result = sys.SavePartial(path, 100, 30, 42, chunks);
    assert(result == atlas::sim::SaveResult::Success);

    atlas::sim::SaveSystem sys2;
    result = sys2.LoadPartial(path);
    assert(result == atlas::sim::SaveResult::Success);

    assert(sys2.PartialHeader().saveTick == 100);
    assert(sys2.PartialHeader().tickRate == 30);
    assert(sys2.PartialHeader().seed == 42);
    assert(sys2.PartialHeader().chunkCount == 2);
    assert(sys2.Chunks().size() == 2);
    assert(sys2.Chunks()[0].x == 1);
    assert(sys2.Chunks()[0].y == 2);
    assert(sys2.Chunks()[0].z == 3);
    assert(sys2.Chunks()[0].data == std::vector<uint8_t>({0x10, 0x20, 0x30, 0x40}));
    assert(sys2.Chunks()[1].x == -5);
    assert(sys2.Chunks()[1].data == std::vector<uint8_t>({0xAA, 0xBB}));

    std::remove(path.c_str());
    std::cout << "[PASS] test_partial_save_and_load" << std::endl;
}

void test_partial_save_hash_integrity() {
    const std::string path = "/tmp/atlas_test_partial_corrupt.aspw";

    std::vector<atlas::sim::ChunkSaveEntry> chunks;
    atlas::sim::ChunkSaveEntry c1;
    c1.x = 0; c1.y = 0; c1.z = 0;
    c1.data = {0x01, 0x02, 0x03};
    chunks.push_back(c1);

    atlas::sim::SaveSystem sys;
    assert(sys.SavePartial(path, 50, 60, 7, chunks) == atlas::sim::SaveResult::Success);

    // Corrupt the file by flipping a byte in the chunk data
    {
        std::fstream f(path, std::ios::in | std::ios::out | std::ios::binary);
        assert(f.is_open());
        // Seek past the header to the chunk data area
        f.seekp(static_cast<std::streamoff>(sizeof(atlas::sim::PartialSaveHeader) + 3 * sizeof(int32_t) + sizeof(uint32_t)));
        uint8_t corrupt = 0xFF;
        f.write(reinterpret_cast<char*>(&corrupt), 1);
    }

    atlas::sim::SaveSystem sys2;
    auto result = sys2.LoadPartial(path);
    assert(result == atlas::sim::SaveResult::HashMismatch);

    std::remove(path.c_str());
    std::cout << "[PASS] test_partial_save_hash_integrity" << std::endl;
}

void test_partial_save_empty_chunks() {
    const std::string path = "/tmp/atlas_test_partial_empty.aspw";

    std::vector<atlas::sim::ChunkSaveEntry> chunks;

    atlas::sim::SaveSystem sys;
    assert(sys.SavePartial(path, 0, 30, 0, chunks) == atlas::sim::SaveResult::Success);

    atlas::sim::SaveSystem sys2;
    assert(sys2.LoadPartial(path) == atlas::sim::SaveResult::Success);
    assert(sys2.PartialHeader().chunkCount == 0);
    assert(sys2.Chunks().empty());

    std::remove(path.c_str());
    std::cout << "[PASS] test_partial_save_empty_chunks" << std::endl;
}

// ============================================================
// Task 3: Save File Browser
// ============================================================

void test_save_file_browser_scan() {
    const std::string dir = "/tmp/atlas_test_savebrowser";
    std::filesystem::create_directories(dir);

    // Create a valid .asav file
    atlas::sim::SaveSystem sys;
    std::vector<uint8_t> ecsData = {1, 2, 3};
    sys.Save(dir + "/test1.asav", 100, 30, 42, ecsData, {}, "Test Save");

    // Create a second one
    sys.Save(dir + "/test2.asav", 200, 60, 99, ecsData);

    atlas::editor::SaveFileBrowserPanel panel;
    panel.ScanDirectory(dir);

    assert(panel.Files().size() == 2);
    assert(panel.Directory() == dir);

    bool foundTest1 = false, foundTest2 = false;
    for (const auto& f : panel.Files()) {
        if (f.filename == "test1.asav") {
            assert(f.valid);
            assert(f.saveTick == 100);
            assert(f.metadata == "Test Save");
            foundTest1 = true;
        }
        if (f.filename == "test2.asav") {
            assert(f.valid);
            assert(f.saveTick == 200);
            foundTest2 = true;
        }
    }
    assert(foundTest1 && foundTest2);

    std::filesystem::remove_all(dir);
    std::cout << "[PASS] test_save_file_browser_scan" << std::endl;
}

void test_save_file_browser_empty_dir() {
    const std::string dir = "/tmp/atlas_test_savebrowser_empty";
    std::filesystem::create_directories(dir);

    atlas::editor::SaveFileBrowserPanel panel;
    panel.ScanDirectory(dir);
    assert(panel.Files().empty());
    assert(panel.SelectedIndex() == -1);

    std::filesystem::remove_all(dir);
    std::cout << "[PASS] test_save_file_browser_empty_dir" << std::endl;
}

void test_save_file_browser_select() {
    const std::string dir = "/tmp/atlas_test_savebrowser_sel";
    std::filesystem::create_directories(dir);

    atlas::sim::SaveSystem sys;
    std::vector<uint8_t> ecsData = {5, 6, 7};
    sys.Save(dir + "/a.asav", 10, 30, 1, ecsData);

    atlas::editor::SaveFileBrowserPanel panel;
    panel.ScanDirectory(dir);
    assert(panel.Files().size() == 1);
    assert(panel.SelectedFile() == nullptr);

    panel.SelectFile(0);
    assert(panel.SelectedIndex() == 0);
    assert(panel.SelectedFile() != nullptr);
    assert(panel.SelectedFile()->valid);

    panel.SelectFile(99);
    assert(panel.SelectedIndex() == -1);
    assert(panel.SelectedFile() == nullptr);

    std::filesystem::remove_all(dir);
    std::cout << "[PASS] test_save_file_browser_select" << std::endl;
}

// ============================================================
// Task 4: Hash Ladder Comparison
// ============================================================

void test_hash_ladder_comparison() {
    atlas::sim::StateHasher a, b;
    a.Reset(0);
    b.Reset(0);

    std::vector<uint8_t> state1 = {1, 2, 3};
    std::vector<uint8_t> state2 = {4, 5, 6};
    std::vector<uint8_t> inputs = {0};

    a.AdvanceTick(0, state1, inputs);
    a.AdvanceTick(1, state1, inputs);
    a.AdvanceTick(2, state2, inputs);

    b.AdvanceTick(0, state1, inputs);
    b.AdvanceTick(1, state1, inputs);
    b.AdvanceTick(2, state1, inputs);  // different state

    atlas::editor::StateHashDiffPanel panel;
    auto result = panel.CompareHashLadders(a, b);

    assert(result.totalCount == 3);
    assert(result.matchCount == 2);
    assert(result.firstDivergenceTick >= 0);
    assert(result.entries.size() == 3);
    assert(result.entries[0].matches);
    assert(result.entries[1].matches);
    assert(!result.entries[2].matches);

    std::cout << "[PASS] test_hash_ladder_comparison" << std::endl;
}

void test_hash_ladder_comparison_no_divergence() {
    atlas::sim::StateHasher a, b;
    a.Reset(0);
    b.Reset(0);

    std::vector<uint8_t> state = {10, 20};
    std::vector<uint8_t> inputs = {0};

    a.AdvanceTick(0, state, inputs);
    b.AdvanceTick(0, state, inputs);

    atlas::editor::StateHashDiffPanel panel;
    auto result = panel.CompareHashLadders(a, b);

    assert(result.totalCount == 1);
    assert(result.matchCount == 1);
    assert(result.firstDivergenceTick == -1);
    assert(result.matchPercentage == 100.0);

    std::cout << "[PASS] test_hash_ladder_comparison_no_divergence" << std::endl;
}

// ============================================================
// Task 5: Divergence Detail
// ============================================================

void test_divergence_detail() {
    atlas::sim::StateHasher local, remote;
    local.Reset(0);
    remote.Reset(0);

    std::vector<uint8_t> state1 = {1};
    std::vector<uint8_t> state2 = {2};
    std::vector<uint8_t> inputs = {0};

    local.AdvanceTick(0, state1, inputs);
    local.AdvanceTick(1, state2, inputs);
    remote.AdvanceTick(0, state1, inputs);
    remote.AdvanceTick(1, state1, inputs);

    atlas::editor::StateHashDiffPanel panel;
    panel.SetLocalHasher(&local);
    panel.SetRemoteHasher(&remote);
    panel.Refresh();

    // Set component breakdown at the divergence tick
    atlas::editor::ComponentHashBreakdown breakdown;
    breakdown.tick = static_cast<uint64_t>(panel.FirstDivergenceTick());
    atlas::editor::ComponentHashEntry comp;
    comp.name = "Physics";
    comp.localHash = 111;
    comp.remoteHash = 222;
    comp.matches = false;
    breakdown.components.push_back(comp);
    panel.SetComponentBreakdown(breakdown);

    auto detail = panel.GetDivergenceDetail();
    assert(detail.tick >= 0);
    assert(detail.localHash != 0 || detail.remoteHash != 0);
    assert(detail.divergentComponents.size() == 1);
    assert(detail.divergentComponents[0].name == "Physics");
    assert(!detail.summary.empty());

    std::cout << "[PASS] test_divergence_detail" << std::endl;
}

void test_divergence_detail_no_divergence() {
    atlas::sim::StateHasher local, remote;
    local.Reset(0);
    remote.Reset(0);

    std::vector<uint8_t> state = {1};
    std::vector<uint8_t> inputs = {0};
    local.AdvanceTick(0, state, inputs);
    remote.AdvanceTick(0, state, inputs);

    atlas::editor::StateHashDiffPanel panel;
    panel.SetLocalHasher(&local);
    panel.SetRemoteHasher(&remote);
    panel.Refresh();

    auto detail = panel.GetDivergenceDetail();
    assert(detail.tick == -1);
    assert(detail.summary == "No divergence");
    assert(detail.divergentComponents.empty());

    std::cout << "[PASS] test_divergence_detail_no_divergence" << std::endl;
}

// ============================================================
// Task 6: Auto-Open Divergence Callback
// ============================================================

void test_replay_auto_divergence_callback() {
    atlas::editor::ReplayTimelinePanel panel;

    // Create two diverging frame sets
    std::vector<atlas::sim::ReplayFrame> framesA;
    atlas::sim::ReplayFrame f1; f1.tick = 0; f1.stateHash = 100;
    atlas::sim::ReplayFrame f2; f2.tick = 1; f2.stateHash = 200;
    framesA.push_back(f1);
    framesA.push_back(f2);

    std::vector<atlas::sim::ReplayFrame> framesB;
    atlas::sim::ReplayFrame f3; f3.tick = 0; f3.stateHash = 100;
    atlas::sim::ReplayFrame f4; f4.tick = 1; f4.stateHash = 999;
    framesB.push_back(f3);
    framesB.push_back(f4);

    panel.LoadReplay(framesA, 30);

    int64_t callbackTick = -1;
    panel.SetOnDivergenceDetected([&](int64_t tick) {
        callbackTick = tick;
    });

    panel.CompareWith(framesB);
    panel.CheckAndNotifyDivergence();

    assert(callbackTick == 1);

    std::cout << "[PASS] test_replay_auto_divergence_callback" << std::endl;
}

void test_replay_no_divergence_callback() {
    atlas::editor::ReplayTimelinePanel panel;

    std::vector<atlas::sim::ReplayFrame> frames;
    atlas::sim::ReplayFrame f1; f1.tick = 0; f1.stateHash = 100;
    frames.push_back(f1);

    panel.LoadReplay(frames, 30);

    int64_t callbackTick = -1;
    panel.SetOnDivergenceDetected([&](int64_t tick) {
        callbackTick = tick;
    });

    panel.CompareWith(frames);
    panel.CheckAndNotifyDivergence();

    assert(callbackTick == -1);

    std::cout << "[PASS] test_replay_no_divergence_callback" << std::endl;
}

// ============================================================
// Task 7: Detailed Divergence Report
// ============================================================

void test_detailed_divergence_report() {
    atlas::sim::StateHasher local, remote;
    local.Reset(0);
    remote.Reset(0);

    std::vector<uint8_t> state1 = {1};
    std::vector<uint8_t> state2 = {2};
    std::vector<uint8_t> inputs = {0};

    local.AdvanceTick(0, state1, inputs);
    remote.AdvanceTick(0, state2, inputs);

    std::vector<std::pair<std::string, uint64_t>> localSys = {
        {"Physics", 111}, {"AI", 222}
    };
    std::vector<std::pair<std::string, uint64_t>> remoteSys = {
        {"Physics", 111}, {"AI", 333}
    };

    auto report = atlas::sim::ReplayDivergenceInspector::CompareDetailed(
        local, remote, localSys, remoteSys);

    assert(report.baseReport.divergeTick >= 0);
    assert(report.systemDiffs.size() == 2);
    assert(report.systemDiffs[0].systemName == "Physics");
    assert(report.systemDiffs[0].matches == true);
    assert(report.systemDiffs[1].systemName == "AI");
    assert(report.systemDiffs[1].matches == false);

    std::cout << "[PASS] test_detailed_divergence_report" << std::endl;
}

void test_detailed_divergence_report_no_divergence() {
    atlas::sim::StateHasher local, remote;
    local.Reset(0);
    remote.Reset(0);

    std::vector<uint8_t> state = {1};
    std::vector<uint8_t> inputs = {0};

    local.AdvanceTick(0, state, inputs);
    remote.AdvanceTick(0, state, inputs);

    std::vector<std::pair<std::string, uint64_t>> localSys = {
        {"Physics", 111}, {"AI", 222}
    };
    std::vector<std::pair<std::string, uint64_t>> remoteSys = {
        {"Physics", 111}, {"AI", 222}
    };

    auto report = atlas::sim::ReplayDivergenceInspector::CompareDetailed(
        local, remote, localSys, remoteSys);

    assert(report.baseReport.divergeTick == -1);
    assert(report.systemDiffs.size() == 2);
    assert(report.systemDiffs[0].matches);
    assert(report.systemDiffs[1].matches);

    std::cout << "[PASS] test_detailed_divergence_report_no_divergence" << std::endl;
}
