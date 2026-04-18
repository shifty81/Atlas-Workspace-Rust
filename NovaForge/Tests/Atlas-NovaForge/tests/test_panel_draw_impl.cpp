#include "../editor/panels/AssetBrowserPanel.h"
#include "../editor/panels/NetInspectorPanel.h"
#include "../editor/panels/JobTracePanel.h"
#include "../editor/panels/StateHashDiffPanel.h"
#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>

using namespace atlas::editor;
using namespace atlas::ui;

// Helper to create a temporary directory with asset files
static std::string create_temp_asset_dir() {
    auto dir = std::filesystem::temp_directory_path() / ("atlas_panel_draw_test_" + std::to_string(time(nullptr)));
    std::filesystem::create_directories(dir);
    // AssetRegistry::Scan() only recognizes .atlas and .atlasb extensions.
    std::ofstream(dir / "ship.atlas") << "mesh";
    std::ofstream(dir / "hull.atlas") << "texture";
    return dir.string();
}

// ---------------------------------------------------------------
// AssetBrowserPanel Draw tests
// ---------------------------------------------------------------

void test_asset_browser_draw_empty() {
    atlas::asset::AssetRegistry registry;
    AssetBrowserPanel panel(registry);
    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);

    bool found = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("No assets found") != std::string::npos) {
            found = true;
            break;
        }
    }
    assert(found);
    std::cout << "[PASS] test_asset_browser_draw_empty" << std::endl;
}

void test_asset_browser_draw_with_assets() {
    std::string dir = create_temp_asset_dir();
    atlas::asset::AssetRegistry registry;
    registry.Scan(dir);

    AssetBrowserPanel panel(registry);
    panel.Draw();

    // Should have at least one asset entry rendered
    bool foundAsset = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find(".atlas") != std::string::npos) {
            foundAsset = true;
            break;
        }
    }
    assert(foundAsset);

    std::filesystem::remove_all(dir);
    std::cout << "[PASS] test_asset_browser_draw_with_assets" << std::endl;
}

void test_asset_browser_draw_shows_title() {
    atlas::asset::AssetRegistry registry;
    AssetBrowserPanel panel(registry);
    panel.Draw();

    bool foundTitle = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text == "Asset Browser") {
            foundTitle = true;
            break;
        }
    }
    assert(foundTitle);
    std::cout << "[PASS] test_asset_browser_draw_shows_title" << std::endl;
}

void test_asset_browser_draw_shows_sort_mode() {
    atlas::asset::AssetRegistry registry;
    AssetBrowserPanel panel(registry);

    panel.SetSortMode(AssetSortMode::ByType);
    panel.Draw();

    bool foundSort = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("Sort: Type") != std::string::npos) {
            foundSort = true;
            break;
        }
    }
    assert(foundSort);
    std::cout << "[PASS] test_asset_browser_draw_shows_sort_mode" << std::endl;
}

void test_asset_browser_draw_shows_filter() {
    std::string dir = create_temp_asset_dir();
    atlas::asset::AssetRegistry registry;
    registry.Scan(dir);

    AssetBrowserPanel panel(registry);
    panel.SetFilter("ship");
    panel.Draw();

    bool foundFilter = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("Filter: ship") != std::string::npos) {
            foundFilter = true;
            break;
        }
    }
    assert(foundFilter);

    std::filesystem::remove_all(dir);
    std::cout << "[PASS] test_asset_browser_draw_shows_filter" << std::endl;
}

void test_asset_browser_draw_clears_between_frames() {
    std::string dir = create_temp_asset_dir();
    atlas::asset::AssetRegistry registry;
    registry.Scan(dir);

    AssetBrowserPanel panel(registry);
    panel.Draw();
    size_t first = panel.GetDrawList().CommandCount();
    assert(first > 0);

    panel.Draw();
    size_t second = panel.GetDrawList().CommandCount();
    assert(second == first);

    std::filesystem::remove_all(dir);
    std::cout << "[PASS] test_asset_browser_draw_clears_between_frames" << std::endl;
}

// ---------------------------------------------------------------
// NetInspectorPanel Draw tests
// ---------------------------------------------------------------

