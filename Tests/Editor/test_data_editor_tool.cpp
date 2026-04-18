// Tests/Editor/test_data_editor_tool.cpp
// Tests for Phase 3 NF::DataEditorTool — fifth real IHostedTool.
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/DataEditorTool.h"

using namespace NF;

// ─────────────────────────────────────────────────────────────────
// DataEditMode helpers
// ─────────────────────────────────────────────────────────────────

TEST_CASE("dataEditModeName returns correct strings", "[DataEditMode]") {
    CHECK(std::string(dataEditModeName(DataEditMode::Table)) == "Table");
    CHECK(std::string(dataEditModeName(DataEditMode::Json))  == "Json");
    CHECK(std::string(dataEditModeName(DataEditMode::Diff))  == "Diff");
}

// ─────────────────────────────────────────────────────────────────
// DataEditorTool — descriptor
// ─────────────────────────────────────────────────────────────────

TEST_CASE("DataEditorTool descriptor is valid at construction", "[DataEditorTool][descriptor]") {
    DataEditorTool tool;
    const auto& d = tool.descriptor();
    CHECK(d.isValid());
    CHECK(d.toolId      == DataEditorTool::kToolId);
    CHECK(d.displayName == "Data Editor");
    CHECK(d.category    == HostedToolCategory::DataEditing);
    CHECK(d.isPrimary   == true);
    CHECK(d.acceptsProjectExtensions == true);
}

TEST_CASE("DataEditorTool toolId matches kToolId", "[DataEditorTool][descriptor]") {
    DataEditorTool tool;
    CHECK(tool.toolId() == std::string(DataEditorTool::kToolId));
    CHECK(tool.toolId() == "workspace.data_editor");
}

TEST_CASE("DataEditorTool declares expected shared panels", "[DataEditorTool][descriptor]") {
    DataEditorTool tool;
    const auto& panels = tool.descriptor().supportedPanels;
    REQUIRE(panels.size() >= 3);
    auto has = [&](const std::string& id) {
        for (const auto& p : panels) if (p == id) return true;
        return false;
    };
    CHECK(has("panel.data_table"));
    CHECK(has("panel.inspector"));
    CHECK(has("panel.console"));
}

TEST_CASE("DataEditorTool declares expected commands", "[DataEditorTool][descriptor]") {
    DataEditorTool tool;
    const auto& cmds = tool.descriptor().commands;
    REQUIRE(cmds.size() >= 6);
    auto has = [&](const std::string& id) {
        for (const auto& c : cmds) if (c == id) return true;
        return false;
    };
    CHECK(has("data.new_row"));
    CHECK(has("data.delete_row"));
    CHECK(has("data.duplicate_row"));
    CHECK(has("data.import_csv"));
    CHECK(has("data.export_csv"));
    CHECK(has("data.save"));
}

// ─────────────────────────────────────────────────────────────────
// DataEditorTool — lifecycle
// ─────────────────────────────────────────────────────────────────

TEST_CASE("DataEditorTool initial state is Unloaded", "[DataEditorTool][lifecycle]") {
    DataEditorTool tool;
    CHECK(tool.state() == HostedToolState::Unloaded);
}

TEST_CASE("DataEditorTool initialize transitions to Ready", "[DataEditorTool][lifecycle]") {
    DataEditorTool tool;
    CHECK(tool.initialize());
    CHECK(tool.state() == HostedToolState::Ready);
}

TEST_CASE("DataEditorTool double initialize returns false", "[DataEditorTool][lifecycle]") {
    DataEditorTool tool;
    CHECK(tool.initialize());
    CHECK_FALSE(tool.initialize());
    CHECK(tool.state() == HostedToolState::Ready);
}

TEST_CASE("DataEditorTool activate from Ready transitions to Active", "[DataEditorTool][lifecycle]") {
    DataEditorTool tool;
    tool.initialize();
    tool.activate();
    CHECK(tool.state() == HostedToolState::Active);
}

TEST_CASE("DataEditorTool suspend from Active transitions to Suspended", "[DataEditorTool][lifecycle]") {
    DataEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.suspend();
    CHECK(tool.state() == HostedToolState::Suspended);
}

TEST_CASE("DataEditorTool activate from Suspended transitions to Active", "[DataEditorTool][lifecycle]") {
    DataEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.suspend();
    tool.activate();
    CHECK(tool.state() == HostedToolState::Active);
}

TEST_CASE("DataEditorTool shutdown from Active transitions to Unloaded", "[DataEditorTool][lifecycle]") {
    DataEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.shutdown();
    CHECK(tool.state() == HostedToolState::Unloaded);
}

// ─────────────────────────────────────────────────────────────────
// DataEditorTool — edit mode
// ─────────────────────────────────────────────────────────────────

