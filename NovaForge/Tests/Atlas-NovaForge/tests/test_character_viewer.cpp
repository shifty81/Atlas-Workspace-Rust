#include "../engine/character/HumanMeshGenerator.h"
#include "../engine/character/CharacterGraph.h"
#include "../engine/character/CharacterNodes.h"
#include "../engine/animation/CharacterAnimationController.h"
#include "../editor/ui/CharacterSelectScreen.h"
#include "../editor/panels/CharacterEditorPanel.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace atlas::character;
using namespace atlas::animation;
using namespace atlas::editor;

// ===== HumanMeshGenerator tests =====

void test_human_mesh_generate_default() {
    HumanMeshParams params;
    auto mesh = HumanMeshGenerator::Generate(params);
    assert(mesh.IsValid());
    assert(mesh.VertexCount() > 0);
    assert(mesh.TriangleCount() > 0);
    std::cout << "[PASS] test_human_mesh_generate_default" << std::endl;
}

void test_human_mesh_generate_from_context() {
    CharacterContext ctx{42, 1, 30.0f, 0.5f};
    auto mesh = HumanMeshGenerator::GenerateFromContext(ctx);
    assert(mesh.IsValid());
    assert(mesh.VertexCount() > 0);
    assert(mesh.TriangleCount() > 0);
    std::cout << "[PASS] test_human_mesh_generate_from_context" << std::endl;
}

void test_human_mesh_deterministic() {
    CharacterContext ctx{42, 1, 30.0f, 0.5f};
    auto a = HumanMeshGenerator::GenerateFromContext(ctx);
    auto b = HumanMeshGenerator::GenerateFromContext(ctx);
    assert(a.vertices == b.vertices);
    assert(a.normals == b.normals);
    assert(a.indices == b.indices);
    std::cout << "[PASS] test_human_mesh_deterministic" << std::endl;
}

void test_human_mesh_different_seeds() {
    auto a = HumanMeshGenerator::GenerateFromContext({42, 1, 30.0f, 0.5f});
    auto b = HumanMeshGenerator::GenerateFromContext({99, 1, 30.0f, 0.5f});
    // Different seeds produce different meshes (vertices will differ)
    assert(a.vertices != b.vertices);
    std::cout << "[PASS] test_human_mesh_different_seeds" << std::endl;
}

void test_human_mesh_params_from_graph() {
    CharacterGraph graph;
    auto bodyId = graph.AddNode(std::make_unique<BaseBodyNode>());
    auto faceId = graph.AddNode(std::make_unique<FacialFeatureNode>());
    auto hairId = graph.AddNode(std::make_unique<HairStyleNode>());
    auto matId  = graph.AddNode(std::make_unique<MaterialNode>());
    graph.Compile();

    CharacterContext ctx{42, 1, 30.0f, 0.5f};
    graph.Execute(ctx);

    auto params = HumanMeshGenerator::ParamsFromGraph(graph, bodyId, faceId, hairId, matId);

    // Height in valid range
    assert(params.height >= 1.5f && params.height <= 2.1f);
    // Mass positive
    assert(params.mass > 0.0f);
    // Facial features in range
    assert(params.noseWidth >= 0.3f && params.noseWidth <= 0.7f);
    // Hair color components in [0, 1]
    assert(params.hairR >= 0.0f && params.hairR <= 1.0f);
    assert(params.hairG >= 0.0f && params.hairG <= 1.0f);
    assert(params.hairB >= 0.0f && params.hairB <= 1.0f);

    std::cout << "[PASS] test_human_mesh_params_from_graph" << std::endl;
}

