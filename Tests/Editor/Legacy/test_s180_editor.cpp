// S180 editor tests: ProcGenRuleEditorV1, WaveFunctionCollapseEditorV1, GrammarGeneratorEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ProcGenRuleEditorV1.h"
#include "NF/Editor/WaveFunctionCollapseEditorV1.h"
#include "NF/Editor/GrammarGeneratorEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── ProcGenRuleEditorV1 ──────────────────────────────────────────────────────

TEST_CASE("Pgrv1Rule validity", "[Editor][S180]") {
    Pgrv1Rule r;
    REQUIRE(!r.isValid());
    r.id = 1; r.name = "PlaceTree";
    REQUIRE(r.isValid());
}

TEST_CASE("ProcGenRuleEditorV1 addRule and ruleCount", "[Editor][S180]") {
    ProcGenRuleEditorV1 pgr;
    REQUIRE(pgr.ruleCount() == 0);
    Pgrv1Rule r; r.id = 1; r.name = "R1";
    REQUIRE(pgr.addRule(r));
    REQUIRE(pgr.ruleCount() == 1);
}

TEST_CASE("ProcGenRuleEditorV1 addRule invalid fails", "[Editor][S180]") {
    ProcGenRuleEditorV1 pgr;
    REQUIRE(!pgr.addRule(Pgrv1Rule{}));
}

TEST_CASE("ProcGenRuleEditorV1 addRule duplicate fails", "[Editor][S180]") {
    ProcGenRuleEditorV1 pgr;
    Pgrv1Rule r; r.id = 1; r.name = "A";
    pgr.addRule(r);
    REQUIRE(!pgr.addRule(r));
}

TEST_CASE("ProcGenRuleEditorV1 removeRule", "[Editor][S180]") {
    ProcGenRuleEditorV1 pgr;
    Pgrv1Rule r; r.id = 2; r.name = "B";
    pgr.addRule(r);
    REQUIRE(pgr.removeRule(2));
    REQUIRE(pgr.ruleCount() == 0);
    REQUIRE(!pgr.removeRule(2));
}

TEST_CASE("ProcGenRuleEditorV1 addConstraint and constraintCount", "[Editor][S180]") {
    ProcGenRuleEditorV1 pgr;
    Pgrv1Constraint c; c.id = 1; c.ruleId = 10; c.description = "No overlap";
    REQUIRE(pgr.addConstraint(c));
    REQUIRE(pgr.constraintCount() == 1);
}

TEST_CASE("ProcGenRuleEditorV1 addConstraint invalid fails", "[Editor][S180]") {
    ProcGenRuleEditorV1 pgr;
    REQUIRE(!pgr.addConstraint(Pgrv1Constraint{}));
}

TEST_CASE("ProcGenRuleEditorV1 removeConstraint", "[Editor][S180]") {
    ProcGenRuleEditorV1 pgr;
    Pgrv1Constraint c; c.id = 1; c.ruleId = 5; c.description = "Min dist";
    pgr.addConstraint(c);
    REQUIRE(pgr.removeConstraint(1));
    REQUIRE(pgr.constraintCount() == 0);
    REQUIRE(!pgr.removeConstraint(1));
}

TEST_CASE("ProcGenRuleEditorV1 enabledCount and lockedCount", "[Editor][S180]") {
    ProcGenRuleEditorV1 pgr;
    Pgrv1Rule r1; r1.id = 1; r1.name = "A"; r1.state = Pgrv1RuleState::Enabled;
    Pgrv1Rule r2; r2.id = 2; r2.name = "B"; r2.state = Pgrv1RuleState::Locked;
    pgr.addRule(r1); pgr.addRule(r2);
    REQUIRE(pgr.enabledCount() == 1);
    REQUIRE(pgr.lockedCount()  == 1);
}

