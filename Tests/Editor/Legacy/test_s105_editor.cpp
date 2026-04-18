// S105 editor tests: PerformanceBudgetEditor, MemoryProfilerPanel, RenderStatsPanel
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/PerformanceBudgetEditor.h"

using namespace NF;

// ── PerformanceBudgetEditor ──────────────────────────────────────────────────

TEST_CASE("BudgetCategory names", "[Editor][S105]") {
    REQUIRE(std::string(budgetCategoryName(BudgetCategory::CPU))              == "CPU");
    REQUIRE(std::string(budgetCategoryName(BudgetCategory::GPU))              == "GPU");
    REQUIRE(std::string(budgetCategoryName(BudgetCategory::Memory))           == "Memory");
    REQUIRE(std::string(budgetCategoryName(BudgetCategory::DrawCalls))        == "DrawCalls");
    REQUIRE(std::string(budgetCategoryName(BudgetCategory::Triangles))        == "Triangles");
    REQUIRE(std::string(budgetCategoryName(BudgetCategory::TextureMemory))    == "TextureMemory");
    REQUIRE(std::string(budgetCategoryName(BudgetCategory::AudioMemory))      == "AudioMemory");
    REQUIRE(std::string(budgetCategoryName(BudgetCategory::NetworkBandwidth)) == "NetworkBandwidth");
}

TEST_CASE("BudgetStatus names", "[Editor][S105]") {
    REQUIRE(std::string(budgetStatusName(BudgetStatus::Ok))       == "Ok");
    REQUIRE(std::string(budgetStatusName(BudgetStatus::Warning))  == "Warning");
    REQUIRE(std::string(budgetStatusName(BudgetStatus::Critical)) == "Critical");
    REQUIRE(std::string(budgetStatusName(BudgetStatus::Exceeded)) == "Exceeded");
}

TEST_CASE("BudgetTargetPlatform names", "[Editor][S105]") {
    REQUIRE(std::string(budgetTargetPlatformName(BudgetTargetPlatform::PC))      == "PC");
    REQUIRE(std::string(budgetTargetPlatformName(BudgetTargetPlatform::Console)) == "Console");
    REQUIRE(std::string(budgetTargetPlatformName(BudgetTargetPlatform::Mobile))  == "Mobile");
    REQUIRE(std::string(budgetTargetPlatformName(BudgetTargetPlatform::VR))      == "VR");
    REQUIRE(std::string(budgetTargetPlatformName(BudgetTargetPlatform::WebGL))   == "WebGL");
}

TEST_CASE("BudgetEntry defaults", "[Editor][S105]") {
    BudgetEntry entry(BudgetCategory::GPU, 16.67f);
    REQUIRE(entry.category()      == BudgetCategory::GPU);
    REQUIRE(entry.budget()        == 16.67f);
    REQUIRE(entry.current()       == 0.0f);
    REQUIRE(entry.warnThreshold() == 0.8f);
    REQUIRE(entry.isEnabled());
    REQUIRE(entry.utilization()   == 0.0f);
    REQUIRE(entry.status()        == BudgetStatus::Ok);
}

TEST_CASE("BudgetEntry status transitions", "[Editor][S105]") {
    BudgetEntry entry(BudgetCategory::CPU, 10.0f, 0.7f);
    entry.setCurrent(5.0f);
    REQUIRE(entry.utilization() == 0.5f);
    REQUIRE(entry.status()      == BudgetStatus::Ok);

    entry.setCurrent(8.0f);
    REQUIRE(entry.status()      == BudgetStatus::Warning);

    entry.setCurrent(9.5f);
    REQUIRE(entry.status()      == BudgetStatus::Critical);

    entry.setCurrent(11.0f);
    REQUIRE(entry.status()      == BudgetStatus::Exceeded);
}

TEST_CASE("BudgetEntry mutation", "[Editor][S105]") {
    BudgetEntry entry(BudgetCategory::Memory, 4096.0f);
    entry.setCurrent(2048.0f);
    entry.setEnabled(false);
    entry.setWarnThreshold(0.6f);
    REQUIRE(!entry.isEnabled());
    REQUIRE(entry.warnThreshold() == 0.6f);
    REQUIRE(entry.utilization()   == 0.5f);
}

