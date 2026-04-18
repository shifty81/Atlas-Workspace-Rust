// Phase C — Panels Edit Real Data
//
// Tests for:
//   C.1: IDocumentPanel interface + DocumentPanelBase lifecycle
//   C.2: Six NovaForge gameplay panels with real data schemas
//   C.3: DocumentPropertyGrid schema-driven editing + validation
//   C.4: DocumentSavePipeline — save/revert/validate with notification sink

#include <catch2/catch_test_macros.hpp>

#include "NovaForge/EditorAdapter/IDocumentPanel.h"
#include "NovaForge/EditorAdapter/DocumentPanelBase.h"
#include "NovaForge/EditorAdapter/DocumentPropertyGrid.h"
#include "NovaForge/EditorAdapter/DocumentSavePipeline.h"
#include "NovaForge/EditorAdapter/NovaForgeDocument.h"

#include "NovaForge/EditorAdapter/Panels/EconomyPanel.h"
#include "NovaForge/EditorAdapter/Panels/InventoryRulesPanel.h"
#include "NovaForge/EditorAdapter/Panels/ShopPanel.h"
#include "NovaForge/EditorAdapter/Panels/MissionRulesPanel.h"
#include "NovaForge/EditorAdapter/Panels/ProgressionPanel.h"
#include "NovaForge/EditorAdapter/Panels/CharacterRulesPanel.h"

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// ── Helpers ─────────────────────────────────────────────────────────────────

static fs::path makeTempDir(const std::string& suffix) {
    auto base = fs::temp_directory_path() / ("phase_c_" + suffix);
    fs::create_directories(base);
    return base;
}

static std::unique_ptr<NovaForge::NovaForgeDocument> makeDoc(
    NovaForge::NovaForgeDocumentType type,
    const std::string& filePath)
{
    return std::make_unique<NovaForge::NovaForgeDocument>(type, filePath);
}

// ═══════════════════════════════════════════════════════════════════════════
//  C.1 — IDocumentPanel / DocumentPanelBase
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("C.1 EconomyPanel: initial state — no document bound", "[phase_c][c1]") {
    NovaForge::EconomyPanel panel;
    REQUIRE(!panel.hasDocument());
    REQUIRE(panel.boundDocument() == nullptr);
    REQUIRE(!panel.isDirty());
}

TEST_CASE("C.1 EconomyPanel: bindDocument sets bound document", "[phase_c][c1]") {
    NovaForge::EconomyPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::EconomyRules, "/tmp/eco.json");
    panel.bindDocument(doc.get());
    REQUIRE(panel.hasDocument());
    REQUIRE(panel.boundDocument() == doc.get());
}

TEST_CASE("C.1 EconomyPanel: bindDocument(nullptr) clears binding", "[phase_c][c1]") {
    NovaForge::EconomyPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::EconomyRules, "/tmp/eco.json");
    panel.bindDocument(doc.get());
    panel.bindDocument(nullptr);
    REQUIRE(!panel.hasDocument());
}

TEST_CASE("C.1 EconomyPanel: bindDocument loads default data", "[phase_c][c1]") {
    NovaForge::EconomyPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::EconomyRules, "/tmp/eco.json");
    panel.bindDocument(doc.get());
    REQUIRE(!panel.currencies().empty());
    REQUIRE(!panel.pricingRules().empty());
}

TEST_CASE("C.1 EconomyPanel: undo stack clears on bindDocument", "[phase_c][c1]") {
    NovaForge::EconomyPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::EconomyRules, "/tmp/eco.json");
    panel.bindDocument(doc.get());
    panel.setGlobalInflationRate(0.5f);
    REQUIRE(panel.undoStack().undoDepth() > 0);
    // Rebind clears the stack
    panel.bindDocument(doc.get());
    REQUIRE(panel.undoStack().undoDepth() == 0);
}

TEST_CASE("C.1 EconomyPanel: isDirty after field edit", "[phase_c][c1]") {
    NovaForge::EconomyPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::EconomyRules, "/tmp/eco.json");
    panel.bindDocument(doc.get());
    REQUIRE(!panel.isDirty());
    panel.setGlobalInflationRate(0.05f);
    REQUIRE(panel.isDirty());
}