TEST_CASE("ProcGenRuleEditorV1 countByRuleType", "[Editor][S180]") {
    ProcGenRuleEditorV1 pgr;
    Pgrv1Rule r1; r1.id = 1; r1.name = "A"; r1.type = Pgrv1RuleType::Placement;
    Pgrv1Rule r2; r2.id = 2; r2.name = "B"; r2.type = Pgrv1RuleType::Filter;
    pgr.addRule(r1); pgr.addRule(r2);
    REQUIRE(pgr.countByRuleType(Pgrv1RuleType::Placement) == 1);
    REQUIRE(pgr.countByRuleType(Pgrv1RuleType::Filter)    == 1);
}

TEST_CASE("ProcGenRuleEditorV1 findRule returns ptr", "[Editor][S180]") {
    ProcGenRuleEditorV1 pgr;
    Pgrv1Rule r; r.id = 5; r.name = "Decoration";
    pgr.addRule(r);
    REQUIRE(pgr.findRule(5) != nullptr);
    REQUIRE(pgr.findRule(5)->name == "Decoration");
    REQUIRE(pgr.findRule(99) == nullptr);
}

TEST_CASE("pgrv1RuleTypeName covers all values", "[Editor][S180]") {
    REQUIRE(std::string(pgrv1RuleTypeName(Pgrv1RuleType::Replacement)) == "Replacement");
    REQUIRE(std::string(pgrv1RuleTypeName(Pgrv1RuleType::Decoration))  == "Decoration");
    REQUIRE(std::string(pgrv1RuleTypeName(Pgrv1RuleType::Constraint))  == "Constraint");
    REQUIRE(std::string(pgrv1RuleTypeName(Pgrv1RuleType::Post))        == "Post");
}

TEST_CASE("pgrv1RuleStateName covers all values", "[Editor][S180]") {
    REQUIRE(std::string(pgrv1RuleStateName(Pgrv1RuleState::Disabled)) == "Disabled");
    REQUIRE(std::string(pgrv1RuleStateName(Pgrv1RuleState::Testing))  == "Testing");
    REQUIRE(std::string(pgrv1RuleStateName(Pgrv1RuleState::Locked))   == "Locked");
}

TEST_CASE("Pgrv1Rule state helpers", "[Editor][S180]") {
    Pgrv1Rule r; r.id = 1; r.name = "X";
    r.state = Pgrv1RuleState::Enabled;
    REQUIRE(r.isEnabled());
    r.state = Pgrv1RuleState::Locked;
    REQUIRE(r.isLocked());
}

TEST_CASE("ProcGenRuleEditorV1 onChange callback", "[Editor][S180]") {
    ProcGenRuleEditorV1 pgr;
    uint64_t notified = 0;
    pgr.setOnChange([&](uint64_t id) { notified = id; });
    Pgrv1Rule r; r.id = 8; r.name = "PostProcess";
    pgr.addRule(r);
    REQUIRE(notified == 8);
}

// ── WaveFunctionCollapseEditorV1 ─────────────────────────────────────────────

TEST_CASE("Wfcv1Tile validity", "[Editor][S180]") {
    Wfcv1Tile t;
    REQUIRE(!t.isValid());
    t.id = 1; t.name = "Grass";
    REQUIRE(t.isValid());
}

TEST_CASE("WaveFunctionCollapseEditorV1 addTile and tileCount", "[Editor][S180]") {
    WaveFunctionCollapseEditorV1 wfc;
    REQUIRE(wfc.tileCount() == 0);
    Wfcv1Tile t; t.id = 1; t.name = "Dirt";
    REQUIRE(wfc.addTile(t));
    REQUIRE(wfc.tileCount() == 1);
}

TEST_CASE("WaveFunctionCollapseEditorV1 addTile invalid fails", "[Editor][S180]") {
    WaveFunctionCollapseEditorV1 wfc;
    REQUIRE(!wfc.addTile(Wfcv1Tile{}));
}

TEST_CASE("WaveFunctionCollapseEditorV1 addTile duplicate fails", "[Editor][S180]") {
    WaveFunctionCollapseEditorV1 wfc;
    Wfcv1Tile t; t.id = 1; t.name = "A";
    wfc.addTile(t);
    REQUIRE(!wfc.addTile(t));
}