TEST_CASE("PerformanceBudgetEditor add/remove", "[Editor][S105]") {
    PerformanceBudgetEditor ed;
    REQUIRE(ed.addEntry(BudgetEntry(BudgetCategory::CPU, 10.0f)));
    REQUIRE(ed.addEntry(BudgetEntry(BudgetCategory::GPU, 16.67f)));
    REQUIRE(!ed.addEntry(BudgetEntry(BudgetCategory::CPU, 5.0f)));
    REQUIRE(ed.entryCount() == 2u);
    REQUIRE(ed.removeEntry(BudgetCategory::CPU));
    REQUIRE(ed.entryCount() == 1u);
    REQUIRE(!ed.removeEntry(BudgetCategory::CPU));
}

TEST_CASE("PerformanceBudgetEditor platform and FPS", "[Editor][S105]") {
    PerformanceBudgetEditor ed;
    ed.setTargetPlatform(BudgetTargetPlatform::Mobile);
    ed.setTargetFPS(30.0f);
    REQUIRE(ed.targetPlatform() == BudgetTargetPlatform::Mobile);
    REQUIRE(ed.targetFPS()      == 30.0f);
}

TEST_CASE("PerformanceBudgetEditor counts", "[Editor][S105]") {
    PerformanceBudgetEditor ed;
    BudgetEntry e1(BudgetCategory::CPU, 10.0f);   e1.setCurrent(11.0f); // Exceeded
    BudgetEntry e2(BudgetCategory::GPU, 16.67f);  e2.setCurrent(14.0f); // Warning/Critical
    BudgetEntry e3(BudgetCategory::Memory, 4096.0f); e3.setCurrent(1000.0f); // Ok
    BudgetEntry e4(BudgetCategory::DrawCalls, 2000.0f); e4.setCurrent(1900.0f); e4.setEnabled(false); // Disabled, Critical
    ed.addEntry(e1); ed.addEntry(e2); ed.addEntry(e3); ed.addEntry(e4);
    REQUIRE(ed.entryCount()    == 4u);
    REQUIRE(ed.enabledCount()  == 3u);
    REQUIRE(ed.exceededCount() == 1u);
    REQUIRE(ed.okCount()       == 1u);
    auto* found = ed.findEntry(BudgetCategory::Memory);
    REQUIRE(found != nullptr);
    REQUIRE(found->budget() == 4096.0f);
    REQUIRE(ed.findEntry(BudgetCategory::Triangles) == nullptr);
}

// ── MemoryProfilerPanel ──────────────────────────────────────────────────────

TEST_CASE("MemoryCategory names", "[Editor][S105]") {
    REQUIRE(std::string(memoryCategoryName(MemoryCategory::Textures))   == "Textures");
    REQUIRE(std::string(memoryCategoryName(MemoryCategory::Meshes))     == "Meshes");
    REQUIRE(std::string(memoryCategoryName(MemoryCategory::Audio))      == "Audio");
    REQUIRE(std::string(memoryCategoryName(MemoryCategory::Animations)) == "Animations");
    REQUIRE(std::string(memoryCategoryName(MemoryCategory::Scripts))    == "Scripts");
    REQUIRE(std::string(memoryCategoryName(MemoryCategory::Physics))    == "Physics");
    REQUIRE(std::string(memoryCategoryName(MemoryCategory::AI))         == "AI");
    REQUIRE(std::string(memoryCategoryName(MemoryCategory::Misc))       == "Misc");
}

TEST_CASE("MemorySortOrder names", "[Editor][S105]") {
    REQUIRE(std::string(memorySortOrderName(MemorySortOrder::BySize))     == "BySize");
    REQUIRE(std::string(memorySortOrderName(MemorySortOrder::ByCategory)) == "ByCategory");
    REQUIRE(std::string(memorySortOrderName(MemorySortOrder::ByName))     == "ByName");
    REQUIRE(std::string(memorySortOrderName(MemorySortOrder::ByAge))      == "ByAge");
    REQUIRE(std::string(memorySortOrderName(MemorySortOrder::ByRefCount)) == "ByRefCount");
}

TEST_CASE("MemorySnapshotState names", "[Editor][S105]") {
    REQUIRE(std::string(memorySnapshotStateName(MemorySnapshotState::Empty))     == "Empty");
    REQUIRE(std::string(memorySnapshotStateName(MemorySnapshotState::Capturing)) == "Capturing");
    REQUIRE(std::string(memorySnapshotStateName(MemorySnapshotState::Ready))     == "Ready");
    REQUIRE(std::string(memorySnapshotStateName(MemorySnapshotState::Comparing)) == "Comparing");
}