TEST_CASE("DataEditorTool default edit mode is Table", "[DataEditorTool][mode]") {
    DataEditorTool tool;
    tool.initialize();
    CHECK(tool.editMode() == DataEditMode::Table);
}

TEST_CASE("DataEditorTool setEditMode roundtrips all modes", "[DataEditorTool][mode]") {
    DataEditorTool tool;
    tool.initialize();
    tool.setEditMode(DataEditMode::Json);
    CHECK(tool.editMode() == DataEditMode::Json);
    tool.setEditMode(DataEditMode::Diff);
    CHECK(tool.editMode() == DataEditMode::Diff);
    tool.setEditMode(DataEditMode::Table);
    CHECK(tool.editMode() == DataEditMode::Table);
}

// ─────────────────────────────────────────────────────────────────
// DataEditorTool — dirty state
// ─────────────────────────────────────────────────────────────────

TEST_CASE("DataEditorTool initially not dirty", "[DataEditorTool][dirty]") {
    DataEditorTool tool;
    tool.initialize();
    CHECK_FALSE(tool.isDirty());
}

TEST_CASE("DataEditorTool markDirty / clearDirty", "[DataEditorTool][dirty]") {
    DataEditorTool tool;
    tool.initialize();
    tool.markDirty();
    CHECK(tool.isDirty());
    tool.clearDirty();
    CHECK_FALSE(tool.isDirty());
}

TEST_CASE("DataEditorTool shutdown clears dirty flag", "[DataEditorTool][dirty]") {
    DataEditorTool tool;
    tool.initialize();
    tool.markDirty();
    tool.shutdown();
    CHECK_FALSE(tool.isDirty());
}

// ─────────────────────────────────────────────────────────────────
// DataEditorTool — stats
// ─────────────────────────────────────────────────────────────────

TEST_CASE("DataEditorTool stats default to zero", "[DataEditorTool][stats]") {
    DataEditorTool tool;
    tool.initialize();
    const auto& s = tool.stats();
    CHECK(s.rowCount         == 0u);
    CHECK(s.selectedRowCount == 0u);
    CHECK(s.columnCount      == 0u);
    CHECK(s.isDirty          == false);
}

TEST_CASE("DataEditorTool setRowCount roundtrips", "[DataEditorTool][stats]") {
    DataEditorTool tool;
    tool.initialize();
    tool.setRowCount(250);
    CHECK(tool.stats().rowCount == 250u);
}

TEST_CASE("DataEditorTool setSelectedRowCount roundtrips", "[DataEditorTool][stats]") {
    DataEditorTool tool;
    tool.initialize();
    tool.setSelectedRowCount(3);
    CHECK(tool.stats().selectedRowCount == 3u);
}

TEST_CASE("DataEditorTool setColumnCount roundtrips", "[DataEditorTool][stats]") {
    DataEditorTool tool;
    tool.initialize();
    tool.setColumnCount(12);
    CHECK(tool.stats().columnCount == 12u);
}

// ─────────────────────────────────────────────────────────────────
// DataEditorTool — open table path
// ─────────────────────────────────────────────────────────────────

TEST_CASE("DataEditorTool openTablePath starts empty", "[DataEditorTool][asset]") {
    DataEditorTool tool;
    tool.initialize();
    CHECK(tool.openTablePath().empty());
}

TEST_CASE("DataEditorTool setOpenTablePath roundtrips", "[DataEditorTool][asset]") {
    DataEditorTool tool;
    tool.initialize();
    tool.setOpenTablePath("Data/Items.datatable");
    CHECK(tool.openTablePath() == "Data/Items.datatable");
}

TEST_CASE("DataEditorTool shutdown clears openTablePath", "[DataEditorTool][asset]") {
    DataEditorTool tool;
    tool.initialize();
    tool.setOpenTablePath("Data/Items.datatable");
    tool.shutdown();
    CHECK(tool.openTablePath().empty());
}

// ─────────────────────────────────────────────────────────────────
// DataEditorTool — project hooks
// ─────────────────────────────────────────────────────────────────

TEST_CASE("DataEditorTool onProjectLoaded clears stats", "[DataEditorTool][project]") {
    DataEditorTool tool;
    tool.initialize();
    tool.setRowCount(100);
    tool.markDirty();
    tool.onProjectLoaded("nova_forge");
    CHECK(tool.stats().rowCount == 0u);
    CHECK_FALSE(tool.isDirty());
}

TEST_CASE("DataEditorTool onProjectUnloaded clears stats", "[DataEditorTool][project]") {
    DataEditorTool tool;
    tool.initialize();
    tool.setRowCount(100);
    tool.onProjectLoaded("nova_forge");
    tool.onProjectUnloaded();
    CHECK(tool.stats().rowCount == 0u);
}
