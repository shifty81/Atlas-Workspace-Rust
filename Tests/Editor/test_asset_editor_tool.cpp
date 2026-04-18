// Tests/Editor/test_asset_editor_tool.cpp
// Tests for Phase 3 NF::AssetEditorTool — second real IHostedTool.
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/AssetEditorTool.h"

using namespace NF;

// ─────────────────────────────────────────────────────────────────
// AssetFilterMode helpers
// ─────────────────────────────────────────────────────────────────

TEST_CASE("assetFilterModeName returns correct strings", "[AssetFilterMode]") {
    CHECK(std::string(assetFilterModeName(AssetFilterMode::All))       == "All");
    CHECK(std::string(assetFilterModeName(AssetFilterMode::Textures))  == "Textures");
    CHECK(std::string(assetFilterModeName(AssetFilterMode::Materials)) == "Materials");
    CHECK(std::string(assetFilterModeName(AssetFilterMode::Meshes))    == "Meshes");
    CHECK(std::string(assetFilterModeName(AssetFilterMode::Audio))     == "Audio");
    CHECK(std::string(assetFilterModeName(AssetFilterMode::Scripts))   == "Scripts");
    CHECK(std::string(assetFilterModeName(AssetFilterMode::Prefabs))   == "Prefabs");
}

// ─────────────────────────────────────────────────────────────────
// AssetEditorTool — descriptor
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AssetEditorTool descriptor is valid at construction", "[AssetEditorTool][descriptor]") {
    AssetEditorTool tool;
    const auto& d = tool.descriptor();
    CHECK(d.isValid());
    CHECK(d.toolId      == AssetEditorTool::kToolId);
    CHECK(d.displayName == "Asset Editor");
    CHECK(d.category    == HostedToolCategory::ProjectBrowser);
    CHECK(d.isPrimary   == true);
    CHECK(d.acceptsProjectExtensions == true);
}

TEST_CASE("AssetEditorTool toolId matches kToolId", "[AssetEditorTool][descriptor]") {
    AssetEditorTool tool;
    CHECK(tool.toolId() == std::string(AssetEditorTool::kToolId));
    CHECK(tool.toolId() == "workspace.asset_editor");
}

TEST_CASE("AssetEditorTool declares expected supported panels", "[AssetEditorTool][descriptor]") {
    AssetEditorTool tool;
    const auto& panels = tool.descriptor().supportedPanels;
    CHECK(panels.size() == 4);
    auto has = [&](const std::string& p) {
        return std::find(panels.begin(), panels.end(), p) != panels.end();
    };
    CHECK(has("panel.content_browser"));
    CHECK(has("panel.inspector"));
    CHECK(has("panel.asset_preview"));
    CHECK(has("panel.console"));
}

TEST_CASE("AssetEditorTool declares expected commands", "[AssetEditorTool][descriptor]") {
    AssetEditorTool tool;
    const auto& cmds = tool.descriptor().commands;
    CHECK(cmds.size() == 7);
    auto has = [&](const std::string& c) {
        return std::find(cmds.begin(), cmds.end(), c) != cmds.end();
    };
    CHECK(has("asset.import"));
    CHECK(has("asset.delete"));
    CHECK(has("asset.rename"));
    CHECK(has("asset.duplicate"));
    CHECK(has("asset.refresh"));
    CHECK(has("asset.open"));
    CHECK(has("asset.create_folder"));
}

// ─────────────────────────────────────────────────────────────────
// AssetEditorTool — initial state
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AssetEditorTool starts in Unloaded state", "[AssetEditorTool][lifecycle]") {
    AssetEditorTool tool;
    CHECK(tool.state() == HostedToolState::Unloaded);
}

TEST_CASE("AssetEditorTool initial filter mode is All", "[AssetEditorTool][state]") {
    AssetEditorTool tool;
    CHECK(tool.filterMode() == AssetFilterMode::All);
}

TEST_CASE("AssetEditorTool initial search query is empty", "[AssetEditorTool][state]") {
    AssetEditorTool tool;
    CHECK(tool.searchQuery().empty());
}

TEST_CASE("AssetEditorTool initial stats are zero/false", "[AssetEditorTool][state]") {
    AssetEditorTool tool;
    CHECK(tool.totalAssetCount()    == 0);
    CHECK(tool.selectionCount()     == 0);
    CHECK(tool.filteredAssetCount() == 0);
    CHECK(tool.isDirty()            == false);
}

// ─────────────────────────────────────────────────────────────────
// AssetEditorTool — lifecycle
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AssetEditorTool initialize transitions Unloaded -> Ready", "[AssetEditorTool][lifecycle]") {
    AssetEditorTool tool;
    REQUIRE(tool.state() == HostedToolState::Unloaded);
    bool ok = tool.initialize();
    CHECK(ok);
    CHECK(tool.state() == HostedToolState::Ready);
}

