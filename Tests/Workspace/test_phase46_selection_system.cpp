// Tests/Workspace/test_phase46_selection_system.cpp
// Phase 46 — SelectionSystem
//
// Tests for:
//   1. SelectionContextType  — enum values and name helper
//   2. SelectionRecord       — isValid, equality
//   3. SelectionSet          — add/remove/contains/find/clear/count/items/countByContext/version
//   4. SelectionHistory      — push/back/forward/canBack/canForward/current/clear/depth
//   5. SelectionSystem       — createSet/removeSet/findSet/setActiveContext/setActiveSet/select/deselect/clearActive/isSelected/activeCount
//   6. Integration           — multi-context selection, history navigation, branching

#include <catch2/catch_test_macros.hpp>
#include "NF/Engine/Engine.h"
#include "NF/Workspace/SelectionSystem.h"
#include <string>

using namespace NF;

// ── Helper ────────────────────────────────────────────────────────

static SelectionRecord makeRecord(EntityID id,
                                   const char* label,
                                   SelectionContextType ctx = SelectionContextType::Scene) {
    SelectionRecord r;
    r.id      = id;
    r.label   = label;
    r.context = ctx;
    return r;
}

// ─────────────────────────────────────────────────────────────────
// 1. SelectionContextType
// ─────────────────────────────────────────────────────────────────

TEST_CASE("SelectionContextType – all values have names", "[phase46][SelectionContextType]") {
    CHECK(std::string(selectionContextTypeName(SelectionContextType::None))    == "None");
    CHECK(std::string(selectionContextTypeName(SelectionContextType::Scene))   == "Scene");
    CHECK(std::string(selectionContextTypeName(SelectionContextType::Asset))   == "Asset");
    CHECK(std::string(selectionContextTypeName(SelectionContextType::UI))      == "UI");
    CHECK(std::string(selectionContextTypeName(SelectionContextType::Console)) == "Console");
    CHECK(std::string(selectionContextTypeName(SelectionContextType::Code))    == "Code");
}

// ─────────────────────────────────────────────────────────────────
// 2. SelectionRecord
// ─────────────────────────────────────────────────────────────────

TEST_CASE("SelectionRecord – default is invalid", "[phase46][SelectionRecord]") {
    SelectionRecord r;
    CHECK_FALSE(r.isValid());
    CHECK(r.id == INVALID_ENTITY);
    CHECK(r.context == SelectionContextType::None);
}

TEST_CASE("SelectionRecord – isValid when id is set", "[phase46][SelectionRecord]") {
    auto r = makeRecord(42, "node_42");
    CHECK(r.isValid());
    CHECK(r.id == 42);
    CHECK(r.label == "node_42");
}

TEST_CASE("SelectionRecord – equality by id", "[phase46][SelectionRecord]") {
    auto a = makeRecord(10, "a");
    auto b = makeRecord(10, "b_different_label");
    auto c = makeRecord(11, "c");
    CHECK(a == b);
    CHECK(a != c);
}

// ─────────────────────────────────────────────────────────────────
// 3. SelectionSet
// ─────────────────────────────────────────────────────────────────

TEST_CASE("SelectionSet – default is empty", "[phase46][SelectionSet]") {
    SelectionSet s("test");
    CHECK(s.name() == "test");
    CHECK(s.count() == 0);
    CHECK(s.isEmpty());
    CHECK(s.version() == 0);
}

TEST_CASE("SelectionSet – add valid record", "[phase46][SelectionSet]") {
    SelectionSet s("s");
    CHECK(s.add(makeRecord(1, "r1")));
    CHECK(s.count() == 1);
    CHECK_FALSE(s.isEmpty());
    CHECK(s.version() == 1);
}

TEST_CASE("SelectionSet – add invalid record rejected", "[phase46][SelectionSet]") {
    SelectionSet s("s");
    SelectionRecord bad;
    CHECK_FALSE(s.add(bad));
    CHECK(s.count() == 0);
}

TEST_CASE("SelectionSet – add duplicate rejected", "[phase46][SelectionSet]") {
    SelectionSet s("s");
    CHECK(s.add(makeRecord(5, "r5")));
    CHECK_FALSE(s.add(makeRecord(5, "r5_dup")));
    CHECK(s.count() == 1);
}

TEST_CASE("SelectionSet – remove existing", "[phase46][SelectionSet]") {
    SelectionSet s("s");
    s.add(makeRecord(1, "r1"));
    s.add(makeRecord(2, "r2"));
    CHECK(s.remove(1));
    CHECK(s.count() == 1);
    CHECK_FALSE(s.contains(1));
    CHECK(s.contains(2));
}