TEST_CASE("C.1 EconomyPanel: dirty callback fires on edit", "[phase_c][c1]") {
    NovaForge::EconomyPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::EconomyRules, "/tmp/eco.json");
    panel.bindDocument(doc.get());

    bool fired = false;
    panel.setOnDirtyCallback([&](NovaForge::IDocumentPanel&) { fired = true; });
    panel.setGlobalInflationRate(0.1f);
    REQUIRE(fired);
}

TEST_CASE("C.1 EconomyPanel: dirtyTitle shows asterisk when dirty", "[phase_c][c1]") {
    NovaForge::EconomyPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::EconomyRules, "/tmp/eco.json");
    panel.bindDocument(doc.get());
    REQUIRE(panel.dirtyTitle() == "Economy");
    panel.setGlobalInflationRate(0.1f);
    REQUIRE(panel.dirtyTitle() == "Economy*");
}

TEST_CASE("C.1 EconomyPanel: undo restores previous state", "[phase_c][c1]") {
    NovaForge::EconomyPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::EconomyRules, "/tmp/eco.json");
    panel.bindDocument(doc.get());
    float original = panel.globalInflationRate();
    panel.setGlobalInflationRate(0.99f);
    REQUIRE(panel.undoStack().undo());
    REQUIRE(panel.globalInflationRate() == original);
}

TEST_CASE("C.1 EconomyPanel: redo reapplies change", "[phase_c][c1]") {
    NovaForge::EconomyPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::EconomyRules, "/tmp/eco.json");
    panel.bindDocument(doc.get());
    panel.setGlobalInflationRate(0.99f);
    panel.undoStack().undo();
    REQUIRE(panel.undoStack().redo());
    REQUIRE(panel.globalInflationRate() == 0.99f);
}

TEST_CASE("C.1 EconomyPanel: onProjectLoaded sets ready", "[phase_c][c1]") {
    NovaForge::EconomyPanel panel;
    REQUIRE(!panel.isReady());
    panel.onProjectLoaded("/proj/root");
    REQUIRE(panel.isReady());
    REQUIRE(panel.projectRoot() == "/proj/root");
}

TEST_CASE("C.1 EconomyPanel: onProjectUnloaded clears ready and unbinds", "[phase_c][c1]") {
    NovaForge::EconomyPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::EconomyRules, "/tmp/eco.json");
    panel.onProjectLoaded("/proj/root");
    panel.bindDocument(doc.get());
    panel.onProjectUnloaded();
    REQUIRE(!panel.isReady());
    REQUIRE(!panel.hasDocument());
}

// ═══════════════════════════════════════════════════════════════════════════
//  C.2 — InventoryRulesPanel
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("C.2 InventoryRulesPanel: panelId and title", "[phase_c][c2]") {
    NovaForge::InventoryRulesPanel panel;
    REQUIRE(panel.panelId()    == std::string("novaforge.inventory_rules"));
    REQUIRE(panel.panelTitle() == std::string("Inventory Rules"));
}

TEST_CASE("C.2 InventoryRulesPanel: default slots after bind", "[phase_c][c2]") {
    NovaForge::InventoryRulesPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::ItemDefinition, "/tmp/inv.json");
    panel.bindDocument(doc.get());
    REQUIRE(!panel.slots().empty());
}

TEST_CASE("C.2 InventoryRulesPanel: addSlot marks dirty", "[phase_c][c2]") {
    NovaForge::InventoryRulesPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::ItemDefinition, "/tmp/inv.json");
    panel.bindDocument(doc.get());
    panel.addSlot({"ring", "accessory", 2, false});
    REQUIRE(panel.isDirty());
    REQUIRE(panel.slots().size() >= 1u);
}

TEST_CASE("C.2 InventoryRulesPanel: setDefaultMaxSlots undo", "[phase_c][c2]") {
    NovaForge::InventoryRulesPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::ItemDefinition, "/tmp/inv.json");
    panel.bindDocument(doc.get());
    int orig = panel.defaultMaxSlots();
    panel.setDefaultMaxSlots(100);
    panel.undoStack().undo();
    REQUIRE(panel.defaultMaxSlots() == orig);
}

