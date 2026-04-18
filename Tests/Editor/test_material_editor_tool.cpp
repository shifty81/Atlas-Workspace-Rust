// Tests/Editor/test_material_editor_tool.cpp
// Tests for Phase 3 NF::MaterialEditorTool — third real IHostedTool.
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/MaterialEditorTool.h"

using namespace NF;

// ─────────────────────────────────────────────────────────────────
// MaterialEditMode helpers
// ─────────────────────────────────────────────────────────────────

TEST_CASE("materialEditModeName returns correct strings", "[MaterialEditMode]") {
    CHECK(std::string(materialEditModeName(MaterialEditMode::Properties)) == "Properties");
    CHECK(std::string(materialEditModeName(MaterialEditMode::NodeGraph))  == "NodeGraph");
    CHECK(std::string(materialEditModeName(MaterialEditMode::Preview))    == "Preview");
}

// ─────────────────────────────────────────────────────────────────
// MaterialEditorTool — descriptor
// ─────────────────────────────────────────────────────────────────

TEST_CASE("MaterialEditorTool descriptor is valid at construction", "[MaterialEditorTool][descriptor]") {
    MaterialEditorTool tool;
    const auto& d = tool.descriptor();
    CHECK(d.isValid());
    CHECK(d.toolId      == MaterialEditorTool::kToolId);
    CHECK(d.displayName == "Material Editor");
    CHECK(d.category    == HostedToolCategory::AssetAuthoring);
    CHECK(d.isPrimary   == true);
    CHECK(d.acceptsProjectExtensions == false);
}

TEST_CASE("MaterialEditorTool toolId matches kToolId", "[MaterialEditorTool][descriptor]") {
    MaterialEditorTool tool;
    CHECK(tool.toolId() == std::string(MaterialEditorTool::kToolId));
    CHECK(tool.toolId() == "workspace.material_editor");
}

TEST_CASE("MaterialEditorTool declares expected shared panels", "[MaterialEditorTool][descriptor]") {
    MaterialEditorTool tool;
    const auto& panels = tool.descriptor().supportedPanels;
    REQUIRE(panels.size() >= 4);
    auto has = [&](const std::string& id) {
        for (const auto& p : panels) if (p == id) return true;
        return false;
    };
    CHECK(has("panel.viewport_material"));
    CHECK(has("panel.inspector"));
    CHECK(has("panel.asset_preview"));
    CHECK(has("panel.console"));
}

TEST_CASE("MaterialEditorTool declares expected commands", "[MaterialEditorTool][descriptor]") {
    MaterialEditorTool tool;
    const auto& cmds = tool.descriptor().commands;
    REQUIRE(cmds.size() >= 6);
    auto has = [&](const std::string& id) {
        for (const auto& c : cmds) if (c == id) return true;
        return false;
    };
    CHECK(has("material.create"));
    CHECK(has("material.save"));
    CHECK(has("material.set_shader"));
    CHECK(has("material.add_texture"));
    CHECK(has("material.duplicate"));
    CHECK(has("material.open"));
}

// ─────────────────────────────────────────────────────────────────
// MaterialEditorTool — lifecycle
// ─────────────────────────────────────────────────────────────────

TEST_CASE("MaterialEditorTool initial state is Unloaded", "[MaterialEditorTool][lifecycle]") {
    MaterialEditorTool tool;
    CHECK(tool.state() == HostedToolState::Unloaded);
}

TEST_CASE("MaterialEditorTool initialize transitions to Ready", "[MaterialEditorTool][lifecycle]") {
    MaterialEditorTool tool;
    CHECK(tool.initialize());
    CHECK(tool.state() == HostedToolState::Ready);
}

TEST_CASE("MaterialEditorTool double initialize returns false", "[MaterialEditorTool][lifecycle]") {
    MaterialEditorTool tool;
    CHECK(tool.initialize());
    CHECK_FALSE(tool.initialize());
    CHECK(tool.state() == HostedToolState::Ready);
}

TEST_CASE("MaterialEditorTool activate from Ready transitions to Active", "[MaterialEditorTool][lifecycle]") {
    MaterialEditorTool tool;
    tool.initialize();
    tool.activate();
    CHECK(tool.state() == HostedToolState::Active);
}

TEST_CASE("MaterialEditorTool suspend from Active transitions to Suspended", "[MaterialEditorTool][lifecycle]") {
    MaterialEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.suspend();
    CHECK(tool.state() == HostedToolState::Suspended);
}

TEST_CASE("MaterialEditorTool activate from Suspended transitions to Active", "[MaterialEditorTool][lifecycle]") {
    MaterialEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.suspend();
    tool.activate();
    CHECK(tool.state() == HostedToolState::Active);
}

TEST_CASE("MaterialEditorTool shutdown from Active transitions to Unloaded", "[MaterialEditorTool][lifecycle]") {
    MaterialEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.shutdown();
    CHECK(tool.state() == HostedToolState::Unloaded);
}

