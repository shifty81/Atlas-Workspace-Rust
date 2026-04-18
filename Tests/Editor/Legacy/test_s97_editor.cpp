// S97 editor tests: SceneStreaming, LODEditorPanel, WorldPartition
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/WorldPartition.h"
#include "NF/Editor/SceneStreaming.h"

using namespace NF;

// ── SceneStreaming ────────────────────────────────────────────────────────────

TEST_CASE("StreamingCellType names", "[Editor][S97]") {
    REQUIRE(std::string(streamingCellTypeName(StreamingCellType::StaticMesh))  == "StaticMesh");
    REQUIRE(std::string(streamingCellTypeName(StreamingCellType::Landscape))   == "Landscape");
    REQUIRE(std::string(streamingCellTypeName(StreamingCellType::Audio))       == "Audio");
    REQUIRE(std::string(streamingCellTypeName(StreamingCellType::Navigation))  == "Navigation");
    REQUIRE(std::string(streamingCellTypeName(StreamingCellType::Foliage))     == "Foliage");
    REQUIRE(std::string(streamingCellTypeName(StreamingCellType::Lighting))    == "Lighting");
}

TEST_CASE("StreamingLoadState names", "[Editor][S97]") {
    REQUIRE(std::string(streamingLoadStateName(StreamingLoadState::Unloaded))  == "Unloaded");
    REQUIRE(std::string(streamingLoadStateName(StreamingLoadState::Loading))   == "Loading");
    REQUIRE(std::string(streamingLoadStateName(StreamingLoadState::Loaded))    == "Loaded");
    REQUIRE(std::string(streamingLoadStateName(StreamingLoadState::Unloading)) == "Unloading");
    REQUIRE(std::string(streamingLoadStateName(StreamingLoadState::Error))     == "Error");
}

TEST_CASE("StreamingPriority names", "[Editor][S97]") {
    REQUIRE(std::string(streamingPriorityName(StreamingPriority::Low))      == "Low");
    REQUIRE(std::string(streamingPriorityName(StreamingPriority::Normal))   == "Normal");
    REQUIRE(std::string(streamingPriorityName(StreamingPriority::High))     == "High");
    REQUIRE(std::string(streamingPriorityName(StreamingPriority::Critical)) == "Critical");
}

TEST_CASE("StreamingCell defaults", "[Editor][S97]") {
    StreamingCell c("zone_A", StreamingCellType::StaticMesh);
    REQUIRE(c.name()         == "zone_A");
    REQUIRE(c.type()         == StreamingCellType::StaticMesh);
    REQUIRE(c.state()        == StreamingLoadState::Unloaded);
    REQUIRE(c.priority()     == StreamingPriority::Normal);
    REQUIRE(c.loadRadius()   == 500.0f);
    REQUIRE(c.unloadRadius() == 600.0f);
    REQUIRE(c.isEnabled());
    REQUIRE(!c.isLoaded());
}

TEST_CASE("StreamingCell mutation", "[Editor][S97]") {
    StreamingCell c("zone_B", StreamingCellType::Audio);
    c.setState(StreamingLoadState::Loaded);
    c.setPriority(StreamingPriority::High);
    c.setLoadRadius(200.0f);
    c.setUnloadRadius(250.0f);
    c.setEnabled(false);
    REQUIRE(c.state()        == StreamingLoadState::Loaded);
    REQUIRE(c.priority()     == StreamingPriority::High);
    REQUIRE(c.loadRadius()   == 200.0f);
    REQUIRE(c.unloadRadius() == 250.0f);
    REQUIRE(!c.isEnabled());
    REQUIRE(c.isLoaded());
}

TEST_CASE("SceneStreaming add/remove cell", "[Editor][S97]") {
    SceneStreaming ss;
    StreamingCell c("c1", StreamingCellType::Foliage);
    REQUIRE(ss.addCell(c));
    REQUIRE(ss.cellCount() == 1u);
    REQUIRE(!ss.addCell(c));
    REQUIRE(ss.removeCell("c1"));
    REQUIRE(ss.cellCount() == 0u);
}

TEST_CASE("SceneStreaming counts", "[Editor][S97]") {
    SceneStreaming ss;
    StreamingCell c1("a", StreamingCellType::StaticMesh); c1.setState(StreamingLoadState::Loaded);
    StreamingCell c2("b", StreamingCellType::Audio);
    StreamingCell c3("c", StreamingCellType::StaticMesh);  c3.setPriority(StreamingPriority::High);
    ss.addCell(c1); ss.addCell(c2); ss.addCell(c3);
    REQUIRE(ss.cellCount()   == 3u);
    REQUIRE(ss.loadedCount() == 1u);
    REQUIRE(ss.countByType(StreamingCellType::StaticMesh)      == 2u);
    REQUIRE(ss.countByState(StreamingLoadState::Unloaded)      == 2u);
    REQUIRE(ss.countByPriority(StreamingPriority::High)        == 1u);
}