TEST_CASE("C.2 InventoryRulesPanel: invalid slot fails validation", "[phase_c][c2]") {
    NovaForge::InventoryRulesPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::ItemDefinition, "/tmp/inv.json");
    panel.bindDocument(doc.get());
    panel.addSlot({"", "any", 0, false}); // empty id, maxStack=0
    auto msgs = panel.validate();
    bool hasError = false;
    for (const auto& m : msgs)
        if (m.severity == NovaForge::DocumentPanelValidationSeverity::Error)
            hasError = true;
    REQUIRE(hasError);
}

// ═══════════════════════════════════════════════════════════════════════════
//  C.2 — ShopPanel
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("C.2 ShopPanel: panelId and title", "[phase_c][c2]") {
    NovaForge::ShopPanel panel;
    REQUIRE(panel.panelId()    == std::string("novaforge.shop"));
    REQUIRE(panel.panelTitle() == std::string("Shop"));
}

TEST_CASE("C.2 ShopPanel: default listings after bind", "[phase_c][c2]") {
    NovaForge::ShopPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::EconomyRules, "/tmp/shop.json");
    panel.bindDocument(doc.get());
    REQUIRE(!panel.listings().empty());
    REQUIRE(!panel.shopConfig().shopId.empty());
}

TEST_CASE("C.2 ShopPanel: addListing marks dirty and increments count", "[phase_c][c2]") {
    NovaForge::ShopPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::EconomyRules, "/tmp/shop.json");
    panel.bindDocument(doc.get());
    size_t before = panel.listings().size();
    panel.addListing({"magic_wand", "credits", 500.0f, 5, false, ""});
    REQUIRE(panel.listings().size() == before + 1);
    REQUIRE(panel.isDirty());
}

TEST_CASE("C.2 ShopPanel: removeListing undo", "[phase_c][c2]") {
    NovaForge::ShopPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::EconomyRules, "/tmp/shop.json");
    panel.bindDocument(doc.get());
    panel.addListing({"magic_wand", "credits", 500.0f, 5, false, ""});
    size_t after = panel.listings().size();
    panel.removeListing("magic_wand");
    panel.undoStack().undo();
    REQUIRE(panel.listings().size() == after);
}

TEST_CASE("C.2 ShopPanel: validation fails for empty shopId", "[phase_c][c2]") {
    NovaForge::ShopPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::EconomyRules, "/tmp/shop.json");
    panel.bindDocument(doc.get());
    panel.setShopConfig({"", "Bad Shop", "", true});
    auto msgs = panel.validate();
    bool hasError = false;
    for (const auto& m : msgs)
        if (m.severity == NovaForge::DocumentPanelValidationSeverity::Error) hasError = true;
    REQUIRE(hasError);
}

// ═══════════════════════════════════════════════════════════════════════════
//  C.2 — MissionRulesPanel
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("C.2 MissionRulesPanel: panelId and title", "[phase_c][c2]") {
    NovaForge::MissionRulesPanel panel;
    REQUIRE(panel.panelId()    == std::string("novaforge.mission_rules"));
    REQUIRE(panel.panelTitle() == std::string("Mission Rules"));
}

TEST_CASE("C.2 MissionRulesPanel: default missions after bind", "[phase_c][c2]") {
    NovaForge::MissionRulesPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::MissionDefinition, "/tmp/mis.json");
    panel.bindDocument(doc.get());
    REQUIRE(panel.missions().size() >= 2u);
}

TEST_CASE("C.2 MissionRulesPanel: addMission marks dirty", "[phase_c][c2]") {
    NovaForge::MissionRulesPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::MissionDefinition, "/tmp/mis.json");
    panel.bindDocument(doc.get());
    NovaForge::MissionDefinitionData m;
    m.missionId   = "side_quest_1";
    m.displayName = "Side Quest";
    panel.addMission(std::move(m));
    REQUIRE(panel.isDirty());
}

TEST_CASE("C.2 MissionRulesPanel: removeMission undo", "[phase_c][c2]") {
    NovaForge::MissionRulesPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::MissionDefinition, "/tmp/mis.json");
    panel.bindDocument(doc.get());
    panel.addMission({"removable", "Removable Quest", "", {}, {}, false});
    size_t after = panel.missions().size();
    panel.removeMission("removable");
    panel.undoStack().undo();
    REQUIRE(panel.missions().size() == after);
}