void test_net_inspector_draw_standalone_mode() {
    atlas::net::NetContext net;
    net.Init(atlas::net::NetMode::Standalone);
    NetInspectorPanel panel(net);
    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);

    bool foundTitle = false, foundMode = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text) {
            if (cmd.text == "Network Inspector") foundTitle = true;
            if (cmd.text.find("Mode: Standalone") != std::string::npos) foundMode = true;
        }
    }
    assert(foundTitle);
    assert(foundMode);
    std::cout << "[PASS] test_net_inspector_draw_standalone_mode" << std::endl;
}

void test_net_inspector_draw_no_peers_msg() {
    atlas::net::NetContext net;
    net.Init(atlas::net::NetMode::Standalone);
    NetInspectorPanel panel(net);
    panel.Draw();

    bool foundNoPeers = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("No peers") != std::string::npos) {
            foundNoPeers = true;
            break;
        }
    }
    assert(foundNoPeers);
    std::cout << "[PASS] test_net_inspector_draw_no_peers_msg" << std::endl;
}

void test_net_inspector_draw_with_peers_count() {
    atlas::net::NetContext net;
    net.Init(atlas::net::NetMode::Server);
    net.AddPeer();
    net.AddPeer();

    NetInspectorPanel panel(net);
    panel.Draw();

    bool foundPeers = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("Peers: 2") != std::string::npos) {
            foundPeers = true;
            break;
        }
    }
    assert(foundPeers);
    std::cout << "[PASS] test_net_inspector_draw_with_peers_count" << std::endl;
}

void test_net_inspector_draw_rtt_display() {
    atlas::net::NetContext net;
    net.Init(atlas::net::NetMode::Standalone);
    NetInspectorPanel panel(net);
    panel.Draw();

    bool foundRtt = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("Avg RTT:") != std::string::npos) {
            foundRtt = true;
            break;
        }
    }
    assert(foundRtt);
    std::cout << "[PASS] test_net_inspector_draw_rtt_display" << std::endl;
}

void test_net_inspector_draw_frame_clear() {
    atlas::net::NetContext net;
    net.Init(atlas::net::NetMode::Standalone);
    NetInspectorPanel panel(net);

    panel.Draw();
    size_t first = panel.GetDrawList().CommandCount();
    assert(first > 0);

    panel.Draw();
    size_t second = panel.GetDrawList().CommandCount();
    assert(second == first);
    std::cout << "[PASS] test_net_inspector_draw_frame_clear" << std::endl;
}

// ---------------------------------------------------------------
// JobTracePanel Draw tests
// ---------------------------------------------------------------

void test_job_trace_draw_no_tracer() {
    JobTracePanel panel;
    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);

    bool found = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("No tracer attached") != std::string::npos) {
            found = true;
            break;
        }
    }
    assert(found);
    std::cout << "[PASS] test_job_trace_draw_no_tracer" << std::endl;
}

void test_job_trace_draw_empty_trace() {
    atlas::sim::JobTracer tracer;
    JobTracePanel panel;
    panel.SetTracer(&tracer);
    panel.Draw();

    bool found = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("No traces recorded") != std::string::npos) {
            found = true;
            break;
        }
    }
    assert(found);
    std::cout << "[PASS] test_job_trace_draw_empty_trace" << std::endl;
}

void test_job_trace_draw_with_entries() {
    atlas::sim::JobTracer tracer;
    tracer.BeginTick(1);
    tracer.RecordSystemStart("Physics");
    tracer.RecordSystemEnd();
    tracer.RecordSystemStart("AI");
    tracer.RecordSystemEnd();
    tracer.EndTick();

    JobTracePanel panel;
    panel.SetTracer(&tracer);
    panel.Draw();

    bool foundTitle = false, foundConsistent = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text) {
            if (cmd.text == "Job Trace") foundTitle = true;
            if (cmd.text.find("consistent") != std::string::npos) foundConsistent = true;
        }
    }
    assert(foundTitle);
    assert(foundConsistent);
    std::cout << "[PASS] test_job_trace_draw_with_entries" << std::endl;
}