void test_human_mesh_height_affects_size() {
    HumanMeshParams small, large;
    small.height = 1.5f;
    large.height = 2.1f;

    auto meshSmall = HumanMeshGenerator::Generate(small);
    auto meshLarge = HumanMeshGenerator::Generate(large);

    // Both valid
    assert(meshSmall.IsValid());
    assert(meshLarge.IsValid());

    // Larger height means larger extents
    float maxYSmall = 0.0f, maxYLarge = 0.0f;
    for (size_t i = 1; i < meshSmall.vertices.size(); i += 3)
        if (meshSmall.vertices[i] > maxYSmall) maxYSmall = meshSmall.vertices[i];
    for (size_t i = 1; i < meshLarge.vertices.size(); i += 3)
        if (meshLarge.vertices[i] > maxYLarge) maxYLarge = meshLarge.vertices[i];

    assert(maxYLarge > maxYSmall);
    std::cout << "[PASS] test_human_mesh_height_affects_size" << std::endl;
}

// ===== CharacterAnimationController tests =====

void test_anim_controller_default_state() {
    CharacterAnimationController ctrl;
    assert(ctrl.CurrentState() == CharacterAnimState::Idle);
    assert(ctrl.Speed() == 1.0f);
    assert(ctrl.Tick() == 0);
    assert(ctrl.ElapsedTime() == 0.0f);
    assert(ctrl.CurrentPose().size() == 8);
    std::cout << "[PASS] test_anim_controller_default_state" << std::endl;
}

void test_anim_controller_set_state() {
    CharacterAnimationController ctrl;
    ctrl.SetState(CharacterAnimState::Walk);
    assert(ctrl.CurrentState() == CharacterAnimState::Walk);

    ctrl.SetState(CharacterAnimState::Run, 0.1f);
    assert(ctrl.CurrentState() == CharacterAnimState::Run);

    // Setting same state should not change
    ctrl.SetState(CharacterAnimState::Run);
    assert(ctrl.CurrentState() == CharacterAnimState::Run);

    std::cout << "[PASS] test_anim_controller_set_state" << std::endl;
}

void test_anim_controller_update() {
    CharacterAnimationController ctrl;
    ctrl.Update(0.016f);
    assert(ctrl.Tick() == 1);
    assert(ctrl.ElapsedTime() > 0.0f);
    assert(!ctrl.CurrentPose().empty());

    ctrl.Update(0.016f);
    assert(ctrl.Tick() == 2);

    std::cout << "[PASS] test_anim_controller_update" << std::endl;
}

void test_anim_controller_clip_lengths() {
    assert(CharacterAnimationController::ClipLengthForState(CharacterAnimState::Idle) > 0.0f);
    assert(CharacterAnimationController::ClipLengthForState(CharacterAnimState::Walk) > 0.0f);
    assert(CharacterAnimationController::ClipLengthForState(CharacterAnimState::Run) > 0.0f);
    assert(CharacterAnimationController::ClipLengthForState(CharacterAnimState::Attack) > 0.0f);
    assert(CharacterAnimationController::ClipLengthForState(CharacterAnimState::Hit) > 0.0f);
    assert(CharacterAnimationController::ClipLengthForState(CharacterAnimState::Death) > 0.0f);

    // Run is faster than walk
    assert(CharacterAnimationController::ClipLengthForState(CharacterAnimState::Run)
         < CharacterAnimationController::ClipLengthForState(CharacterAnimState::Walk));

    std::cout << "[PASS] test_anim_controller_clip_lengths" << std::endl;
}

void test_anim_controller_speed_and_damage() {
    CharacterAnimationController ctrl;
    ctrl.SetSpeed(2.0f);
    assert(ctrl.Speed() == 2.0f);
    ctrl.SetFatigue(0.5f);
    ctrl.SetDamageLevel(0.3f);
    ctrl.Update(0.016f);
    assert(!ctrl.CurrentPose().empty());
    std::cout << "[PASS] test_anim_controller_speed_and_damage" << std::endl;
}

// ===== CharacterSelectScreen tests =====

void test_char_select_init_slots() {
    CharacterSelectScreen screen;
    screen.InitSlots({10, 20, 30});
    assert(screen.SlotCount() == 3);
    assert(screen.SelectedIndex() == 0);
    assert(screen.SelectedSlot() != nullptr);
    assert(screen.SelectedSlot()->meshValid);
    std::cout << "[PASS] test_char_select_init_slots" << std::endl;
}