// ── LODEditorPanel ───────────────────────────────────────────────────────────

TEST_CASE("LODTransitionType names", "[Editor][S97]") {
    REQUIRE(std::string(lodTransitionTypeName(LODTransitionType::Discrete))   == "Discrete");
    REQUIRE(std::string(lodTransitionTypeName(LODTransitionType::Dithered))   == "Dithered");
    REQUIRE(std::string(lodTransitionTypeName(LODTransitionType::ScreenSize)) == "ScreenSize");
    REQUIRE(std::string(lodTransitionTypeName(LODTransitionType::Distance))   == "Distance");
    REQUIRE(std::string(lodTransitionTypeName(LODTransitionType::Custom))     == "Custom");
}

TEST_CASE("LODGenerationMode names", "[Editor][S97]") {
    REQUIRE(std::string(lodGenerationModeName(LODGenerationMode::Manual))   == "Manual");
    REQUIRE(std::string(lodGenerationModeName(LODGenerationMode::Auto))     == "Auto");
    REQUIRE(std::string(lodGenerationModeName(LODGenerationMode::Nanite))   == "Nanite");
    REQUIRE(std::string(lodGenerationModeName(LODGenerationMode::Impostor)) == "Impostor");
}

TEST_CASE("LODGroupPreset names", "[Editor][S97]") {
    REQUIRE(std::string(lodGroupPresetName(LODGroupPreset::None))       == "None");
    REQUIRE(std::string(lodGroupPresetName(LODGroupPreset::SmallProp))  == "SmallProp");
    REQUIRE(std::string(lodGroupPresetName(LODGroupPreset::Character))  == "Character");
    REQUIRE(std::string(lodGroupPresetName(LODGroupPreset::Vehicle))    == "Vehicle");
    REQUIRE(std::string(lodGroupPresetName(LODGroupPreset::Building))   == "Building");
    REQUIRE(std::string(lodGroupPresetName(LODGroupPreset::Landscape))  == "Landscape");
}

TEST_CASE("LODPanelLevel defaults", "[Editor][S97]") {
    LODPanelLevel l(0, 1.0f);
    REQUIRE(l.levelIndex()        == 0u);
    REQUIRE(l.screenSize()        == 1.0f);
    REQUIRE(l.triangleReduction() == 0.5f);
    REQUIRE(l.isEnabled());
}

TEST_CASE("LODPanelConfig add levels", "[Editor][S97]") {
    LODPanelConfig cfg("SM_Rock");
    LODPanelLevel l0(0, 1.0f);
    LODPanelLevel l1(1, 0.5f);
    REQUIRE(cfg.addLevel(l0));
    REQUIRE(cfg.addLevel(l1));
    REQUIRE(!cfg.addLevel(l0));
    REQUIRE(cfg.levelCount()        == 2u);
    REQUIRE(cfg.enabledLevelCount() == 2u);
}

TEST_CASE("LODPanelConfig defaults", "[Editor][S97]") {
    LODPanelConfig cfg("SM_Wall");
    REQUIRE(cfg.assetName()       == "SM_Wall");
    REQUIRE(cfg.transitionType()  == LODTransitionType::ScreenSize);
    REQUIRE(cfg.generationMode()  == LODGenerationMode::Auto);
    REQUIRE(cfg.groupPreset()     == LODGroupPreset::None);
    REQUIRE(cfg.minDrawDistance() == 0.0f);
    REQUIRE(cfg.maxDrawDistance() == 50000.0f);
}

TEST_CASE("LODEditorPanel add/remove", "[Editor][S97]") {
    LODEditorPanel panel;
    LODPanelConfig cfg("SK_Player");
    REQUIRE(panel.addConfig(cfg));
    REQUIRE(panel.configCount() == 1u);
    REQUIRE(!panel.addConfig(cfg));
    REQUIRE(panel.removeConfig("SK_Player"));
    REQUIRE(panel.configCount() == 0u);
}

TEST_CASE("LODEditorPanel counts", "[Editor][S97]") {
    LODEditorPanel panel;
    LODPanelConfig c1("A"); c1.setGroupPreset(LODGroupPreset::Character); c1.setGenerationMode(LODGenerationMode::Auto);
    LODPanelConfig c2("B"); c2.setGroupPreset(LODGroupPreset::Vehicle);   c2.setGenerationMode(LODGenerationMode::Nanite);
    LODPanelConfig c3("C"); c3.setGroupPreset(LODGroupPreset::Character); c3.setGenerationMode(LODGenerationMode::Manual);
    panel.addConfig(c1); panel.addConfig(c2); panel.addConfig(c3);
    REQUIRE(panel.countByPreset(LODGroupPreset::Character)           == 2u);
    REQUIRE(panel.countByGenerationMode(LODGenerationMode::Nanite)   == 1u);
}

// ── WorldPartition ───────────────────────────────────────────────────────────