TEST_CASE("C.2 MissionRulesPanel: objectiveType names", "[phase_c][c2]") {
    REQUIRE(std::string(NovaForge::objectiveTypeName(NovaForge::ObjectiveType::Kill))    == "Kill");
    REQUIRE(std::string(NovaForge::objectiveTypeName(NovaForge::ObjectiveType::Collect)) == "Collect");
    REQUIRE(std::string(NovaForge::objectiveTypeName(NovaForge::ObjectiveType::Reach))   == "Reach");
}

// ═══════════════════════════════════════════════════════════════════════════
//  C.2 — ProgressionPanel
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("C.2 ProgressionPanel: panelId and title", "[phase_c][c2]") {
    NovaForge::ProgressionPanel panel;
    REQUIRE(panel.panelId()    == std::string("novaforge.progression"));
    REQUIRE(panel.panelTitle() == std::string("Progression"));
}

TEST_CASE("C.2 ProgressionPanel: default level thresholds after bind", "[phase_c][c2]") {
    NovaForge::ProgressionPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::ProgressionRules, "/tmp/prog.json");
    panel.bindDocument(doc.get());
    REQUIRE(panel.levelThresholds().size() >= 10u);
}

TEST_CASE("C.2 ProgressionPanel: default skills after bind", "[phase_c][c2]") {
    NovaForge::ProgressionPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::ProgressionRules, "/tmp/prog.json");
    panel.bindDocument(doc.get());
    REQUIRE(!panel.skillUnlocks().empty());
}

TEST_CASE("C.2 ProgressionPanel: setMaxLevel undo/redo", "[phase_c][c2]") {
    NovaForge::ProgressionPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::ProgressionRules, "/tmp/prog.json");
    panel.bindDocument(doc.get());
    int orig = panel.maxLevel();
    panel.setMaxLevel(100);
    REQUIRE(panel.maxLevel() == 100);
    panel.undoStack().undo();
    REQUIRE(panel.maxLevel() == orig);
    panel.undoStack().redo();
    REQUIRE(panel.maxLevel() == 100);
}

TEST_CASE("C.2 ProgressionPanel: validation fails for maxLevel=0", "[phase_c][c2]") {
    NovaForge::ProgressionPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::ProgressionRules, "/tmp/prog.json");
    panel.bindDocument(doc.get());
    panel.setMaxLevel(0);
    auto msgs = panel.validate();
    bool hasError = false;
    for (const auto& m : msgs)
        if (m.severity == NovaForge::DocumentPanelValidationSeverity::Error) hasError = true;
    REQUIRE(hasError);
}

TEST_CASE("C.2 ProgressionPanel: addLevelThreshold keeps sorted order", "[phase_c][c2]") {
    NovaForge::ProgressionPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::ProgressionRules, "/tmp/prog.json");
    panel.bindDocument(doc.get());
    panel.addLevelThreshold({99, 99999.0f, 2.0f});
    const auto& levels = panel.levelThresholds();
    REQUIRE(levels.back().level == 99);
}

// ═══════════════════════════════════════════════════════════════════════════
//  C.2 — CharacterRulesPanel
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("C.2 CharacterRulesPanel: panelId and title", "[phase_c][c2]") {
    NovaForge::CharacterRulesPanel panel;
    REQUIRE(panel.panelId()    == std::string("novaforge.character_rules"));
    REQUIRE(panel.panelTitle() == std::string("Character Rules"));
}

TEST_CASE("C.2 CharacterRulesPanel: default classes after bind", "[phase_c][c2]") {
    NovaForge::CharacterRulesPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::CharacterRules, "/tmp/chr.json");
    panel.bindDocument(doc.get());
    REQUIRE(panel.classPresets().size() >= 2u);
}

TEST_CASE("C.2 CharacterRulesPanel: addClass marks dirty", "[phase_c][c2]") {
    NovaForge::CharacterRulesPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::CharacterRules, "/tmp/chr.json");
    panel.bindDocument(doc.get());
    NovaForge::CharacterClassPreset mage;
    mage.classId     = "mage";
    mage.displayName = "Mage";
    panel.addClass(std::move(mage));
    REQUIRE(panel.isDirty());
}

TEST_CASE("C.2 CharacterRulesPanel: removeClass undo", "[phase_c][c2]") {
    NovaForge::CharacterRulesPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::CharacterRules, "/tmp/chr.json");
    panel.bindDocument(doc.get());
    panel.addClass({"temp_class", "Temp", "", {}, {}, true});
    size_t after = panel.classPresets().size();
    panel.removeClass("temp_class");
    panel.undoStack().undo();
    REQUIRE(panel.classPresets().size() == after);
}