void test_char_select_select_slot() {
    CharacterSelectScreen screen;
    screen.InitSlots({10, 20, 30});
    screen.SelectSlot(2);
    assert(screen.SelectedIndex() == 2);
    assert(screen.SelectedSlot()->seed == 30);

    // Out-of-range does not change
    screen.SelectSlot(100);
    assert(screen.SelectedIndex() == 2);

    std::cout << "[PASS] test_char_select_select_slot" << std::endl;
}

void test_char_select_confirm() {
    CharacterSelectScreen screen;
    screen.InitSlots({10});
    assert(!screen.IsSelectionConfirmed());
    screen.ConfirmSelection();
    assert(screen.IsSelectionConfirmed());
    std::cout << "[PASS] test_char_select_confirm" << std::endl;
}

void test_char_select_rotation() {
    CharacterSelectScreen screen;
    assert(screen.RotationY() == 0.0f);

    screen.SetRotationY(45.0f);
    assert(screen.RotationY() == 45.0f);

    screen.Rotate(15.0f);
    assert(std::abs(screen.RotationY() - 60.0f) < 0.001f);

    screen.Rotate(-30.0f);
    assert(std::abs(screen.RotationY() - 30.0f) < 0.001f);

    std::cout << "[PASS] test_char_select_rotation" << std::endl;
}

void test_char_select_zoom() {
    CharacterSelectScreen screen;
    assert(screen.Zoom() == 1.0f);
    screen.SetZoom(1.5f);
    assert(screen.Zoom() == 1.5f);
    std::cout << "[PASS] test_char_select_zoom" << std::endl;
}

void test_char_select_animation() {
    CharacterSelectScreen screen;
    screen.InitSlots({42});
    screen.UpdateAnimation(0.016f);
    assert(screen.AnimController().Tick() == 1);
    std::cout << "[PASS] test_char_select_animation" << std::endl;
}

void test_char_select_draw() {
    CharacterSelectScreen screen;
    screen.InitSlots({42, 99});
    screen.SetRotationY(90.0f);
    screen.Draw();
    assert(screen.GetDrawList().CommandCount() > 0);
    std::cout << "[PASS] test_char_select_draw" << std::endl;
}

void test_char_select_regenerate() {
    CharacterSelectScreen screen;
    screen.InitSlots({42});
    auto* slot = screen.SelectedSlot();
    assert(slot != nullptr);
    size_t origVerts = slot->mesh.VertexCount();

    screen.RegenerateSelectedMesh();
    // After regeneration, mesh is still valid with same vertex count (same params)
    assert(screen.SelectedSlot()->meshValid);
    assert(screen.SelectedSlot()->mesh.VertexCount() == origVerts);

    std::cout << "[PASS] test_char_select_regenerate" << std::endl;
}

// ===== CharacterEditorPanel tests =====

void test_char_editor_name() {
    CharacterEditorPanel panel;
    assert(std::string(panel.Name()) == "Character Editor");
    std::cout << "[PASS] test_char_editor_name" << std::endl;
}

void test_char_editor_default_params() {
    CharacterEditorPanel panel;
    auto& p = panel.Params();
    assert(p.height == 1.8f);
    assert(p.mass == 70.0f);
    assert(panel.IsDirty());
    std::cout << "[PASS] test_char_editor_default_params" << std::endl;
}

void test_char_editor_set_params() {
    CharacterEditorPanel panel;
    HumanMeshParams p;
    p.height = 2.0f;
    p.mass = 90.0f;
    panel.SetParams(p);
    assert(panel.Params().height == 2.0f);
    assert(panel.Params().mass == 90.0f);
    assert(panel.IsDirty());
    std::cout << "[PASS] test_char_editor_set_params" << std::endl;
}