TEST_CASE("AssetEditorTool initialize returns false if already Ready", "[AssetEditorTool][lifecycle]") {
    AssetEditorTool tool;
    tool.initialize();
    bool second = tool.initialize();
    CHECK_FALSE(second);
    CHECK(tool.state() == HostedToolState::Ready);
}

TEST_CASE("AssetEditorTool activate transitions Ready -> Active", "[AssetEditorTool][lifecycle]") {
    AssetEditorTool tool;
    tool.initialize();
    tool.activate();
    CHECK(tool.state() == HostedToolState::Active);
}

TEST_CASE("AssetEditorTool suspend transitions Active -> Suspended", "[AssetEditorTool][lifecycle]") {
    AssetEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.suspend();
    CHECK(tool.state() == HostedToolState::Suspended);
}

TEST_CASE("AssetEditorTool can re-activate from Suspended", "[AssetEditorTool][lifecycle]") {
    AssetEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.suspend();
    tool.activate();
    CHECK(tool.state() == HostedToolState::Active);
}

TEST_CASE("AssetEditorTool shutdown resets to Unloaded", "[AssetEditorTool][lifecycle]") {
    AssetEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.shutdown();
    CHECK(tool.state() == HostedToolState::Unloaded);
}

TEST_CASE("AssetEditorTool shutdown resets filter and query", "[AssetEditorTool][lifecycle]") {
    AssetEditorTool tool;
    tool.initialize();
    tool.setFilterMode(AssetFilterMode::Textures);
    tool.setSearchQuery("hero");
    tool.shutdown();
    CHECK(tool.filterMode()   == AssetFilterMode::All);
    CHECK(tool.searchQuery()  == "");
}

TEST_CASE("AssetEditorTool shutdown resets stats", "[AssetEditorTool][lifecycle]") {
    AssetEditorTool tool;
    tool.initialize();
    tool.setTotalAssetCount(500);
    tool.setSelectionCount(10);
    tool.markDirty();
    tool.shutdown();
    CHECK(tool.totalAssetCount() == 0);
    CHECK(tool.selectionCount()  == 0);
    CHECK(tool.isDirty()         == false);
}

TEST_CASE("AssetEditorTool can reinitialize after shutdown", "[AssetEditorTool][lifecycle]") {
    AssetEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.shutdown();
    bool ok = tool.initialize();
    CHECK(ok);
    CHECK(tool.state() == HostedToolState::Ready);
}

TEST_CASE("AssetEditorTool activate when Unloaded has no effect", "[AssetEditorTool][lifecycle]") {
    AssetEditorTool tool;
    tool.activate();
    CHECK(tool.state() == HostedToolState::Unloaded);
}

TEST_CASE("AssetEditorTool suspend when Ready has no effect", "[AssetEditorTool][lifecycle]") {
    AssetEditorTool tool;
    tool.initialize();
    tool.suspend();
    CHECK(tool.state() == HostedToolState::Ready);
}

// ─────────────────────────────────────────────────────────────────
// AssetEditorTool — update
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AssetEditorTool update is a no-op (event-driven design)", "[AssetEditorTool][update]") {
    AssetEditorTool tool;
    tool.initialize();
    tool.activate();
    // update must not throw or crash; state unchanged
    tool.update(0.016f);
    tool.update(0.016f);
    CHECK(tool.state() == HostedToolState::Active);
}

// ─────────────────────────────────────────────────────────────────
// AssetEditorTool — filter mode
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AssetEditorTool setFilterMode persists mode", "[AssetEditorTool][filter]") {
    AssetEditorTool tool;
    tool.initialize();

    tool.setFilterMode(AssetFilterMode::Textures);
    CHECK(tool.filterMode() == AssetFilterMode::Textures);

    tool.setFilterMode(AssetFilterMode::Materials);
    CHECK(tool.filterMode() == AssetFilterMode::Materials);

    tool.setFilterMode(AssetFilterMode::Meshes);
    CHECK(tool.filterMode() == AssetFilterMode::Meshes);

    tool.setFilterMode(AssetFilterMode::Audio);
    CHECK(tool.filterMode() == AssetFilterMode::Audio);

    tool.setFilterMode(AssetFilterMode::Scripts);
    CHECK(tool.filterMode() == AssetFilterMode::Scripts);

    tool.setFilterMode(AssetFilterMode::Prefabs);
    CHECK(tool.filterMode() == AssetFilterMode::Prefabs);

    tool.setFilterMode(AssetFilterMode::All);
    CHECK(tool.filterMode() == AssetFilterMode::All);
}