TEST_CASE("C.2 CharacterRulesPanel: validation fails for empty classes list", "[phase_c][c2]") {
    NovaForge::CharacterRulesPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::CharacterRules, "/tmp/chr.json");
    panel.bindDocument(doc.get());
    // Remove all classes by unloading and not binding a doc with data
    // Access via empty panel (no bind)
    NovaForge::CharacterRulesPanel emptyPanel;
    auto msgs = emptyPanel.validate();
    // Without binding, has no document but validate() is data-only — empty classes = error
    bool hasError = false;
    for (const auto& m : msgs)
        if (m.severity == NovaForge::DocumentPanelValidationSeverity::Error) hasError = true;
    REQUIRE(hasError);
}

TEST_CASE("C.2 CharacterRulesPanel: setMaxCharactersPerAccount undo", "[phase_c][c2]") {
    NovaForge::CharacterRulesPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::CharacterRules, "/tmp/chr.json");
    panel.bindDocument(doc.get());
    int orig = panel.maxCharactersPerAccount();
    panel.setMaxCharactersPerAccount(10);
    panel.undoStack().undo();
    REQUIRE(panel.maxCharactersPerAccount() == orig);
}

// ═══════════════════════════════════════════════════════════════════════════
//  C.3 — DocumentPropertyGrid
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("C.3 DocumentPropertyGrid: starts empty", "[phase_c][c3]") {
    NovaForge::DocumentPropertyGrid grid;
    REQUIRE(grid.categories().empty());
    REQUIRE(grid.fieldCount() == 0u);
    REQUIRE(!grid.isDirty());
}

TEST_CASE("C.3 DocumentPropertyGrid: addField creates category and field", "[phase_c][c3]") {
    NovaForge::DocumentPropertyGrid grid;
    NovaForge::PropertyField f;
    f.key          = "name";
    f.displayName  = "Name";
    f.type         = NovaForge::PropertyFieldType::String;
    f.value        = "Default";
    f.defaultValue = "Default";
    grid.addField("General", std::move(f));
    REQUIRE(grid.categories().size() == 1u);
    REQUIRE(grid.fieldCount() == 1u);
}

TEST_CASE("C.3 DocumentPropertyGrid: setValue updates value and sets dirty", "[phase_c][c3]") {
    NovaForge::DocumentPropertyGrid grid;
    NovaForge::PropertyField f;
    f.key = "speed"; f.displayName = "Speed";
    f.type = NovaForge::PropertyFieldType::Float;
    f.value = "1.0"; f.defaultValue = "1.0";
    grid.addField("Stats", std::move(f));

    REQUIRE(grid.setValue("speed", "2.5"));
    REQUIRE(grid.getValue("speed") == "2.5");
    REQUIRE(grid.isDirty());
}

TEST_CASE("C.3 DocumentPropertyGrid: getValue returns fallback for missing key", "[phase_c][c3]") {
    NovaForge::DocumentPropertyGrid grid;
    REQUIRE(grid.getValue("missing", "fallback") == "fallback");
}

TEST_CASE("C.3 DocumentPropertyGrid: readOnly field cannot be set", "[phase_c][c3]") {
    NovaForge::DocumentPropertyGrid grid;
    NovaForge::PropertyField f;
    f.key      = "id";
    f.displayName = "ID";
    f.type     = NovaForge::PropertyFieldType::String;
    f.value    = "fixed";
    f.readOnly = true;
    grid.addField("Meta", std::move(f));

    REQUIRE(!grid.setValue("id", "changed"));
    REQUIRE(grid.getValue("id") == "fixed");
}

TEST_CASE("C.3 DocumentPropertyGrid: resetToDefaults clears dirty", "[phase_c][c3]") {
    NovaForge::DocumentPropertyGrid grid;
    NovaForge::PropertyField f;
    f.key = "x"; f.displayName = "X";
    f.type = NovaForge::PropertyFieldType::Float;
    f.value = "0.0"; f.defaultValue = "0.0";
    grid.addField("Transform", std::move(f));
    grid.setValue("x", "99.0");
    REQUIRE(grid.isDirty());
    grid.resetToDefaults();
    REQUIRE(!grid.isDirty());
    REQUIRE(grid.getValue("x") == "0.0");
}