TEST_CASE("SelectionSet – remove unknown returns false", "[phase46][SelectionSet]") {
    SelectionSet s("s");
    s.add(makeRecord(1, "r1"));
    CHECK_FALSE(s.remove(99));
    CHECK(s.count() == 1);
}

TEST_CASE("SelectionSet – contains and find", "[phase46][SelectionSet]") {
    SelectionSet s("s");
    s.add(makeRecord(3, "r3", SelectionContextType::Asset));
    CHECK(s.contains(3));
    CHECK_FALSE(s.contains(99));
    const auto* p = s.find(3);
    REQUIRE(p != nullptr);
    CHECK(p->label == "r3");
    CHECK(p->context == SelectionContextType::Asset);
    CHECK(s.find(99) == nullptr);
}

TEST_CASE("SelectionSet – clear empties and bumps version", "[phase46][SelectionSet]") {
    SelectionSet s("s");
    s.add(makeRecord(1, "r1"));
    s.add(makeRecord(2, "r2"));
    uint32_t v = s.version();
    s.clear();
    CHECK(s.isEmpty());
    CHECK(s.version() == v + 1);
}

TEST_CASE("SelectionSet – items returns all records", "[phase46][SelectionSet]") {
    SelectionSet s("s");
    s.add(makeRecord(10, "a"));
    s.add(makeRecord(20, "b"));
    const auto& items = s.items();
    CHECK(items.size() == 2);
}

TEST_CASE("SelectionSet – countByContext", "[phase46][SelectionSet]") {
    SelectionSet s("s");
    s.add(makeRecord(1, "a", SelectionContextType::Scene));
    s.add(makeRecord(2, "b", SelectionContextType::Scene));
    s.add(makeRecord(3, "c", SelectionContextType::Asset));
    CHECK(s.countByContext(SelectionContextType::Scene) == 2);
    CHECK(s.countByContext(SelectionContextType::Asset) == 1);
    CHECK(s.countByContext(SelectionContextType::UI)    == 0);
}

// ─────────────────────────────────────────────────────────────────
// 4. SelectionHistory
// ─────────────────────────────────────────────────────────────────

TEST_CASE("SelectionHistory – default has no history", "[phase46][SelectionHistory]") {
    SelectionHistory h;
    CHECK_FALSE(h.hasHistory());
    CHECK(h.depth() == 0);
    CHECK_FALSE(h.canBack());
    CHECK_FALSE(h.canForward());
    CHECK(h.current() == nullptr);
}

TEST_CASE("SelectionHistory – push one entry", "[phase46][SelectionHistory]") {
    SelectionHistory h;
    SelectionSet s("snap");
    s.add(makeRecord(1, "r1"));
    h.push(s);
    CHECK(h.hasHistory());
    CHECK(h.depth() == 1);
    CHECK_FALSE(h.canBack());
    CHECK_FALSE(h.canForward());
    REQUIRE(h.current() != nullptr);
    CHECK(h.current()->count() == 1);
}

TEST_CASE("SelectionHistory – back and forward", "[phase46][SelectionHistory]") {
    SelectionHistory h;
    SelectionSet s1("s1");
    s1.add(makeRecord(1, "r1"));
    SelectionSet s2("s2");
    s2.add(makeRecord(2, "r2"));

    h.push(s1);
    h.push(s2);

    CHECK(h.depth() == 2);
    CHECK(h.canBack());
    CHECK_FALSE(h.canForward());

    CHECK(h.back());
    CHECK_FALSE(h.canBack());
    CHECK(h.canForward());
    REQUIRE(h.current() != nullptr);
    CHECK(h.current()->name() == "s1");

    CHECK(h.forward());
    CHECK_FALSE(h.canForward());
    REQUIRE(h.current() != nullptr);
    CHECK(h.current()->name() == "s2");
}

TEST_CASE("SelectionHistory – back at beginning returns false", "[phase46][SelectionHistory]") {
    SelectionHistory h;
    h.push(SelectionSet("only"));
    CHECK_FALSE(h.back());
}

TEST_CASE("SelectionHistory – forward at end returns false", "[phase46][SelectionHistory]") {
    SelectionHistory h;
    h.push(SelectionSet("only"));
    CHECK_FALSE(h.forward());
}

TEST_CASE("SelectionHistory – push truncates forward history", "[phase46][SelectionHistory]") {
    SelectionHistory h;
    h.push(SelectionSet("a"));
    h.push(SelectionSet("b"));
    h.push(SelectionSet("c"));
    h.back();   // cursor at b
    h.back();   // cursor at a
    // push new entry — forward (b, c) should be dropped
    h.push(SelectionSet("x"));
    CHECK_FALSE(h.canForward());
    CHECK(h.depth() == 2);  // a + x
    REQUIRE(h.current() != nullptr);
    CHECK(h.current()->name() == "x");
}