TEST_CASE("MemoryRecord defaults", "[Editor][S105]") {
    MemoryRecord rec("grass_diffuse", MemoryCategory::Textures, 4194304);
    REQUIRE(rec.name()       == "grass_diffuse");
    REQUIRE(rec.category()   == MemoryCategory::Textures);
    REQUIRE(rec.bytes()      == 4194304u);
    REQUIRE(rec.refCount()   == 1u);
    REQUIRE(rec.isResident());
    REQUIRE(!rec.isFlagged());
}

TEST_CASE("MemoryRecord mutation", "[Editor][S105]") {
    MemoryRecord rec("huge_texture", MemoryCategory::Textures, 67108864);
    rec.setRefCount(3);
    rec.setResident(false);
    rec.setFlagged(true);
    REQUIRE(rec.refCount()   == 3u);
    REQUIRE(!rec.isResident());
    REQUIRE(rec.isFlagged());
}

TEST_CASE("MemoryProfilerPanel defaults", "[Editor][S105]") {
    MemoryProfilerPanel panel;
    REQUIRE(panel.sortOrder()      == MemorySortOrder::BySize);
    REQUIRE(panel.snapshotState()  == MemorySnapshotState::Empty);
    REQUIRE(!panel.isFiltering());
    REQUIRE(panel.recordCount()    == 0u);
    REQUIRE(panel.totalBytes()     == 0u);
}

TEST_CASE("MemoryProfilerPanel add records and counts", "[Editor][S105]") {
    MemoryProfilerPanel panel;
    MemoryRecord r1("tex_a",  MemoryCategory::Textures, 4096); r1.setFlagged(true);
    MemoryRecord r2("mesh_b", MemoryCategory::Meshes,   2048);
    MemoryRecord r3("tex_c",  MemoryCategory::Textures, 8192); r3.setFlagged(true);
    panel.addRecord(r1); panel.addRecord(r2); panel.addRecord(r3);
    REQUIRE(panel.recordCount()                           == 3u);
    REQUIRE(panel.totalBytes()                            == 14336u);
    REQUIRE(panel.flaggedCount()                          == 2u);
    REQUIRE(panel.countByCategory(MemoryCategory::Textures)== 2u);
    REQUIRE(panel.bytesForCategory(MemoryCategory::Textures)== 12288u);
    REQUIRE(panel.countByCategory(MemoryCategory::Meshes)  == 1u);
}

TEST_CASE("MemoryProfilerPanel mutation", "[Editor][S105]") {
    MemoryProfilerPanel panel;
    panel.setSortOrder(MemorySortOrder::ByCategory);
    panel.setSnapshotState(MemorySnapshotState::Ready);
    panel.setFilterCategory(MemoryCategory::Textures);
    REQUIRE(panel.sortOrder()     == MemorySortOrder::ByCategory);
    REQUIRE(panel.snapshotState() == MemorySnapshotState::Ready);
    REQUIRE(panel.isFiltering());
    panel.clearFilter();
    REQUIRE(!panel.isFiltering());
}

TEST_CASE("MemoryProfilerPanel clear", "[Editor][S105]") {
    MemoryProfilerPanel panel;
    panel.addRecord(MemoryRecord("a", MemoryCategory::Audio, 1024));
    panel.addRecord(MemoryRecord("b", MemoryCategory::Audio, 2048));
    REQUIRE(panel.recordCount() == 2u);
    panel.clearRecords();
    REQUIRE(panel.recordCount() == 0u);
    REQUIRE(panel.totalBytes()  == 0u);
}

// ── RenderStatsPanel ─────────────────────────────────────────────────────────

TEST_CASE("RenderStatCategory names", "[Editor][S105]") {
    REQUIRE(std::string(renderStatCategoryName(RenderStatCategory::General))      == "General");
    REQUIRE(std::string(renderStatCategoryName(RenderStatCategory::Geometry))     == "Geometry");
    REQUIRE(std::string(renderStatCategoryName(RenderStatCategory::Shadow))       == "Shadow");
    REQUIRE(std::string(renderStatCategoryName(RenderStatCategory::PostProcess))  == "PostProcess");
    REQUIRE(std::string(renderStatCategoryName(RenderStatCategory::Particle))     == "Particle");
    REQUIRE(std::string(renderStatCategoryName(RenderStatCategory::Lighting))     == "Lighting");
    REQUIRE(std::string(renderStatCategoryName(RenderStatCategory::Transparency)) == "Transparency");
    REQUIRE(std::string(renderStatCategoryName(RenderStatCategory::Compute))      == "Compute");
}