TEST_CASE("C.3 DocumentPropertyGrid: validator blocks invalid values", "[phase_c][c3]") {
    NovaForge::DocumentPropertyGrid grid;
    NovaForge::PropertyField f;
    f.key = "level"; f.displayName = "Level";
    f.type = NovaForge::PropertyFieldType::Int;
    f.value = "1"; f.defaultValue = "1";
    f.validator = [](const std::string& v) {
        try { return std::stoi(v) >= 1; } catch (...) { return false; }
    };
    grid.addField("Meta", std::move(f));

    REQUIRE(!grid.setValue("level", "0"));    // blocked
    REQUIRE(!grid.setValue("level", "abc"));  // blocked
    REQUIRE(grid.setValue("level", "5"));     // ok
}

TEST_CASE("C.3 DocumentPropertyGrid: validateAll finds Enum mismatch", "[phase_c][c3]") {
    NovaForge::DocumentPropertyGrid grid;
    NovaForge::PropertyField f;
    f.key = "rarity"; f.displayName = "Rarity";
    f.type = NovaForge::PropertyFieldType::Enum;
    f.enumOptions = {"common", "rare", "epic"};
    f.value = "legendary";  // not in options
    f.defaultValue = "common";
    grid.addField("Loot", std::move(f));

    auto msgs = grid.validateAll();
    REQUIRE(!msgs.empty());
    bool hasWarning = false;
    for (const auto& m : msgs)
        if (m.severity == NovaForge::DocumentPanelValidationSeverity::Warning)
            hasWarning = true;
    REQUIRE(hasWarning);
}

TEST_CASE("C.3 DocumentPropertyGridBuilder: builds categories and fields", "[phase_c][c3]") {
    NovaForge::DocumentPropertyGrid grid;
    NovaForge::DocumentPropertyGridBuilder builder(grid);
    builder.category("Identity")
           .field("id",   "ID",   NovaForge::PropertyFieldType::String, "")
           .field("name", "Name", NovaForge::PropertyFieldType::String, "Unnamed")
           .category("Stats")
           .field("health", "Health", NovaForge::PropertyFieldType::Float, "100.0");

    REQUIRE(grid.categories().size() == 2u);
    REQUIRE(grid.fieldCount() == 3u);
    REQUIRE(grid.getValue("name") == "Unnamed");
    REQUIRE(grid.getValue("health") == "100.0");
}

TEST_CASE("C.3 DocumentPropertyGrid: toFlatMap returns all fields", "[phase_c][c3]") {
    NovaForge::DocumentPropertyGrid grid;
    NovaForge::DocumentPropertyGridBuilder builder(grid);
    builder.category("A")
           .field("x", "X", NovaForge::PropertyFieldType::Float, "1.0")
           .field("y", "Y", NovaForge::PropertyFieldType::Float, "2.0");
    auto flat = grid.toFlatMap();
    REQUIRE(flat.size() == 2u);
    REQUIRE(flat[0].first == "x");
    REQUIRE(flat[1].first == "y");
}

// ═══════════════════════════════════════════════════════════════════════════
//  C.4 — DocumentSavePipeline
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("C.4 DocumentSavePipeline: save with no document returns NoDocument", "[phase_c][c4]") {
    NovaForge::DocumentSavePipeline pipeline;
    NovaForge::EconomyPanel panel;
    // no document bound
    auto result = pipeline.save(panel);
    REQUIRE(result.status == NovaForge::SaveResultStatus::NoDocument);
}

TEST_CASE("C.4 DocumentSavePipeline: save clean document succeeds", "[phase_c][c4]") {
    auto tmpDir = makeTempDir("save_clean");
    std::string filePath = (tmpDir / "eco.json").string();

    NovaForge::DocumentSavePipeline pipeline;
    NovaForge::EconomyPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::EconomyRules, filePath);
    panel.bindDocument(doc.get());

    auto result = pipeline.save(panel);
    REQUIRE(result.ok());
    // File should now exist
    REQUIRE(fs::exists(filePath));
}