void test_job_trace_draw_mismatch_highlight() {
    atlas::sim::JobTracer tracer;
    tracer.BeginTick(1);
    tracer.RecordSystemStart("Physics");
    tracer.RecordSystemEnd();
    tracer.EndTick();

    atlas::sim::JobTracer ref;
    ref.BeginTick(1);
    ref.RecordSystemStart("AI");
    ref.RecordSystemEnd();
    ref.EndTick();

    JobTracePanel panel;
    panel.SetTracer(&tracer);
    panel.SetReferenceTracer(&ref);
    panel.Draw();

    bool foundMismatch = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("MISMATCH") != std::string::npos) {
            foundMismatch = true;
            break;
        }
    }
    assert(foundMismatch);
    std::cout << "[PASS] test_job_trace_draw_mismatch_highlight" << std::endl;
}

void test_job_trace_draw_shows_title() {
    JobTracePanel panel;
    panel.Draw();

    bool found = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text && cmd.text == "Job Trace") {
            found = true;
            break;
        }
    }
    assert(found);
    std::cout << "[PASS] test_job_trace_draw_shows_title" << std::endl;
}

void test_job_trace_draw_clears_between_frames() {
    atlas::sim::JobTracer tracer;
    tracer.BeginTick(1);
    tracer.RecordSystemStart("Physics");
    tracer.RecordSystemEnd();
    tracer.EndTick();

    JobTracePanel panel;
    panel.SetTracer(&tracer);

    panel.Draw();
    size_t first = panel.GetDrawList().CommandCount();
    assert(first > 0);

    panel.Draw();
    size_t second = panel.GetDrawList().CommandCount();
    assert(second == first);
    std::cout << "[PASS] test_job_trace_draw_clears_between_frames" << std::endl;
}

void test_job_trace_is_editor_panel() {
    JobTracePanel panel;
    EditorPanel* base = &panel;
    assert(std::string(base->Name()) == "Job Trace");
    std::cout << "[PASS] test_job_trace_is_editor_panel" << std::endl;
}

// ---------------------------------------------------------------
// StateHashDiffPanel Draw tests
// ---------------------------------------------------------------

void test_state_hash_diff_draw_no_hashers() {
    StateHashDiffPanel panel;
    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);

    bool found = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("No hashers attached") != std::string::npos) {
            found = true;
            break;
        }
    }
    assert(found);
    std::cout << "[PASS] test_state_hash_diff_draw_no_hashers" << std::endl;
}

void test_state_hash_diff_draw_no_entries() {
    atlas::sim::StateHasher local, remote;
    local.Reset(0);
    remote.Reset(0);

    StateHashDiffPanel panel;
    panel.SetLocalHasher(&local);
    panel.SetRemoteHasher(&remote);
    panel.Draw();

    bool found = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("No hash entries") != std::string::npos) {
            found = true;
            break;
        }
    }
    assert(found);
    std::cout << "[PASS] test_state_hash_diff_draw_no_entries" << std::endl;
}

void test_state_hash_diff_draw_matching() {
    atlas::sim::StateHasher local, remote;
    local.Reset(42);
    remote.Reset(42);
    std::vector<uint8_t> state = {1, 2, 3};
    std::vector<uint8_t> inputs = {4, 5};
    local.AdvanceTick(1, state, inputs);
    remote.AdvanceTick(1, state, inputs);

    StateHashDiffPanel panel;
    panel.SetLocalHasher(&local);
    panel.SetRemoteHasher(&remote);
    panel.Draw();

    bool foundNoDiv = false, foundMatch = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text) {
            if (cmd.text.find("No divergence") != std::string::npos) foundNoDiv = true;
            if (cmd.text.find("Match") != std::string::npos) foundMatch = true;
        }
    }
    assert(foundNoDiv);
    assert(foundMatch);
    std::cout << "[PASS] test_state_hash_diff_draw_matching" << std::endl;
}

void test_state_hash_diff_draw_divergent() {
    atlas::sim::StateHasher local, remote;
    local.Reset(42);
    remote.Reset(99);
    std::vector<uint8_t> state = {1, 2, 3};
    std::vector<uint8_t> inputs = {4, 5};
    local.AdvanceTick(1, state, inputs);
    remote.AdvanceTick(1, state, inputs);

    StateHashDiffPanel panel;
    panel.SetLocalHasher(&local);
    panel.SetRemoteHasher(&remote);
    panel.Draw();

    bool foundDiv = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("DIVERGENT") != std::string::npos) {
            foundDiv = true;
            break;
        }
    }
    assert(foundDiv);
    std::cout << "[PASS] test_state_hash_diff_draw_divergent" << std::endl;
}