TEST_CASE("SelectionHistory – clear empties everything", "[phase46][SelectionHistory]") {
    SelectionHistory h;
    h.push(SelectionSet("a"));
    h.push(SelectionSet("b"));
    h.clear();
    CHECK_FALSE(h.hasHistory());
    CHECK(h.current() == nullptr);
}

TEST_CASE("SelectionHistory – MAX_HISTORY caps depth", "[phase46][SelectionHistory]") {
    SelectionHistory h;
    for (size_t i = 0; i < SelectionHistory::MAX_HISTORY + 5; ++i) {
        h.push(SelectionSet("snap_" + std::to_string(i)));
    }
    CHECK(h.depth() == SelectionHistory::MAX_HISTORY);
}

// ─────────────────────────────────────────────────────────────────
// 5. SelectionSystem
// ─────────────────────────────────────────────────────────────────

TEST_CASE("SelectionSystem – default empty", "[phase46][SelectionSystem]") {
    SelectionSystem sys;
    CHECK(sys.setCount() == 0);
    CHECK(sys.activeSetName().empty());
    CHECK(sys.activeContext() == SelectionContextType::None);
    CHECK(sys.activeSet() == nullptr);
}

TEST_CASE("SelectionSystem – createSet", "[phase46][SelectionSystem]") {
    SelectionSystem sys;
    CHECK(sys.createSet("scene"));
    CHECK(sys.setCount() == 1);
    CHECK(sys.findSet("scene") != nullptr);
}

TEST_CASE("SelectionSystem – createSet duplicate rejected", "[phase46][SelectionSystem]") {
    SelectionSystem sys;
    CHECK(sys.createSet("scene"));
    CHECK_FALSE(sys.createSet("scene"));
    CHECK(sys.setCount() == 1);
}

TEST_CASE("SelectionSystem – createSet empty name rejected", "[phase46][SelectionSystem]") {
    SelectionSystem sys;
    CHECK_FALSE(sys.createSet(""));
    CHECK(sys.setCount() == 0);
}

TEST_CASE("SelectionSystem – removeSet", "[phase46][SelectionSystem]") {
    SelectionSystem sys;
    sys.createSet("scene");
    CHECK(sys.removeSet("scene"));
    CHECK(sys.setCount() == 0);
    CHECK(sys.findSet("scene") == nullptr);
}

TEST_CASE("SelectionSystem – removeSet unknown returns false", "[phase46][SelectionSystem]") {
    SelectionSystem sys;
    CHECK_FALSE(sys.removeSet("none"));
}

TEST_CASE("SelectionSystem – removeSet clears activeSet name", "[phase46][SelectionSystem]") {
    SelectionSystem sys;
    sys.createSet("scene");
    sys.setActiveSet("scene");
    CHECK(sys.activeSetName() == "scene");
    sys.removeSet("scene");
    CHECK(sys.activeSetName().empty());
}

TEST_CASE("SelectionSystem – setActiveSet", "[phase46][SelectionSystem]") {
    SelectionSystem sys;
    sys.createSet("assets");
    CHECK(sys.setActiveSet("assets"));
    CHECK(sys.activeSetName() == "assets");
    CHECK(sys.activeSet() != nullptr);
}

TEST_CASE("SelectionSystem – setActiveSet unknown fails", "[phase46][SelectionSystem]") {
    SelectionSystem sys;
    CHECK_FALSE(sys.setActiveSet("missing"));
}

TEST_CASE("SelectionSystem – setActiveContext", "[phase46][SelectionSystem]") {
    SelectionSystem sys;
    sys.setActiveContext(SelectionContextType::Asset);
    CHECK(sys.activeContext() == SelectionContextType::Asset);
}

TEST_CASE("SelectionSystem – select adds to active set", "[phase46][SelectionSystem]") {
    SelectionSystem sys;
    sys.createSet("main");
    sys.setActiveSet("main");
    sys.setActiveContext(SelectionContextType::Scene);

    SelectionRecord r;
    r.id    = 7;
    r.label = "obj7";
    CHECK(sys.select(r));
    CHECK(sys.isSelected(7));
    CHECK(sys.activeCount() == 1);
}

TEST_CASE("SelectionSystem – select with no active set returns false", "[phase46][SelectionSystem]") {
    SelectionSystem sys;
    SelectionRecord r;
    r.id = 1;
    CHECK_FALSE(sys.select(r));
}