TEST_CASE("WaveFunctionCollapseEditorV1 removeTile", "[Editor][S180]") {
    WaveFunctionCollapseEditorV1 wfc;
    Wfcv1Tile t; t.id = 2; t.name = "B";
    wfc.addTile(t);
    REQUIRE(wfc.removeTile(2));
    REQUIRE(wfc.tileCount() == 0);
    REQUIRE(!wfc.removeTile(2));
}

TEST_CASE("WaveFunctionCollapseEditorV1 addPattern and patternCount", "[Editor][S180]") {
    WaveFunctionCollapseEditorV1 wfc;
    Wfcv1Pattern p; p.id = 1; p.tileId = 10; p.data = "adj:wall";
    REQUIRE(wfc.addPattern(p));
    REQUIRE(wfc.patternCount() == 1);
}

TEST_CASE("WaveFunctionCollapseEditorV1 addPattern invalid fails", "[Editor][S180]") {
    WaveFunctionCollapseEditorV1 wfc;
    REQUIRE(!wfc.addPattern(Wfcv1Pattern{}));
}

TEST_CASE("WaveFunctionCollapseEditorV1 collapsedCount and solvedCount", "[Editor][S180]") {
    WaveFunctionCollapseEditorV1 wfc;
    Wfcv1Tile t1; t1.id = 1; t1.name = "A"; t1.state = Wfcv1CollapseState::Collapsed;
    Wfcv1Tile t2; t2.id = 2; t2.name = "B"; t2.state = Wfcv1CollapseState::Solved;
    wfc.addTile(t1); wfc.addTile(t2);
    REQUIRE(wfc.collapsedCount() == 1);
    REQUIRE(wfc.solvedCount()    == 1);
}

TEST_CASE("WaveFunctionCollapseEditorV1 countByTileCategory", "[Editor][S180]") {
    WaveFunctionCollapseEditorV1 wfc;
    Wfcv1Tile t1; t1.id = 1; t1.name = "A"; t1.category = Wfcv1TileCategory::Wall;
    Wfcv1Tile t2; t2.id = 2; t2.name = "B"; t2.category = Wfcv1TileCategory::Roof;
    wfc.addTile(t1); wfc.addTile(t2);
    REQUIRE(wfc.countByTileCategory(Wfcv1TileCategory::Wall) == 1);
    REQUIRE(wfc.countByTileCategory(Wfcv1TileCategory::Roof) == 1);
}

TEST_CASE("WaveFunctionCollapseEditorV1 findTile returns ptr", "[Editor][S180]") {
    WaveFunctionCollapseEditorV1 wfc;
    Wfcv1Tile t; t.id = 6; t.name = "Special";
    wfc.addTile(t);
    REQUIRE(wfc.findTile(6) != nullptr);
    REQUIRE(wfc.findTile(6)->name == "Special");
    REQUIRE(wfc.findTile(99) == nullptr);
}

TEST_CASE("wfcv1TileCategoryName covers all values", "[Editor][S180]") {
    REQUIRE(std::string(wfcv1TileCategoryName(Wfcv1TileCategory::Ground))  == "Ground");
    REQUIRE(std::string(wfcv1TileCategoryName(Wfcv1TileCategory::Detail))  == "Detail");
    REQUIRE(std::string(wfcv1TileCategoryName(Wfcv1TileCategory::Special)) == "Special");
    REQUIRE(std::string(wfcv1TileCategoryName(Wfcv1TileCategory::Empty))   == "Empty");
}

TEST_CASE("wfcv1CollapseStateName covers all values", "[Editor][S180]") {
    REQUIRE(std::string(wfcv1CollapseStateName(Wfcv1CollapseState::Superposition)) == "Superposition");
    REQUIRE(std::string(wfcv1CollapseStateName(Wfcv1CollapseState::Contradiction)) == "Contradiction");
    REQUIRE(std::string(wfcv1CollapseStateName(Wfcv1CollapseState::Solved))        == "Solved");
}