void test_state_hash_diff_draw_shows_title() {
    StateHashDiffPanel panel;
    panel.Draw();

    bool found = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text && cmd.text == "State Hash Diff") {
            found = true;
            break;
        }
    }
    assert(found);
    std::cout << "[PASS] test_state_hash_diff_draw_shows_title" << std::endl;
}

void test_state_hash_diff_draw_per_system_breakdown() {
    atlas::sim::StateHasher local, remote;
    local.Reset(42);
    remote.Reset(99);
    std::vector<uint8_t> state = {1, 2, 3};
    std::vector<uint8_t> inputs = {4, 5};
    local.AdvanceTick(1, state, inputs);
    remote.AdvanceTick(1, state, inputs);

    StateHashDiffPanel panel;
    panel.SetLocalHasher(&local);
    panel.SetRemoteHasher(&remote);

    PerSystemHashBreakdown breakdown;
    breakdown.tick = 1;
    breakdown.divergentSystems = {"Physics", "AI"};
    panel.SetPerSystemBreakdown(breakdown);
    panel.Draw();

    bool foundBreakdown = false, foundPhysics = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text) {
            if (cmd.text.find("Per-System Breakdown") != std::string::npos) foundBreakdown = true;
            if (cmd.text.find("Physics") != std::string::npos) foundPhysics = true;
        }
    }
    assert(foundBreakdown);
    assert(foundPhysics);
    std::cout << "[PASS] test_state_hash_diff_draw_per_system_breakdown" << std::endl;
}

void test_state_hash_diff_draw_clears_between_frames() {
    atlas::sim::StateHasher local, remote;
    local.Reset(42);
    remote.Reset(42);
    std::vector<uint8_t> state = {1, 2, 3};
    std::vector<uint8_t> inputs = {4, 5};
    local.AdvanceTick(1, state, inputs);
    remote.AdvanceTick(1, state, inputs);

    StateHashDiffPanel panel;
    panel.SetLocalHasher(&local);
    panel.SetRemoteHasher(&remote);

    panel.Draw();
    size_t first = panel.GetDrawList().CommandCount();
    assert(first > 0);

    panel.Draw();
    size_t second = panel.GetDrawList().CommandCount();
    assert(second == first);
    std::cout << "[PASS] test_state_hash_diff_draw_clears_between_frames" << std::endl;
}

void test_state_hash_diff_is_editor_panel() {
    StateHashDiffPanel panel;
    EditorPanel* base = &panel;
    assert(std::string(base->Name()) == "State Hash Diff");
    std::cout << "[PASS] test_state_hash_diff_is_editor_panel" << std::endl;
}

// ---------------------------------------------------------------
// Runner
// ---------------------------------------------------------------

void register_panel_draw_impl_tests() {
    std::cout << "\n--- AssetBrowserPanel Draw ---" << std::endl;
    test_asset_browser_draw_empty();
    test_asset_browser_draw_with_assets();
    test_asset_browser_draw_shows_title();
    test_asset_browser_draw_shows_sort_mode();
    test_asset_browser_draw_shows_filter();
    test_asset_browser_draw_clears_between_frames();

    std::cout << "\n--- NetInspectorPanel Draw ---" << std::endl;
    test_net_inspector_draw_standalone_mode();
    test_net_inspector_draw_no_peers_msg();
    test_net_inspector_draw_with_peers_count();
    test_net_inspector_draw_rtt_display();
    test_net_inspector_draw_frame_clear();

    std::cout << "\n--- JobTracePanel Draw ---" << std::endl;
    test_job_trace_draw_no_tracer();
    test_job_trace_draw_empty_trace();
    test_job_trace_draw_with_entries();
    test_job_trace_draw_mismatch_highlight();
    test_job_trace_draw_shows_title();
    test_job_trace_draw_clears_between_frames();
    test_job_trace_is_editor_panel();

    std::cout << "\n--- StateHashDiffPanel Draw ---" << std::endl;
    test_state_hash_diff_draw_no_hashers();
    test_state_hash_diff_draw_no_entries();
    test_state_hash_diff_draw_matching();
    test_state_hash_diff_draw_divergent();
    test_state_hash_diff_draw_shows_title();
    test_state_hash_diff_draw_per_system_breakdown();
    test_state_hash_diff_draw_clears_between_frames();
    test_state_hash_diff_is_editor_panel();
}
