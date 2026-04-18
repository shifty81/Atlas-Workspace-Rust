// test_editor_panel_registry.cpp — Verify all editor panels can be
// instantiated and satisfy the EditorPanel interface (Name + Draw).

#include "../editor/ui/EditorPanel.h"
#include "../editor/panels/AIDebuggerPanel.h"
#include "../editor/panels/AIDiffViewerPanel.h"
#include "../editor/panels/AssetBrowserPanel.h"
#include "../editor/panels/AtlasAssistantPanel.h"
#include "../editor/panels/CIDashboardPanel.h"
#include "../editor/panels/ConsolePanel.h"
#include "../editor/panels/DesyncVisualizerPanel.h"
#include "../editor/panels/ECSInspectorPanel.h"
#include "../editor/panels/GameMechanicsUIPanel.h"
#include "../editor/panels/InteractionDebugPanel.h"
#include "../editor/panels/InventoryEditorPanel.h"
#include "../editor/panels/JobTracePanel.h"
#include "../editor/panels/MaterialEditorPanel.h"
#include "../editor/panels/MeshViewerPanel.h"
#include "../editor/panels/NetInspectorPanel.h"
#include "../editor/panels/PrefabEditorPanel.h"
#include "../editor/panels/ProfilerPanel.h"
#include "../editor/panels/ProjectPickerPanel.h"
#include "../editor/panels/ProofViewerPanel.h"
#include "../editor/panels/QuestEditorPanel.h"
#include "../editor/panels/ReplayTimelinePanel.h"
#include "../editor/panels/RuleGraphEditorPanel.h"
#include "../editor/panels/SaveFileBrowserPanel.h"
#include "../editor/panels/StateHashDiffPanel.h"
#include "../editor/panels/TilePalettePanel.h"
#include "../editor/panels/TruthUIPanel.h"
#include "../editor/panels/VoiceCommandPanel.h"
#include "../editor/panels/WorldGraphPanel.h"
#include "../engine/ecs/ECS.h"
#include "../engine/net/NetContext.h"
#include "../engine/assets/AssetRegistry.h"
#include "../engine/voice/VoiceCommand.h"
#include "../engine/interaction/Interaction.h"
#include "../engine/sim/TickScheduler.h"
#include <cassert>
#include <iostream>
#include <string>

using namespace atlas::editor;

// Helper: verify a panel has a non-empty Name and Draw() doesn't crash.
static void verify_panel(EditorPanel* panel, const char* label) {
    const char* name = panel->Name();
    assert(name != nullptr);
    assert(std::string(name).size() > 0);
    panel->Draw();
    std::cout << "  [PASS] " << label << " (" << name << ")" << std::endl;
}

void test_all_panels_instantiate() {
    // Panels with default constructors
    AIDebuggerPanel aidbg;
    verify_panel(&aidbg, "AIDebuggerPanel");

    AIDiffViewerPanel aidiff;
    verify_panel(&aidiff, "AIDiffViewerPanel");

    AtlasAssistantPanel assist;
    verify_panel(&assist, "AtlasAssistantPanel");

    CIDashboardPanel ci;
    verify_panel(&ci, "CIDashboardPanel");

    DesyncVisualizerPanel desync;
    verify_panel(&desync, "DesyncVisualizerPanel");

    GameMechanicsUIPanel gm;
    verify_panel(&gm, "GameMechanicsUIPanel");

    InteractionDebugPanel idp;
    verify_panel(&idp, "InteractionDebugPanel");

    InventoryEditorPanel inv;
    verify_panel(&inv, "InventoryEditorPanel");

    JobTracePanel jt;
    verify_panel(&jt, "JobTracePanel");

    MaterialEditorPanel mat;
    verify_panel(&mat, "MaterialEditorPanel");

    MeshViewerPanel mesh;
    verify_panel(&mesh, "MeshViewerPanel");

    PrefabEditorPanel prefab;
    verify_panel(&prefab, "PrefabEditorPanel");

    ProfilerPanel prof;
    verify_panel(&prof, "ProfilerPanel");

    ProjectPickerPanel picker;
    verify_panel(&picker, "ProjectPickerPanel");

    ProofViewerPanel proof;
    verify_panel(&proof, "ProofViewerPanel");

    QuestEditorPanel quest;
    verify_panel(&quest, "QuestEditorPanel");

    ReplayTimelinePanel replay;
    verify_panel(&replay, "ReplayTimelinePanel");

    RuleGraphEditorPanel rule;
    verify_panel(&rule, "RuleGraphEditorPanel");

    SaveFileBrowserPanel save;
    verify_panel(&save, "SaveFileBrowserPanel");

    StateHashDiffPanel shd;
    verify_panel(&shd, "StateHashDiffPanel");

    TilePalettePanel tile;
    verify_panel(&tile, "TilePalettePanel");

    TruthUIPanel truth;
    verify_panel(&truth, "TruthUIPanel");

    WorldGraphPanel wg;
    verify_panel(&wg, "WorldGraphPanel");

    // Panels requiring constructor dependencies
    atlas::asset::AssetRegistry registry;
    AssetBrowserPanel ab(registry);
    verify_panel(&ab, "AssetBrowserPanel");

    atlas::ecs::World world;
    atlas::net::NetContext net;
    atlas::sim::TickScheduler scheduler;
    ConsolePanel con(world, net, scheduler);
    verify_panel(&con, "ConsolePanel");

    ECSInspectorPanel ecs(world);
    verify_panel(&ecs, "ECSInspectorPanel");

    net.Init(atlas::net::NetMode::Standalone);
    NetInspectorPanel ni(net);
    verify_panel(&ni, "NetInspectorPanel");

    atlas::voice::VoiceCommandRegistry voiceReg;
    atlas::interaction::IntentRegistry intentReg;
    VoiceCommandPanel voice(voiceReg, intentReg);
    verify_panel(&voice, "VoiceCommandPanel");

    std::cout << "  [PASS] test_all_panels_instantiate (28 panels)" << std::endl;
}

void test_panel_draw_produces_drawlist() {
    atlas::asset::AssetRegistry registry;
    AssetBrowserPanel ab(registry);
    ab.Draw();
    assert(ab.GetDrawList().CommandCount() > 0);

    atlas::net::NetContext net;
    net.Init(atlas::net::NetMode::Standalone);
    NetInspectorPanel ni(net);
    ni.Draw();
    assert(ni.GetDrawList().CommandCount() > 0);

    JobTracePanel jt;
    jt.Draw();
    assert(jt.GetDrawList().CommandCount() > 0);

    StateHashDiffPanel shd;
    shd.Draw();
    assert(shd.GetDrawList().CommandCount() > 0);

    ProfilerPanel prof;
    prof.Draw();
    assert(prof.GetDrawList().CommandCount() > 0);

    DesyncVisualizerPanel dv;
    dv.Draw();
    assert(dv.GetDrawList().CommandCount() > 0);

    AIDebuggerPanel aid;
    aid.Draw();
    assert(aid.GetDrawList().CommandCount() > 0);

    std::cout << "  [PASS] test_panel_draw_produces_drawlist (7 panels)" << std::endl;
}

void register_editor_panel_registry_tests() {
    std::cout << "\n--- Editor Panel Registry Tests ---" << std::endl;
    test_all_panels_instantiate();
    test_panel_draw_produces_drawlist();
    std::cout << "  Editor panel registry tests: 2/2 passed\n";
}