TEST_CASE("Wfcv1Tile state helpers", "[Editor][S180]") {
    Wfcv1Tile t; t.id = 1; t.name = "X";
    t.state = Wfcv1CollapseState::Collapsed;
    REQUIRE(t.isCollapsed());
    t.state = Wfcv1CollapseState::Solved;
    REQUIRE(t.isSolved());
}

TEST_CASE("WaveFunctionCollapseEditorV1 onChange callback", "[Editor][S180]") {
    WaveFunctionCollapseEditorV1 wfc;
    uint64_t notified = 0;
    wfc.setOnChange([&](uint64_t id) { notified = id; });
    Wfcv1Tile t; t.id = 3; t.name = "C";
    wfc.addTile(t);
    REQUIRE(notified == 3);
}

// ── GrammarGeneratorEditorV1 ─────────────────────────────────────────────────

TEST_CASE("Ggev1Symbol validity", "[Editor][S180]") {
    Ggev1Symbol s;
    REQUIRE(!s.isValid());
    s.id = 1; s.name = "Sentence";
    REQUIRE(s.isValid());
}

TEST_CASE("GrammarGeneratorEditorV1 addSymbol and symbolCount", "[Editor][S180]") {
    GrammarGeneratorEditorV1 gge;
    REQUIRE(gge.symbolCount() == 0);
    Ggev1Symbol s; s.id = 1; s.name = "NP";
    REQUIRE(gge.addSymbol(s));
    REQUIRE(gge.symbolCount() == 1);
}

TEST_CASE("GrammarGeneratorEditorV1 addSymbol invalid fails", "[Editor][S180]") {
    GrammarGeneratorEditorV1 gge;
    REQUIRE(!gge.addSymbol(Ggev1Symbol{}));
}

TEST_CASE("GrammarGeneratorEditorV1 addSymbol duplicate fails", "[Editor][S180]") {
    GrammarGeneratorEditorV1 gge;
    Ggev1Symbol s; s.id = 1; s.name = "A";
    gge.addSymbol(s);
    REQUIRE(!gge.addSymbol(s));
}

TEST_CASE("GrammarGeneratorEditorV1 removeSymbol", "[Editor][S180]") {
    GrammarGeneratorEditorV1 gge;
    Ggev1Symbol s; s.id = 2; s.name = "B";
    gge.addSymbol(s);
    REQUIRE(gge.removeSymbol(2));
    REQUIRE(gge.symbolCount() == 0);
    REQUIRE(!gge.removeSymbol(2));
}

TEST_CASE("GrammarGeneratorEditorV1 addProduction and productionCount", "[Editor][S180]") {
    GrammarGeneratorEditorV1 gge;
    Ggev1Production p; p.id = 1; p.symbolId = 10; p.rule = "NP -> Det N";
    REQUIRE(gge.addProduction(p));
    REQUIRE(gge.productionCount() == 1);
}

TEST_CASE("GrammarGeneratorEditorV1 addProduction invalid fails", "[Editor][S180]") {
    GrammarGeneratorEditorV1 gge;
    REQUIRE(!gge.addProduction(Ggev1Production{}));
}

TEST_CASE("GrammarGeneratorEditorV1 removeProduction", "[Editor][S180]") {
    GrammarGeneratorEditorV1 gge;
    Ggev1Production p; p.id = 1; p.symbolId = 5; p.rule = "VP -> V NP";
    gge.addProduction(p);
    REQUIRE(gge.removeProduction(1));
    REQUIRE(gge.productionCount() == 0);
    REQUIRE(!gge.removeProduction(1));
}