TEST_CASE("WorldPartitionCellSize names", "[Editor][S97]") {
    REQUIRE(std::string(worldPartitionCellSizeName(WorldPartitionCellSize::Small))  == "Small");
    REQUIRE(std::string(worldPartitionCellSizeName(WorldPartitionCellSize::Medium)) == "Medium");
    REQUIRE(std::string(worldPartitionCellSizeName(WorldPartitionCellSize::Large))  == "Large");
    REQUIRE(std::string(worldPartitionCellSizeName(WorldPartitionCellSize::Custom)) == "Custom");
}

TEST_CASE("WorldPartitionLoadStrategy names", "[Editor][S97]") {
    REQUIRE(std::string(worldPartitionLoadStrategyName(WorldPartitionLoadStrategy::Distance))     == "Distance");
    REQUIRE(std::string(worldPartitionLoadStrategyName(WorldPartitionLoadStrategy::Region))       == "Region");
    REQUIRE(std::string(worldPartitionLoadStrategyName(WorldPartitionLoadStrategy::AlwaysLoaded)) == "AlwaysLoaded");
    REQUIRE(std::string(worldPartitionLoadStrategyName(WorldPartitionLoadStrategy::DataLayer))    == "DataLayer");
    REQUIRE(std::string(worldPartitionLoadStrategyName(WorldPartitionLoadStrategy::Custom))       == "Custom");
}

TEST_CASE("WorldPartitionRegionState names", "[Editor][S97]") {
    REQUIRE(std::string(worldPartitionRegionStateName(WorldPartitionRegionState::Active))    == "Active");
    REQUIRE(std::string(worldPartitionRegionStateName(WorldPartitionRegionState::Inactive))  == "Inactive");
    REQUIRE(std::string(worldPartitionRegionStateName(WorldPartitionRegionState::Loading))   == "Loading");
    REQUIRE(std::string(worldPartitionRegionStateName(WorldPartitionRegionState::Unloading)) == "Unloading");
    REQUIRE(std::string(worldPartitionRegionStateName(WorldPartitionRegionState::Error))     == "Error");
}

TEST_CASE("WorldPartitionRegion defaults", "[Editor][S97]") {
    WorldPartitionRegion r("Open_World_East");
    REQUIRE(r.name()         == "Open_World_East");
    REQUIRE(r.state()        == WorldPartitionRegionState::Inactive);
    REQUIRE(r.loadStrategy() == WorldPartitionLoadStrategy::Distance);
    REQUIRE(r.cellSize()     == WorldPartitionCellSize::Medium);
    REQUIRE(r.extentX()      == 1000.0f);
    REQUIRE(r.extentY()      == 1000.0f);
    REQUIRE(r.isEnabled());
    REQUIRE(!r.isActive());
}

TEST_CASE("WorldPartitionRegion mutation", "[Editor][S97]") {
    WorldPartitionRegion r("Hub");
    r.setState(WorldPartitionRegionState::Active);
    r.setLoadStrategy(WorldPartitionLoadStrategy::AlwaysLoaded);
    r.setCellSize(WorldPartitionCellSize::Large);
    r.setExtentX(2000.0f);
    r.setExtentY(3000.0f);
    r.setEnabled(false);
    REQUIRE(r.isActive());
    REQUIRE(r.loadStrategy() == WorldPartitionLoadStrategy::AlwaysLoaded);
    REQUIRE(r.cellSize()     == WorldPartitionCellSize::Large);
    REQUIRE(r.extentX()      == 2000.0f);
    REQUIRE(r.extentY()      == 3000.0f);
    REQUIRE(!r.isEnabled());
}

TEST_CASE("WorldPartition add/remove region", "[Editor][S97]") {
    WorldPartition wp;
    WorldPartitionRegion r("zone_1");
    REQUIRE(wp.addRegion(r));
    REQUIRE(wp.regionCount() == 1u);
    REQUIRE(!wp.addRegion(r));
    REQUIRE(wp.removeRegion("zone_1"));
    REQUIRE(wp.regionCount() == 0u);
}

TEST_CASE("WorldPartition counts", "[Editor][S97]") {
    WorldPartition wp;
    WorldPartitionRegion r1("a"); r1.setState(WorldPartitionRegionState::Active);
    WorldPartitionRegion r2("b"); r2.setState(WorldPartitionRegionState::Loading);
    r2.setLoadStrategy(WorldPartitionLoadStrategy::Region);
    WorldPartitionRegion r3("c"); r3.setState(WorldPartitionRegionState::Active);
    wp.addRegion(r1); wp.addRegion(r2); wp.addRegion(r3);
    REQUIRE(wp.regionCount() == 3u);
    REQUIRE(wp.activeCount() == 2u);
    REQUIRE(wp.countByState(WorldPartitionRegionState::Loading)             == 1u);
    REQUIRE(wp.countByLoadStrategy(WorldPartitionLoadStrategy::Region)      == 1u);
}