TEST_CASE("MaterialEditorTool update does not change state", "[MaterialEditorTool][lifecycle]") {
    MaterialEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.update(0.016f);
    CHECK(tool.state() == HostedToolState::Active);
}

// ─────────────────────────────────────────────────────────────────
// MaterialEditorTool — edit mode
// ─────────────────────────────────────────────────────────────────

TEST_CASE("MaterialEditorTool default edit mode is Properties", "[MaterialEditorTool][mode]") {
    MaterialEditorTool tool;
    tool.initialize();
    CHECK(tool.editMode() == MaterialEditMode::Properties);
}

TEST_CASE("MaterialEditorTool setEditMode roundtrips all modes", "[MaterialEditorTool][mode]") {
    MaterialEditorTool tool;
    tool.initialize();

    tool.setEditMode(MaterialEditMode::NodeGraph);
    CHECK(tool.editMode() == MaterialEditMode::NodeGraph);

    tool.setEditMode(MaterialEditMode::Preview);
    CHECK(tool.editMode() == MaterialEditMode::Preview);

    tool.setEditMode(MaterialEditMode::Properties);
    CHECK(tool.editMode() == MaterialEditMode::Properties);
}

// ─────────────────────────────────────────────────────────────────
// MaterialEditorTool — dirty state
// ─────────────────────────────────────────────────────────────────

TEST_CASE("MaterialEditorTool initially not dirty", "[MaterialEditorTool][dirty]") {
    MaterialEditorTool tool;
    tool.initialize();
    CHECK_FALSE(tool.isDirty());
}

TEST_CASE("MaterialEditorTool markDirty / clearDirty", "[MaterialEditorTool][dirty]") {
    MaterialEditorTool tool;
    tool.initialize();
    tool.markDirty();
    CHECK(tool.isDirty());
    tool.clearDirty();
    CHECK_FALSE(tool.isDirty());
}

TEST_CASE("MaterialEditorTool shutdown clears dirty flag", "[MaterialEditorTool][dirty]") {
    MaterialEditorTool tool;
    tool.initialize();
    tool.markDirty();
    tool.shutdown();
    CHECK_FALSE(tool.isDirty());
}

// ─────────────────────────────────────────────────────────────────
// MaterialEditorTool — stats
// ─────────────────────────────────────────────────────────────────

TEST_CASE("MaterialEditorTool stats default to zero", "[MaterialEditorTool][stats]") {
    MaterialEditorTool tool;
    tool.initialize();
    const auto& s = tool.stats();
    CHECK(s.nodeCount        == 0);
    CHECK(s.textureSlotCount == 0);
    CHECK(s.isDirty          == false);
}

TEST_CASE("MaterialEditorTool setNodeCount updates stats", "[MaterialEditorTool][stats]") {
    MaterialEditorTool tool;
    tool.initialize();
    tool.setNodeCount(12);
    CHECK(tool.stats().nodeCount == 12);
}

TEST_CASE("MaterialEditorTool setTextureSlotCount updates stats", "[MaterialEditorTool][stats]") {
    MaterialEditorTool tool;
    tool.initialize();
    tool.setTextureSlotCount(4);
    CHECK(tool.stats().textureSlotCount == 4);
}

// ─────────────────────────────────────────────────────────────────
// MaterialEditorTool — open asset path
// ─────────────────────────────────────────────────────────────────

TEST_CASE("MaterialEditorTool openAssetPath starts empty", "[MaterialEditorTool][asset]") {
    MaterialEditorTool tool;
    tool.initialize();
    CHECK(tool.openAssetPath().empty());
}

TEST_CASE("MaterialEditorTool setOpenAssetPath roundtrips", "[MaterialEditorTool][asset]") {
    MaterialEditorTool tool;
    tool.initialize();
    tool.setOpenAssetPath("Assets/Materials/Rock.mat");
    CHECK(tool.openAssetPath() == "Assets/Materials/Rock.mat");
}

TEST_CASE("MaterialEditorTool shutdown clears openAssetPath", "[MaterialEditorTool][asset]") {
    MaterialEditorTool tool;
    tool.initialize();
    tool.setOpenAssetPath("Assets/Materials/Rock.mat");
    tool.shutdown();
    CHECK(tool.openAssetPath().empty());
}

// ─────────────────────────────────────────────────────────────────
// MaterialEditorTool — project hooks
// ─────────────────────────────────────────────────────────────────

TEST_CASE("MaterialEditorTool onProjectLoaded clears stats", "[MaterialEditorTool][project]") {
    MaterialEditorTool tool;
    tool.initialize();
    tool.setNodeCount(8);
    tool.markDirty();
    tool.onProjectLoaded("nova_forge");
    CHECK(tool.stats().nodeCount == 0);
    CHECK_FALSE(tool.isDirty());
}

TEST_CASE("MaterialEditorTool onProjectUnloaded clears stats", "[MaterialEditorTool][project]") {
    MaterialEditorTool tool;
    tool.initialize();
    tool.setNodeCount(8);
    tool.onProjectLoaded("nova_forge");
    tool.onProjectUnloaded();
    CHECK(tool.stats().nodeCount == 0);
}