TEST_CASE("C.4 DocumentSavePipeline: save clears dirty state on success", "[phase_c][c4]") {
    auto tmpDir = makeTempDir("save_dirty");
    std::string filePath = (tmpDir / "eco2.json").string();

    NovaForge::DocumentSavePipeline pipeline;
    NovaForge::EconomyPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::EconomyRules, filePath);
    panel.bindDocument(doc.get());
    panel.setGlobalInflationRate(0.5f);
    REQUIRE(panel.isDirty());

    pipeline.save(panel);
    REQUIRE(!panel.boundDocument()->isDirty());
}

TEST_CASE("C.4 DocumentSavePipeline: save calls notification callback on success", "[phase_c][c4]") {
    auto tmpDir = makeTempDir("save_notify");
    std::string filePath = (tmpDir / "eco3.json").string();

    NovaForge::DocumentSavePipeline pipeline;
    std::vector<std::string> notifications;
    pipeline.setNotificationCallback([&](const std::string& sev, const std::string& msg) {
        notifications.push_back(sev + ":" + msg);
    });

    NovaForge::EconomyPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::EconomyRules, filePath);
    panel.bindDocument(doc.get());
    pipeline.save(panel);

    REQUIRE(!notifications.empty());
}

TEST_CASE("C.4 DocumentSavePipeline: save validation error fires notification", "[phase_c][c4]") {
    NovaForge::DocumentSavePipeline pipeline;
    std::vector<std::string> notifications;
    pipeline.setNotificationCallback([&](const std::string& sev, const std::string& msg) {
        notifications.push_back(sev + ":" + msg);
    });

    // InventoryRulesPanel with a slot having invalid maxStack
    NovaForge::InventoryRulesPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::ItemDefinition, "/tmp/inv_bad.json");
    panel.bindDocument(doc.get());
    panel.addSlot({"bad_slot", "any", 0, false}); // maxStack=0 triggers validation error

    auto result = pipeline.save(panel);
    REQUIRE(result.status == NovaForge::SaveResultStatus::ValidationFailed);
    // At least one error notification
    bool hasError = false;
    for (const auto& n : notifications) {
        if (n.find("error:") != std::string::npos) hasError = true;
    }
    REQUIRE(hasError);
}

TEST_CASE("C.4 DocumentSavePipeline: validateOnly does not write file", "[phase_c][c4]") {
    auto tmpDir = makeTempDir("validate_only");
    std::string filePath = (tmpDir / "validate_only.json").string();

    NovaForge::DocumentSavePipeline pipeline;
    NovaForge::EconomyPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::EconomyRules, filePath);
    panel.bindDocument(doc.get());

    pipeline.validateOnly(panel);
    // File must not be created by validateOnly
    REQUIRE(!fs::exists(filePath));
}

TEST_CASE("C.4 DocumentSavePipeline: revert returns true for bound document", "[phase_c][c4]") {
    NovaForge::DocumentSavePipeline pipeline;
    NovaForge::EconomyPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::EconomyRules, "/tmp/eco_rv.json");
    panel.bindDocument(doc.get());
    panel.setGlobalInflationRate(0.9f);
    bool ok = pipeline.revert(panel);
    REQUIRE(ok);
    REQUIRE(!panel.isDirty());
}

TEST_CASE("C.4 DocumentSavePipeline: revert returns false with no document", "[phase_c][c4]") {
    NovaForge::DocumentSavePipeline pipeline;
    NovaForge::EconomyPanel panel; // no document bound
    REQUIRE(!pipeline.revert(panel));
}

TEST_CASE("C.4 DocumentSavePipeline: save writes valid JSON file", "[phase_c][c4]") {
    auto tmpDir = makeTempDir("json_write");
    std::string filePath = (tmpDir / "out.json").string();

    NovaForge::DocumentSavePipeline pipeline;
    NovaForge::EconomyPanel panel;
    auto doc = makeDoc(NovaForge::NovaForgeDocumentType::EconomyRules, filePath);
    doc->setDisplayName("TestEconomy");
    panel.bindDocument(doc.get());
    auto result = pipeline.save(panel);
    REQUIRE(result.ok());

    // File should contain "EconomyRules" type token
    std::ifstream f(filePath);
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    REQUIRE(content.find("EconomyRules") != std::string::npos);
    REQUIRE(content.find("TestEconomy")  != std::string::npos);
}