void test_char_editor_individual_setters() {
    CharacterEditorPanel panel;
    panel.RegenerateMesh(); // clear dirty
    assert(!panel.IsDirty());

    panel.SetHeight(1.9f);
    assert(panel.IsDirty());
    assert(panel.Params().height == 1.9f);

    panel.SetMass(80.0f);
    assert(panel.Params().mass == 80.0f);

    panel.SetNoseWidth(0.6f);
    assert(panel.Params().noseWidth == 0.6f);

    panel.SetHairColor(0.8f, 0.2f, 0.1f);
    assert(panel.Params().hairR == 0.8f);
    assert(panel.Params().hairG == 0.2f);
    assert(panel.Params().hairB == 0.1f);

    std::cout << "[PASS] test_char_editor_individual_setters" << std::endl;
}

void test_char_editor_regenerate_mesh() {
    CharacterEditorPanel panel;
    assert(!panel.HasMesh());

    bool changed = panel.RegenerateMesh();
    assert(changed);
    assert(panel.HasMesh());
    assert(panel.GetMesh().IsValid());
    assert(panel.GetMesh().VertexCount() > 0);
    assert(!panel.IsDirty());

    // Regenerate when not dirty returns false
    changed = panel.RegenerateMesh();
    assert(!changed);

    std::cout << "[PASS] test_char_editor_regenerate_mesh" << std::endl;
}

void test_char_editor_sections() {
    CharacterEditorPanel panel;
    assert(panel.CurrentSection() == CharacterEditorSection::Body);

    panel.SetSection(CharacterEditorSection::Face);
    assert(panel.CurrentSection() == CharacterEditorSection::Face);

    panel.SetSection(CharacterEditorSection::Hair);
    assert(panel.CurrentSection() == CharacterEditorSection::Hair);

    panel.SetSection(CharacterEditorSection::Material);
    assert(panel.CurrentSection() == CharacterEditorSection::Material);

    panel.SetSection(CharacterEditorSection::Animation);
    assert(panel.CurrentSection() == CharacterEditorSection::Animation);

    std::cout << "[PASS] test_char_editor_sections" << std::endl;
}

void test_char_editor_animation() {
    CharacterEditorPanel panel;
    panel.SetAnimState(CharacterAnimState::Walk);
    assert(panel.AnimState() == CharacterAnimState::Walk);

    panel.UpdateAnimation(0.016f);
    assert(panel.AnimController().Tick() == 1);

    std::cout << "[PASS] test_char_editor_animation" << std::endl;
}

void test_char_editor_draw() {
    CharacterEditorPanel panel;
    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);

    // Test all sections draw without errors
    panel.SetSection(CharacterEditorSection::Face);
    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);

    panel.SetSection(CharacterEditorSection::Hair);
    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);

    panel.SetSection(CharacterEditorSection::Material);
    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);

    panel.SetSection(CharacterEditorSection::Animation);
    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);

    std::cout << "[PASS] test_char_editor_draw" << std::endl;
}

void test_char_editor_summary() {
    CharacterEditorPanel panel;
    std::string summary = panel.Summary();
    assert(summary.find("Height:") != std::string::npos);
    assert(summary.find("Mass:") != std::string::npos);

    panel.RegenerateMesh();
    summary = panel.Summary();
    assert(summary.find("Verts:") != std::string::npos);
    assert(summary.find("Tris:") != std::string::npos);

    std::cout << "[PASS] test_char_editor_summary" << std::endl;
}

void test_char_editor_callback() {
    CharacterEditorPanel panel;
    bool callbackFired = false;
    HumanMeshParams receivedParams;

    panel.SetOnParamsChanged([&](const HumanMeshParams& p) {
        callbackFired = true;
        receivedParams = p;
    });

    panel.SetHeight(1.95f);
    panel.RegenerateMesh();

    assert(callbackFired);
    assert(receivedParams.height == 1.95f);

    std::cout << "[PASS] test_char_editor_callback" << std::endl;
}