TEST_CASE("SelectionSystem – deselect removes from active set", "[phase46][SelectionSystem]") {
    SelectionSystem sys;
    sys.createSet("main");
    sys.setActiveSet("main");
    sys.setActiveContext(SelectionContextType::Scene);

    SelectionRecord r; r.id = 5; r.label = "obj5";
    sys.select(r);
    CHECK(sys.deselect(5));
    CHECK_FALSE(sys.isSelected(5));
    CHECK(sys.activeCount() == 0);
}

TEST_CASE("SelectionSystem – clearActive empties active set", "[phase46][SelectionSystem]") {
    SelectionSystem sys;
    sys.createSet("main");
    sys.setActiveSet("main");
    sys.setActiveContext(SelectionContextType::Scene);

    SelectionRecord r1; r1.id = 1; r1.label = "a";
    SelectionRecord r2; r2.id = 2; r2.label = "b";
    sys.select(r1);
    sys.select(r2);
    CHECK(sys.activeCount() == 2);

    sys.clearActive();
    CHECK(sys.activeCount() == 0);
}

TEST_CASE("SelectionSystem – MAX_SETS enforced", "[phase46][SelectionSystem]") {
    SelectionSystem sys;
    for (size_t i = 0; i < SelectionSystem::MAX_SETS; ++i) {
        CHECK(sys.createSet("set_" + std::to_string(i)));
    }
    CHECK_FALSE(sys.createSet("overflow"));
    CHECK(sys.setCount() == SelectionSystem::MAX_SETS);
}

TEST_CASE("SelectionSystem – clearAll clears sets and history", "[phase46][SelectionSystem]") {
    SelectionSystem sys;
    sys.createSet("main");
    sys.setActiveSet("main");
    sys.setActiveContext(SelectionContextType::Scene);

    SelectionRecord r; r.id = 1; r.label = "x";
    sys.select(r);
    sys.clearAll();

    CHECK(sys.activeCount() == 0);
    CHECK_FALSE(sys.history().hasHistory());
}

// ─────────────────────────────────────────────────────────────────
// 6. Integration
// ─────────────────────────────────────────────────────────────────

TEST_CASE("Integration – multi-context selection", "[phase46][integration]") {
    SelectionSystem sys;
    sys.createSet("scene");
    sys.createSet("assets");

    sys.setActiveSet("scene");
    sys.setActiveContext(SelectionContextType::Scene);
    SelectionRecord sr; sr.id = 100; sr.label = "enemy";
    sys.select(sr);

    sys.setActiveSet("assets");
    sys.setActiveContext(SelectionContextType::Asset);
    SelectionRecord ar; ar.id = 200; ar.label = "mesh.fbx";
    sys.select(ar);

    sys.setActiveSet("scene");
    CHECK(sys.isSelected(100));
    CHECK_FALSE(sys.isSelected(200));

    sys.setActiveSet("assets");
    CHECK(sys.isSelected(200));

    // Verify context was stamped on record
    const auto* sceneSet = sys.findSet("scene");
    REQUIRE(sceneSet != nullptr);
    const auto* rec = sceneSet->find(100);
    REQUIRE(rec != nullptr);
    CHECK(rec->context == SelectionContextType::Scene);
}

TEST_CASE("Integration – history navigation after select/deselect", "[phase46][integration]") {
    SelectionSystem sys;
    sys.createSet("main");
    sys.setActiveSet("main");
    sys.setActiveContext(SelectionContextType::UI);

    SelectionRecord r1; r1.id = 1; r1.label = "widget_a";
    SelectionRecord r2; r2.id = 2; r2.label = "widget_b";

    sys.select(r1);   // history depth 1
    sys.select(r2);   // history depth 2
    sys.deselect(1);  // history depth 3

    CHECK(sys.history().depth() == 3);
    CHECK(sys.history().canBack());

    sys.history().back();
    // current snapshot shows 2 items selected (before deselect)
    const auto* snap = sys.history().current();
    REQUIRE(snap != nullptr);
    CHECK(snap->count() == 2);
}

TEST_CASE("Integration – countByContext across sets", "[phase46][integration]") {
    SelectionSystem sys;
    sys.createSet("mixed");
    sys.setActiveSet("mixed");
    sys.setActiveContext(SelectionContextType::Scene);

    SelectionRecord s1; s1.id = 1; s1.label = "s1";
    SelectionRecord s2; s2.id = 2; s2.label = "s2";
    sys.select(s1);
    sys.select(s2);

    sys.setActiveContext(SelectionContextType::Console);
    SelectionRecord c1; c1.id = 3; c1.label = "log_entry";
    sys.select(c1);

    const auto* set = sys.findSet("mixed");
    REQUIRE(set != nullptr);
    CHECK(set->countByContext(SelectionContextType::Scene)   == 2);
    CHECK(set->countByContext(SelectionContextType::Console) == 1);
}
