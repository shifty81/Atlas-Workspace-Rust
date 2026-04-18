// Tests/Workspace/test_phase21_focus.cpp
// Phase 21 — Workspace Focus and Context Tracking
//
// Tests for:
//   1. FocusLayer — enum name helpers
//   2. FocusTarget — named focus target with layer classification
//   3. FocusRecord — timestamped gain/lose focus event
//   4. FocusStack — layered focus management with chronological history
//   5. FocusManager — workspace-scoped focus lifecycle with observers
//   6. Integration — full focus pipeline

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceFocus.h"
#include <string>
#include <vector>

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — Enums
// ═════════════════════════════════════════════════════════════════

TEST_CASE("focusLayerName returns correct strings", "[Phase21][FocusLayer]") {
    CHECK(std::string(focusLayerName(FocusLayer::Background)) == "Background");
    CHECK(std::string(focusLayerName(FocusLayer::Base))       == "Base");
    CHECK(std::string(focusLayerName(FocusLayer::Overlay))    == "Overlay");
    CHECK(std::string(focusLayerName(FocusLayer::Modal))      == "Modal");
    CHECK(std::string(focusLayerName(FocusLayer::Popup))      == "Popup");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — FocusTarget
// ═════════════════════════════════════════════════════════════════

TEST_CASE("FocusTarget default is invalid", "[Phase21][FocusTarget]") {
    FocusTarget t;
    CHECK_FALSE(t.isValid());
    CHECK(t.id.empty());
    CHECK(t.layer == FocusLayer::Base);
}

TEST_CASE("FocusTarget valid construction with all fields", "[Phase21][FocusTarget]") {
    FocusTarget t{"viewport_1", "Main Viewport", "panel_view", "tool_scene", FocusLayer::Overlay};
    CHECK(t.isValid());
    CHECK(t.id == "viewport_1");
    CHECK(t.displayName == "Main Viewport");
    CHECK(t.panelId == "panel_view");
    CHECK(t.toolId == "tool_scene");
    CHECK(t.layer == FocusLayer::Overlay);
}

TEST_CASE("FocusTarget equality by id", "[Phase21][FocusTarget]") {
    FocusTarget a{"id_1", "A", "", "", FocusLayer::Base};
    FocusTarget b{"id_1", "B", "", "", FocusLayer::Modal};
    FocusTarget c{"id_2", "C", "", "", FocusLayer::Base};
    CHECK(a == b);
    CHECK(a != c);
}

TEST_CASE("FocusTarget layer field", "[Phase21][FocusTarget]") {
    FocusTarget t{"t1", "T", "", "", FocusLayer::Modal};
    CHECK(t.layer == FocusLayer::Modal);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — FocusRecord
// ═════════════════════════════════════════════════════════════════

TEST_CASE("FocusRecord default is invalid", "[Phase21][FocusRecord]") {
    FocusRecord r;
    CHECK_FALSE(r.isValid());
    CHECK(r.timestamp == 0);
    CHECK(r.gained == true);
}

TEST_CASE("FocusRecord valid gained record", "[Phase21][FocusRecord]") {
    FocusTarget t{"t1", "T", "", "", FocusLayer::Base};
    FocusRecord r{t, 100, true};
    CHECK(r.isValid());
    CHECK(r.gained == true);
    CHECK(r.timestamp == 100);
    CHECK(r.target.id == "t1");
}

TEST_CASE("FocusRecord valid lost record", "[Phase21][FocusRecord]") {
    FocusTarget t{"t1", "T", "", "", FocusLayer::Base};
    FocusRecord r{t, 200, false};
    CHECK(r.isValid());
    CHECK(r.gained == false);
    CHECK(r.timestamp == 200);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — FocusStack
// ═════════════════════════════════════════════════════════════════

TEST_CASE("FocusStack empty state", "[Phase21][FocusStack]") {
    FocusStack stack;
    CHECK(stack.depth() == 0);
    CHECK(stack.current() == nullptr);
    CHECK_FALSE(stack.pop());
    CHECK(stack.history().empty());
}

TEST_CASE("FocusStack push and current", "[Phase21][FocusStack]") {
    FocusStack stack;
    FocusTarget t{"t1", "T1", "", "", FocusLayer::Base};
    CHECK(stack.push(t));
    CHECK(stack.depth() == 1);
    auto* cur = stack.current();
    REQUIRE(cur != nullptr);
    CHECK(cur->id == "t1");
}

TEST_CASE("FocusStack push invalid rejected", "[Phase21][FocusStack]") {
    FocusStack stack;
    FocusTarget invalid; // no id
    CHECK_FALSE(stack.push(invalid));
    CHECK(stack.depth() == 0);
}

TEST_CASE("FocusStack pop", "[Phase21][FocusStack]") {
    FocusStack stack;
    FocusTarget t{"t1", "T1", "", "", FocusLayer::Base};
    stack.push(t);
    CHECK(stack.pop());
    CHECK(stack.depth() == 0);
    CHECK(stack.current() == nullptr);
    CHECK_FALSE(stack.pop()); // empty
}

TEST_CASE("FocusStack depth", "[Phase21][FocusStack]") {
    FocusStack stack;
    FocusTarget t1{"t1", "", "", "", FocusLayer::Base};
    FocusTarget t2{"t2", "", "", "", FocusLayer::Overlay};
    CHECK(stack.depth() == 0);
    stack.push(t1);
    CHECK(stack.depth() == 1);
    stack.push(t2);
    CHECK(stack.depth() == 2);
}

TEST_CASE("FocusStack hasTarget", "[Phase21][FocusStack]") {
    FocusStack stack;
    FocusTarget t{"t1", "", "", "", FocusLayer::Base};
    CHECK_FALSE(stack.hasTarget("t1"));
    stack.push(t);
    CHECK(stack.hasTarget("t1"));
    CHECK_FALSE(stack.hasTarget("t2"));
}

TEST_CASE("FocusStack push multiple layers", "[Phase21][FocusStack]") {
    FocusStack stack;
    FocusTarget base{"base", "", "", "", FocusLayer::Base};
    FocusTarget overlay{"overlay", "", "", "", FocusLayer::Overlay};
    FocusTarget modal{"modal", "", "", "", FocusLayer::Modal};
    stack.push(base);
    stack.push(overlay);
    stack.push(modal);
    CHECK(stack.depth() == 3);
    CHECK(stack.current()->id == "modal");
}

TEST_CASE("FocusStack pop restores previous", "[Phase21][FocusStack]") {
    FocusStack stack;
    FocusTarget t1{"t1", "", "", "", FocusLayer::Base};
    FocusTarget t2{"t2", "", "", "", FocusLayer::Overlay};
    stack.push(t1);
    stack.push(t2);
    stack.pop();
    CHECK(stack.current()->id == "t1");
}

TEST_CASE("FocusStack clear", "[Phase21][FocusStack]") {
    FocusStack stack;
    FocusTarget t{"t1", "", "", "", FocusLayer::Base};
    stack.push(t);
    stack.clear();
    CHECK(stack.depth() == 0);
    CHECK(stack.current() == nullptr);
}

TEST_CASE("FocusStack history grows on push", "[Phase21][FocusStack]") {
    FocusStack stack;
    FocusTarget t{"t1", "", "", "", FocusLayer::Base};
    stack.push(t);
    // First push: 1 gained event (no prior top to lose)
    CHECK(stack.history().size() >= 1);
    CHECK(stack.history().back().gained == true);
}

TEST_CASE("FocusStack history grows on pop", "[Phase21][FocusStack]") {
    FocusStack stack;
    FocusTarget t{"t1", "", "", "", FocusLayer::Base};
    stack.push(t);
    size_t beforePop = stack.history().size();
    stack.pop();
    CHECK(stack.history().size() > beforePop);
}

TEST_CASE("FocusStack clearHistory", "[Phase21][FocusStack]") {
    FocusStack stack;
    FocusTarget t{"t1", "", "", "", FocusLayer::Base};
    stack.push(t);
    stack.clearHistory();
    CHECK(stack.history().empty());
    CHECK(stack.depth() == 1); // stack intact
}

TEST_CASE("FocusStack MAX_DEPTH enforcement", "[Phase21][FocusStack]") {
    FocusStack stack;
    for (int i = 0; i < FocusStack::MAX_DEPTH; ++i) {
        FocusTarget t{std::string("t") + std::to_string(i), "", "", "", FocusLayer::Base};
        stack.push(t);
    }
    CHECK(stack.depth() == FocusStack::MAX_DEPTH);
    FocusTarget extra{"extra", "", "", "", FocusLayer::Base};
    CHECK_FALSE(stack.push(extra));
    CHECK(stack.depth() == FocusStack::MAX_DEPTH);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — FocusManager
// ═════════════════════════════════════════════════════════════════

TEST_CASE("FocusManager empty state", "[Phase21][FocusManager]") {
    FocusManager mgr;
    CHECK(mgr.allTargets().empty());
    CHECK(mgr.currentFocus() == nullptr);
    CHECK_FALSE(mgr.isRegistered("t1"));
}

TEST_CASE("FocusManager registerTarget", "[Phase21][FocusManager]") {
    FocusManager mgr;
    FocusTarget t{"t1", "T1", "", "", FocusLayer::Base};
    CHECK(mgr.registerTarget(t));
    CHECK(mgr.isRegistered("t1"));
    CHECK(mgr.allTargets().size() == 1);
}

TEST_CASE("FocusManager duplicate registration rejected", "[Phase21][FocusManager]") {
    FocusManager mgr;
    FocusTarget t{"t1", "T1", "", "", FocusLayer::Base};
    mgr.registerTarget(t);
    CHECK_FALSE(mgr.registerTarget(t));
    CHECK(mgr.allTargets().size() == 1);
}

TEST_CASE("FocusManager invalid registration rejected", "[Phase21][FocusManager]") {
    FocusManager mgr;
    FocusTarget invalid; // no id
    CHECK_FALSE(mgr.registerTarget(invalid));
    CHECK(mgr.allTargets().empty());
}

TEST_CASE("FocusManager unregisterTarget", "[Phase21][FocusManager]") {
    FocusManager mgr;
    FocusTarget t{"t1", "T1", "", "", FocusLayer::Base};
    mgr.registerTarget(t);
    CHECK(mgr.unregisterTarget("t1"));
    CHECK_FALSE(mgr.isRegistered("t1"));
    CHECK_FALSE(mgr.unregisterTarget("nonexistent"));
}

TEST_CASE("FocusManager isRegistered", "[Phase21][FocusManager]") {
    FocusManager mgr;
    CHECK_FALSE(mgr.isRegistered("t1"));
    mgr.registerTarget({"t1", "", "", "", FocusLayer::Base});
    CHECK(mgr.isRegistered("t1"));
}

TEST_CASE("FocusManager findTarget", "[Phase21][FocusManager]") {
    FocusManager mgr;
    FocusTarget t{"t1", "T1", "panel", "tool", FocusLayer::Modal};
    mgr.registerTarget(t);
    auto* found = mgr.findTarget("t1");
    REQUIRE(found != nullptr);
    CHECK(found->displayName == "T1");
    CHECK(found->layer == FocusLayer::Modal);
    CHECK(mgr.findTarget("nonexistent") == nullptr);
}

TEST_CASE("FocusManager requestFocus", "[Phase21][FocusManager]") {
    FocusManager mgr;
    mgr.registerTarget({"t1", "", "", "", FocusLayer::Base});
    CHECK(mgr.requestFocus("t1"));
    auto* cur = mgr.currentFocus();
    REQUIRE(cur != nullptr);
    CHECK(cur->id == "t1");
}

TEST_CASE("FocusManager requestFocus unknown fails", "[Phase21][FocusManager]") {
    FocusManager mgr;
    CHECK_FALSE(mgr.requestFocus("nonexistent"));
    CHECK(mgr.currentFocus() == nullptr);
}

TEST_CASE("FocusManager releaseFocus", "[Phase21][FocusManager]") {
    FocusManager mgr;
    mgr.registerTarget({"t1", "", "", "", FocusLayer::Base});
    mgr.requestFocus("t1");
    CHECK(mgr.releaseFocus("t1"));
    CHECK(mgr.currentFocus() == nullptr);
}

TEST_CASE("FocusManager releaseFocus non-current fails", "[Phase21][FocusManager]") {
    FocusManager mgr;
    mgr.registerTarget({"t1", "", "", "", FocusLayer::Base});
    mgr.registerTarget({"t2", "", "", "", FocusLayer::Overlay});
    mgr.requestFocus("t1");
    CHECK_FALSE(mgr.releaseFocus("t2")); // t2 is not current
    CHECK(mgr.currentFocus()->id == "t1");
}

TEST_CASE("FocusManager currentFocus", "[Phase21][FocusManager]") {
    FocusManager mgr;
    CHECK(mgr.currentFocus() == nullptr);
    mgr.registerTarget({"t1", "", "", "", FocusLayer::Base});
    mgr.requestFocus("t1");
    CHECK(mgr.currentFocus()->id == "t1");
}

TEST_CASE("FocusManager canFocus", "[Phase21][FocusManager]") {
    FocusManager mgr;
    mgr.registerTarget({"t1", "", "", "", FocusLayer::Base});
    CHECK(mgr.canFocus("t1"));
    mgr.requestFocus("t1");
    CHECK_FALSE(mgr.canFocus("t1")); // already on top
    CHECK_FALSE(mgr.canFocus("unregistered"));
}

TEST_CASE("FocusManager allTargets", "[Phase21][FocusManager]") {
    FocusManager mgr;
    mgr.registerTarget({"t1", "", "", "", FocusLayer::Base});
    mgr.registerTarget({"t2", "", "", "", FocusLayer::Overlay});
    auto targets = mgr.allTargets();
    CHECK(targets.size() == 2);
}

TEST_CASE("FocusManager clear", "[Phase21][FocusManager]") {
    FocusManager mgr;
    mgr.registerTarget({"t1", "", "", "", FocusLayer::Base});
    mgr.requestFocus("t1");
    mgr.clear();
    CHECK(mgr.allTargets().empty());
    CHECK(mgr.currentFocus() == nullptr);
}

TEST_CASE("FocusManager observer on requestFocus", "[Phase21][FocusManager]") {
    FocusManager mgr;
    mgr.registerTarget({"t1", "", "", "", FocusLayer::Base});
    std::string lastId;
    bool lastGained = false;
    mgr.addObserver([&](const FocusTarget& t, bool gained) {
        lastId = t.id;
        lastGained = gained;
    });
    mgr.requestFocus("t1");
    CHECK(lastId == "t1");
    CHECK(lastGained == true);
}

TEST_CASE("FocusManager observer on releaseFocus", "[Phase21][FocusManager]") {
    FocusManager mgr;
    mgr.registerTarget({"t1", "", "", "", FocusLayer::Base});
    mgr.requestFocus("t1");
    std::string lastId;
    bool lastGained = true;
    mgr.addObserver([&](const FocusTarget& t, bool gained) {
        lastId = t.id;
        lastGained = gained;
    });
    mgr.releaseFocus("t1");
    CHECK(lastId == "t1");
    CHECK(lastGained == false);
}

TEST_CASE("FocusManager removeObserver stops notifications", "[Phase21][FocusManager]") {
    FocusManager mgr;
    mgr.registerTarget({"t1", "", "", "", FocusLayer::Base});
    mgr.registerTarget({"t2", "", "", "", FocusLayer::Overlay});
    int count = 0;
    auto id = mgr.addObserver([&](const FocusTarget&, bool) { ++count; });
    mgr.requestFocus("t1");
    int countAfter = count;
    mgr.removeObserver(id);
    mgr.requestFocus("t2");
    CHECK(count == countAfter);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 6 — Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: multi-target focus sequence", "[Phase21][Integration]") {
    FocusManager mgr;
    mgr.registerTarget({"base",    "", "", "", FocusLayer::Base});
    mgr.registerTarget({"overlay", "", "", "", FocusLayer::Overlay});
    mgr.registerTarget({"modal",   "", "", "", FocusLayer::Modal});
    mgr.requestFocus("base");
    mgr.requestFocus("overlay");
    mgr.requestFocus("modal");
    CHECK(mgr.currentFocus()->id == "modal");
    CHECK(mgr.stack().depth() == 3);
}

TEST_CASE("Integration: requestFocus + releaseFocus lifecycle", "[Phase21][Integration]") {
    FocusManager mgr;
    mgr.registerTarget({"t1", "", "", "", FocusLayer::Base});
    mgr.registerTarget({"t2", "", "", "", FocusLayer::Overlay});
    mgr.requestFocus("t1");
    mgr.requestFocus("t2");
    CHECK(mgr.currentFocus()->id == "t2");
    mgr.releaseFocus("t2");
    CHECK(mgr.currentFocus()->id == "t1");
    mgr.releaseFocus("t1");
    CHECK(mgr.currentFocus() == nullptr);
}

TEST_CASE("Integration: observer tracks focus chain", "[Phase21][Integration]") {
    FocusManager mgr;
    mgr.registerTarget({"t1", "", "", "", FocusLayer::Base});
    mgr.registerTarget({"t2", "", "", "", FocusLayer::Overlay});
    std::vector<std::pair<std::string,bool>> events;
    mgr.addObserver([&](const FocusTarget& t, bool gained) {
        events.push_back({t.id, gained});
    });
    mgr.requestFocus("t1");
    mgr.requestFocus("t2");
    mgr.releaseFocus("t2");
    REQUIRE(events.size() == 3);
    CHECK(events[0] == std::make_pair(std::string("t1"), true));
    CHECK(events[1] == std::make_pair(std::string("t2"), true));
    CHECK(events[2] == std::make_pair(std::string("t2"), false));
}

TEST_CASE("Integration: modal layer focus isolation", "[Phase21][Integration]") {
    FocusManager mgr;
    FocusTarget base{"base", "Base", "", "", FocusLayer::Base};
    FocusTarget modal{"modal", "Modal", "", "", FocusLayer::Modal};
    mgr.registerTarget(base);
    mgr.registerTarget(modal);
    mgr.requestFocus("base");
    mgr.requestFocus("modal");
    // Modal is now on top
    CHECK(mgr.currentFocus()->layer == FocusLayer::Modal);
    // canFocus on base while modal is current
    CHECK(mgr.canFocus("base")); // base is registered and not on top
    CHECK_FALSE(mgr.canFocus("modal")); // modal IS on top
}

TEST_CASE("Integration: allTargets after register/unregister", "[Phase21][Integration]") {
    FocusManager mgr;
    mgr.registerTarget({"t1", "", "", "", FocusLayer::Base});
    mgr.registerTarget({"t2", "", "", "", FocusLayer::Overlay});
    mgr.registerTarget({"t3", "", "", "", FocusLayer::Modal});
    CHECK(mgr.allTargets().size() == 3);
    mgr.unregisterTarget("t2");
    CHECK(mgr.allTargets().size() == 2);
    CHECK_FALSE(mgr.isRegistered("t2"));
}

TEST_CASE("Integration: canFocus false after requestFocus", "[Phase21][Integration]") {
    FocusManager mgr;
    mgr.registerTarget({"t1", "", "", "", FocusLayer::Base});
    CHECK(mgr.canFocus("t1"));
    mgr.requestFocus("t1");
    CHECK_FALSE(mgr.canFocus("t1"));
}

TEST_CASE("Integration: history accumulates across manager operations", "[Phase21][Integration]") {
    FocusManager mgr;
    mgr.registerTarget({"t1", "", "", "", FocusLayer::Base});
    mgr.registerTarget({"t2", "", "", "", FocusLayer::Overlay});
    mgr.requestFocus("t1");
    mgr.requestFocus("t2");
    mgr.releaseFocus("t2");
    CHECK(mgr.stack().history().size() >= 3);
}

TEST_CASE("Integration: clearObservers stops all notifications", "[Phase21][Integration]") {
    FocusManager mgr;
    mgr.registerTarget({"t1", "", "", "", FocusLayer::Base});
    int count = 0;
    mgr.addObserver([&](const FocusTarget&, bool) { ++count; });
    mgr.addObserver([&](const FocusTarget&, bool) { ++count; });
    mgr.requestFocus("t1");
    int countAfter = count;
    mgr.clearObservers();
    mgr.releaseFocus("t1");
    CHECK(count == countAfter);
}
