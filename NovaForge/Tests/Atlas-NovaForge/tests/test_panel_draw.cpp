#include "../editor/panels/ConsolePanel.h"
#include "../editor/panels/ProfilerPanel.h"
#include "../editor/panels/ReplayTimelinePanel.h"
#include "../editor/panels/SaveFileBrowserPanel.h"
#include "../editor/panels/ProjectPickerPanel.h"
#include "../editor/panels/CIDashboardPanel.h"
#include "../editor/panels/InteractionDebugPanel.h"
#include "../editor/panels/AtlasAssistantPanel.h"
#include "../editor/panels/GameMechanicsUIPanel.h"
#include "../editor/panels/AIDiffViewerPanel.h"
#include "../editor/panels/ProofViewerPanel.h"
#include "../editor/tools/GamePackagerPanel.h"
#include "../editor/tools/TileEditorModule.h"
#include <iostream>
#include <cassert>

using namespace atlas::editor;

// --- ConsolePanel Draw ---
void test_console_panel_draw_empty() {
    atlas::ecs::World world;
    atlas::net::NetContext net;
    atlas::sim::TickScheduler sched;
    ConsolePanel panel(world, net, sched);
    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);
    std::cout << "[PASS] test_console_panel_draw_empty" << std::endl;
}

void test_console_panel_draw_with_history() {
    atlas::ecs::World world;
    atlas::net::NetContext net;
    atlas::sim::TickScheduler sched;
    ConsolePanel panel(world, net, sched);
    panel.AddLine("Hello, Atlas");
    panel.Execute("help");
    panel.Draw();
    // Should have background + title + history lines + input field
    assert(panel.GetDrawList().CommandCount() >= 4);
    std::cout << "[PASS] test_console_panel_draw_with_history" << std::endl;
}

// --- ProfilerPanel Draw ---
void test_profiler_panel_draw_empty() {
    ProfilerPanel panel;
    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);
    std::cout << "[PASS] test_profiler_panel_draw_empty" << std::endl;
}

void test_profiler_panel_draw_with_data() {
    ProfilerPanel panel;
    FrameTiming ft;
    ft.frameNumber = 1;
    ft.frameDurationMs = 16.0;
    panel.RecordFrame(ft);

    SystemMetric sm;
    sm.systemName = "Physics";
    sm.durationMs = 4.0;
    panel.RecordSystemMetric(sm);

    panel.Draw();
    // Background + title + summary + bar + system metric
    assert(panel.GetDrawList().CommandCount() >= 5);
    std::cout << "[PASS] test_profiler_panel_draw_with_data" << std::endl;
}

// --- ReplayTimelinePanel Draw ---
void test_replay_timeline_panel_draw_empty() {
    ReplayTimelinePanel panel;
    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);
    std::cout << "[PASS] test_replay_timeline_panel_draw_empty" << std::endl;
}

void test_replay_timeline_panel_draw_with_markers() {
    ReplayTimelinePanel panel;
    std::vector<atlas::sim::ReplayFrame> frames;
    for (uint32_t i = 0; i < 100; ++i) {
        atlas::sim::ReplayFrame f;
        f.tick = i;
        f.stateHash = i * 42;
        frames.push_back(f);
    }
    panel.LoadReplay(frames, 60);
    panel.SetCurrentTick(50);
    panel.AddMarker({25, "Checkpoint", MarkerType::Bookmark});
    panel.AddMarker({75, "Branch", MarkerType::Branch});
    panel.Draw();
    // Should have background + title + bar + progress + markers
    assert(panel.GetDrawList().CommandCount() >= 6);
    std::cout << "[PASS] test_replay_timeline_panel_draw_with_markers" << std::endl;
}

// --- SaveFileBrowserPanel Draw ---
void test_save_file_browser_draw() {
    SaveFileBrowserPanel panel;
    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);
    std::cout << "[PASS] test_save_file_browser_draw" << std::endl;
}

// --- ProjectPickerPanel Draw ---
void test_project_picker_draw() {
    ProjectPickerPanel panel;
    panel.AddRecentProject("/home/test/project1.atlas");
    panel.AddRecentProject("/home/test/project2.atlas");
    panel.Draw();
    // Background + title + header + 2 projects
    assert(panel.GetDrawList().CommandCount() >= 5);
    std::cout << "[PASS] test_project_picker_draw" << std::endl;
}

// --- CIDashboardPanel Draw ---
void test_ci_dashboard_draw_empty() {
    CIDashboardPanel panel;
    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);
    std::cout << "[PASS] test_ci_dashboard_draw_empty" << std::endl;
}

void test_ci_dashboard_draw_with_runs() {
    CIDashboardPanel panel;
    panel.StartPipelineRun("abc123", "main");
    CICheckResult check;
    check.name = "determinism";
    check.status = CICheckStatus::Passed;
    panel.AddCheckResult(check);
    panel.CompletePipelineRun();
    panel.Draw();
    // Background + title + summary + run line
    assert(panel.GetDrawList().CommandCount() >= 4);
    std::cout << "[PASS] test_ci_dashboard_draw_with_runs" << std::endl;
}