// ─────────────────────────────────────────────────────────────────
// AssetEditorTool — search query
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AssetEditorTool setSearchQuery persists value", "[AssetEditorTool][filter]") {
    AssetEditorTool tool;
    tool.initialize();

    tool.setSearchQuery("hero_sword");
    CHECK(tool.searchQuery() == "hero_sword");

    tool.setSearchQuery("");
    CHECK(tool.searchQuery() == "");
}

// ─────────────────────────────────────────────────────────────────
// AssetEditorTool — asset counts
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AssetEditorTool setTotalAssetCount persists value", "[AssetEditorTool][stats]") {
    AssetEditorTool tool;
    tool.initialize();
    tool.setTotalAssetCount(1234);
    CHECK(tool.totalAssetCount() == 1234);
    CHECK(tool.stats().totalAssetCount == 1234);
}

TEST_CASE("AssetEditorTool setSelectionCount persists value", "[AssetEditorTool][stats]") {
    AssetEditorTool tool;
    tool.initialize();
    tool.setSelectionCount(7);
    CHECK(tool.selectionCount() == 7);
    CHECK(tool.stats().selectionCount == 7);
}

TEST_CASE("AssetEditorTool setFilteredAssetCount persists value", "[AssetEditorTool][stats]") {
    AssetEditorTool tool;
    tool.initialize();
    tool.setFilteredAssetCount(42);
    CHECK(tool.filteredAssetCount() == 42);
    CHECK(tool.stats().filteredAssetCount == 42);
}

// ─────────────────────────────────────────────────────────────────
// AssetEditorTool — dirty flag
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AssetEditorTool markDirty and clearDirty work correctly", "[AssetEditorTool][dirty]") {
    AssetEditorTool tool;
    tool.initialize();

    CHECK_FALSE(tool.isDirty());
    tool.markDirty();
    CHECK(tool.isDirty());
    CHECK(tool.stats().isDirty == true);
    tool.clearDirty();
    CHECK_FALSE(tool.isDirty());
    CHECK(tool.stats().isDirty == false);
}

// ─────────────────────────────────────────────────────────────────
// AssetEditorTool — project adapter hooks
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AssetEditorTool onProjectLoaded resets stats and state", "[AssetEditorTool][project]") {
    AssetEditorTool tool;
    tool.initialize();
    tool.setTotalAssetCount(999);
    tool.setSelectionCount(5);
    tool.setFilterMode(AssetFilterMode::Audio);
    tool.setSearchQuery("music");
    tool.markDirty();

    tool.onProjectLoaded("proj.nova");

    CHECK(tool.totalAssetCount() == 0);
    CHECK(tool.selectionCount()  == 0);
    CHECK(tool.filterMode()      == AssetFilterMode::All);
    CHECK(tool.searchQuery()     == "");
    CHECK(tool.isDirty()         == false);
}

TEST_CASE("AssetEditorTool onProjectUnloaded resets stats and state", "[AssetEditorTool][project]") {
    AssetEditorTool tool;
    tool.initialize();
    tool.onProjectLoaded("proj.nova");
    tool.setTotalAssetCount(200);
    tool.setFilterMode(AssetFilterMode::Textures);
    tool.setSearchQuery("wood");

    tool.onProjectUnloaded();

    CHECK(tool.totalAssetCount() == 0);
    CHECK(tool.filterMode()      == AssetFilterMode::All);
    CHECK(tool.searchQuery()     == "");
    CHECK(tool.isDirty()         == false);
}

// ─────────────────────────────────────────────────────────────────
// AssetEditorTool — IHostedTool polymorphism
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AssetEditorTool is usable through IHostedTool interface", "[AssetEditorTool][polymorphism]") {
    auto tool = std::make_unique<AssetEditorTool>();
    IHostedTool* base = tool.get();

    CHECK(base->toolId()              == "workspace.asset_editor");
    CHECK(base->descriptor().isValid());
    CHECK(base->state()               == HostedToolState::Unloaded);

    REQUIRE(base->initialize());
    CHECK(base->state()   == HostedToolState::Ready);

    base->activate();
    CHECK(base->state()   == HostedToolState::Active);

    base->update(0.016f);

    base->suspend();
    CHECK(base->state()   == HostedToolState::Suspended);

    base->activate();
    CHECK(base->state()   == HostedToolState::Active);

    base->onProjectLoaded("project.x");
    base->onProjectUnloaded();

    base->shutdown();
    CHECK(base->state()   == HostedToolState::Unloaded);
}

TEST_CASE("AssetEditorTool category name is ProjectBrowser", "[AssetEditorTool][category]") {
    AssetEditorTool tool;
    CHECK(std::string(hostedToolCategoryName(tool.descriptor().category)) == "ProjectBrowser");
}