TEST_CASE("RenderStatUnit names", "[Editor][S105]") {
    REQUIRE(std::string(renderStatUnitName(RenderStatUnit::Count))        == "Count");
    REQUIRE(std::string(renderStatUnitName(RenderStatUnit::Milliseconds)) == "Milliseconds");
    REQUIRE(std::string(renderStatUnitName(RenderStatUnit::Megabytes))    == "Megabytes");
    REQUIRE(std::string(renderStatUnitName(RenderStatUnit::Percentage))   == "Percentage");
    REQUIRE(std::string(renderStatUnitName(RenderStatUnit::MegaPixels))   == "MegaPixels");
}

TEST_CASE("RenderStatsPanelMode names", "[Editor][S105]") {
    REQUIRE(std::string(renderStatsPanelModeName(RenderStatsPanelMode::Compact))  == "Compact");
    REQUIRE(std::string(renderStatsPanelModeName(RenderStatsPanelMode::Detailed)) == "Detailed");
    REQUIRE(std::string(renderStatsPanelModeName(RenderStatsPanelMode::Graph))    == "Graph");
    REQUIRE(std::string(renderStatsPanelModeName(RenderStatsPanelMode::Overlay))  == "Overlay");
}

TEST_CASE("RenderStat defaults", "[Editor][S105]") {
    RenderStat stat("draw_calls", RenderStatCategory::Geometry, RenderStatUnit::Count);
    REQUIRE(stat.name()      == "draw_calls");
    REQUIRE(stat.category()  == RenderStatCategory::Geometry);
    REQUIRE(stat.unit()      == RenderStatUnit::Count);
    REQUIRE(stat.value()     == 0.0);
    REQUIRE(stat.isEnabled());
    REQUIRE(!stat.isPinned());
}

TEST_CASE("RenderStat mutation", "[Editor][S105]") {
    RenderStat stat("gpu_time", RenderStatCategory::General, RenderStatUnit::Milliseconds);
    stat.setValue(4.7);
    stat.setEnabled(false);
    stat.setPinned(true);
    REQUIRE(stat.value()     == 4.7);
    REQUIRE(!stat.isEnabled());
    REQUIRE(stat.isPinned());
}

TEST_CASE("RenderStatsPanel defaults", "[Editor][S105]") {
    RenderStatsPanel panel;
    REQUIRE(panel.mode()      == RenderStatsPanelMode::Compact);
    REQUIRE(panel.isVisible());
    REQUIRE(!panel.isFrozen());
    REQUIRE(panel.statCount() == 0u);
}

TEST_CASE("RenderStatsPanel mutation", "[Editor][S105]") {
    RenderStatsPanel panel;
    panel.setMode(RenderStatsPanelMode::Graph);
    panel.setVisible(false);
    panel.setFreezeStats(true);
    REQUIRE(panel.mode()      == RenderStatsPanelMode::Graph);
    REQUIRE(!panel.isVisible());
    REQUIRE(panel.isFrozen());
}

TEST_CASE("RenderStatsPanel add stats and counts", "[Editor][S105]") {
    RenderStatsPanel panel;
    RenderStat s1("draw_calls",  RenderStatCategory::Geometry, RenderStatUnit::Count);
    s1.setValue(1500.0); s1.setPinned(true);
    RenderStat s2("tri_count",   RenderStatCategory::Geometry, RenderStatUnit::Count);
    s2.setValue(250000.0);
    RenderStat s3("shadow_pass", RenderStatCategory::Shadow, RenderStatUnit::Milliseconds);
    s3.setValue(2.3); s3.setEnabled(false);
    RenderStat s4("gpu_total",   RenderStatCategory::General, RenderStatUnit::Milliseconds);
    s4.setValue(8.5); s4.setPinned(true);
    panel.addStat(s1); panel.addStat(s2); panel.addStat(s3); panel.addStat(s4);
    REQUIRE(panel.statCount()                                              == 4u);
    REQUIRE(panel.enabledCount()                                           == 3u);
    REQUIRE(panel.pinnedCount()                                            == 2u);
    REQUIRE(panel.countByCategory(RenderStatCategory::Geometry)            == 2u);
    REQUIRE(panel.totalValueForCategory(RenderStatCategory::Geometry)      == 251500.0);
    auto* found = panel.findStat("shadow_pass");
    REQUIRE(found != nullptr);
    REQUIRE(found->value() == 2.3);
    REQUIRE(panel.findStat("missing") == nullptr);
}