// --- InteractionDebugPanel Draw ---
void test_interaction_debug_draw() {
    InteractionDebugPanel panel;
    panel.LogInteraction({"Player", "Hello", "greet", 0.95f});
    panel.Draw();
    // Background + title + headers + entry columns
    assert(panel.GetDrawList().CommandCount() >= 6);
    std::cout << "[PASS] test_interaction_debug_draw" << std::endl;
}

// --- AtlasAssistantPanel Draw ---
void test_atlas_assistant_draw() {
    AtlasAssistantPanel panel;
    AssistantPrompt prompt;
    prompt.query = "Explain ECS";
    panel.SubmitPrompt(prompt);
    AssistantSuggestion suggestion;
    suggestion.title = "ECS Overview";
    suggestion.description = "Entity-Component-System pattern";
    panel.AddSuggestion(suggestion);
    panel.Draw();
    assert(panel.GetDrawList().CommandCount() >= 4);
    std::cout << "[PASS] test_atlas_assistant_draw" << std::endl;
}

// --- GameMechanicsUIPanel Draw ---
void test_game_mechanics_ui_draw() {
    GameMechanicsUIPanel panel;
    MechanicsUIElement elem;
    elem.name = "HealthBar";
    elem.elementType = "ProgressBar";
    panel.AddElement(elem);
    panel.Draw();
    // Background + title + element
    assert(panel.GetDrawList().CommandCount() >= 3);
    std::cout << "[PASS] test_game_mechanics_ui_draw" << std::endl;
}

// --- AIDiffViewerPanel Draw ---
void test_ai_diff_viewer_draw_no_diff() {
    AIDiffViewerPanel panel;
    panel.Draw();
    // Should show "No diff loaded"
    assert(panel.GetDrawList().CommandCount() >= 3);
    std::cout << "[PASS] test_ai_diff_viewer_draw_no_diff" << std::endl;
}

void test_ai_diff_viewer_draw_with_diff() {
    AIDiffViewerPanel panel;
    DiffSet diff;
    diff.title = "AI suggestion";
    DiffFile file;
    file.path = "src/main.cpp";
    DiffHunk hunk;
    hunk.lineStart = 10;
    hunk.lineCount = 5;
    file.hunks.push_back(hunk);
    diff.files.push_back(file);
    panel.LoadDiffSet(diff);
    panel.Draw();
    assert(panel.GetDrawList().CommandCount() >= 5);
    std::cout << "[PASS] test_ai_diff_viewer_draw_with_diff" << std::endl;
}

// --- ProofViewerPanel Draw ---
void test_proof_viewer_draw() {
    ProofViewerPanel panel;
    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);
    std::cout << "[PASS] test_proof_viewer_draw" << std::endl;
}

// --- GamePackagerPanel Draw ---
void test_game_packager_draw() {
    GamePackagerPanel panel;
    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);
    std::cout << "[PASS] test_game_packager_draw" << std::endl;
}

// --- TileEditorModule HandleInput ---
void test_tileeditor_handle_input_paint() {
    TileEditorModule module;
    module.OnRegister();
    assert(module.GetMode() == TileEditorMode::Paint);
    bool handled = module.HandleInput('E', true);
    assert(handled);
    assert(module.GetMode() == TileEditorMode::Erase);
    std::cout << "[PASS] test_tileeditor_handle_input_paint" << std::endl;
}

void test_tileeditor_handle_input_all_modes() {
    TileEditorModule module;
    module.OnRegister();

    module.HandleInput('P', true);
    assert(module.GetMode() == TileEditorMode::Paint);
    module.HandleInput('E', true);
    assert(module.GetMode() == TileEditorMode::Erase);
    module.HandleInput('S', true);
    assert(module.GetMode() == TileEditorMode::Select);
    module.HandleInput('L', true);
    assert(module.GetMode() == TileEditorMode::LayerEdit);
    module.HandleInput('R', true);
    assert(module.GetMode() == TileEditorMode::RuleEdit);
    std::cout << "[PASS] test_tileeditor_handle_input_all_modes" << std::endl;
}

void test_tileeditor_handle_input_unhandled() {
    TileEditorModule module;
    module.OnRegister();
    bool handled = module.HandleInput('X', true);
    assert(!handled);
    // Key release should not be handled
    handled = module.HandleInput('P', false);
    assert(!handled);
    std::cout << "[PASS] test_tileeditor_handle_input_unhandled" << std::endl;
}

// --- Runner ---
void run_panel_draw_tests() {
    std::cout << "\n--- Panel Draw Tests ---" << std::endl;
    test_console_panel_draw_empty();
    test_console_panel_draw_with_history();
    test_profiler_panel_draw_empty();
    test_profiler_panel_draw_with_data();
    test_replay_timeline_panel_draw_empty();
    test_replay_timeline_panel_draw_with_markers();
    test_save_file_browser_draw();
    test_project_picker_draw();
    test_ci_dashboard_draw_empty();
    test_ci_dashboard_draw_with_runs();
    test_interaction_debug_draw();
    test_atlas_assistant_draw();
    test_game_mechanics_ui_draw();
    test_ai_diff_viewer_draw_no_diff();
    test_ai_diff_viewer_draw_with_diff();
    test_proof_viewer_draw();
    test_game_packager_draw();

    std::cout << "\n--- TileEditor HandleInput Tests ---" << std::endl;
    test_tileeditor_handle_input_paint();
    test_tileeditor_handle_input_all_modes();
    test_tileeditor_handle_input_unhandled();
}