TEST_CASE("GrammarGeneratorEditorV1 activeCount (productions)", "[Editor][S180]") {
    GrammarGeneratorEditorV1 gge;
    Ggev1Production p1; p1.id = 1; p1.symbolId = 1; p1.rule = "A->B"; p1.state = Ggev1ProductionState::Active;
    Ggev1Production p2; p2.id = 2; p2.symbolId = 1; p2.rule = "C->D"; p2.state = Ggev1ProductionState::Disabled;
    gge.addProduction(p1); gge.addProduction(p2);
    REQUIRE(gge.activeCount() == 1);
}

TEST_CASE("GrammarGeneratorEditorV1 terminalCount", "[Editor][S180]") {
    GrammarGeneratorEditorV1 gge;
    Ggev1Symbol s1; s1.id = 1; s1.name = "dog";  s1.type = Ggev1SymbolType::Terminal;
    Ggev1Symbol s2; s2.id = 2; s2.name = "NP";   s2.type = Ggev1SymbolType::NonTerminal;
    gge.addSymbol(s1); gge.addSymbol(s2);
    REQUIRE(gge.terminalCount() == 1);
}

TEST_CASE("GrammarGeneratorEditorV1 countBySymbolType", "[Editor][S180]") {
    GrammarGeneratorEditorV1 gge;
    Ggev1Symbol s1; s1.id = 1; s1.name = "S";  s1.type = Ggev1SymbolType::Start;
    Ggev1Symbol s2; s2.id = 2; s2.name = "E";  s2.type = Ggev1SymbolType::End;
    gge.addSymbol(s1); gge.addSymbol(s2);
    REQUIRE(gge.countBySymbolType(Ggev1SymbolType::Start) == 1);
    REQUIRE(gge.countBySymbolType(Ggev1SymbolType::End)   == 1);
}

TEST_CASE("GrammarGeneratorEditorV1 findSymbol returns ptr", "[Editor][S180]") {
    GrammarGeneratorEditorV1 gge;
    Ggev1Symbol s; s.id = 7; s.name = "VP";
    gge.addSymbol(s);
    REQUIRE(gge.findSymbol(7) != nullptr);
    REQUIRE(gge.findSymbol(7)->name == "VP");
    REQUIRE(gge.findSymbol(99) == nullptr);
}

TEST_CASE("ggev1SymbolTypeName covers all values", "[Editor][S180]") {
    REQUIRE(std::string(ggev1SymbolTypeName(Ggev1SymbolType::Terminal))    == "Terminal");
    REQUIRE(std::string(ggev1SymbolTypeName(Ggev1SymbolType::NonTerminal)) == "NonTerminal");
    REQUIRE(std::string(ggev1SymbolTypeName(Ggev1SymbolType::Start))       == "Start");
    REQUIRE(std::string(ggev1SymbolTypeName(Ggev1SymbolType::Special))     == "Special");
}

TEST_CASE("ggev1ProductionStateName covers all values", "[Editor][S180]") {
    REQUIRE(std::string(ggev1ProductionStateName(Ggev1ProductionState::Draft))        == "Draft");
    REQUIRE(std::string(ggev1ProductionStateName(Ggev1ProductionState::Active))       == "Active");
    REQUIRE(std::string(ggev1ProductionStateName(Ggev1ProductionState::Experimental)) == "Experimental");
}

TEST_CASE("Ggev1Symbol and Ggev1Production helpers", "[Editor][S180]") {
    Ggev1Symbol s; s.id = 1; s.name = "word";
    s.type = Ggev1SymbolType::Terminal;
    REQUIRE(s.isTerminal());

    Ggev1Production p; p.id = 1; p.symbolId = 1; p.rule = "x";
    p.state = Ggev1ProductionState::Active;
    REQUIRE(p.isActive());
}

TEST_CASE("GrammarGeneratorEditorV1 onChange callback", "[Editor][S180]") {
    GrammarGeneratorEditorV1 gge;
    uint64_t notified = 0;
    gge.setOnChange([&](uint64_t id) { notified = id; });
    Ggev1Symbol s; s.id = 11; s.name = "Det";
    gge.addSymbol(s);
    REQUIRE(notified == 11);
}
