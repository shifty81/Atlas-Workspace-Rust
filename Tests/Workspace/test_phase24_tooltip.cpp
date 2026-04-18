// Tests/Workspace/test_phase24_tooltip.cpp
// Phase 24 — Workspace Tooltip and Help System
//
// Tests for:
//   1. TooltipTrigger  — enum name helpers
//   2. TooltipPosition — enum name helpers
//   3. TooltipEntry    — tooltip content and metadata; isValid, equality
//   4. TooltipState    — current visibility record; isValid
//   5. TooltipManager  — registry with show/hide lifecycle and observers
//   6. Integration     — full tooltip pipeline

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceTooltip.h"
#include <string>
#include <vector>

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — TooltipTrigger enum
// ═════════════════════════════════════════════════════════════════

TEST_CASE("tooltipTriggerName returns correct strings", "[Phase24][TooltipTrigger]") {
    CHECK(std::string(tooltipTriggerName(TooltipTrigger::Hover))  == "Hover");
    CHECK(std::string(tooltipTriggerName(TooltipTrigger::Focus))  == "Focus");
    CHECK(std::string(tooltipTriggerName(TooltipTrigger::Manual)) == "Manual");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — TooltipPosition enum
// ═════════════════════════════════════════════════════════════════

TEST_CASE("tooltipPositionName returns correct strings", "[Phase24][TooltipPosition]") {
    CHECK(std::string(tooltipPositionName(TooltipPosition::Auto))   == "Auto");
    CHECK(std::string(tooltipPositionName(TooltipPosition::Top))    == "Top");
    CHECK(std::string(tooltipPositionName(TooltipPosition::Bottom)) == "Bottom");
    CHECK(std::string(tooltipPositionName(TooltipPosition::Left))   == "Left");
    CHECK(std::string(tooltipPositionName(TooltipPosition::Right))  == "Right");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — TooltipEntry
// ═════════════════════════════════════════════════════════════════

TEST_CASE("TooltipEntry default is invalid", "[Phase24][TooltipEntry]") {
    TooltipEntry e;
    CHECK_FALSE(e.isValid());
    CHECK(e.id.empty());
    CHECK(e.body.empty());
    CHECK(e.trigger  == TooltipTrigger::Hover);
    CHECK(e.position == TooltipPosition::Auto);
    CHECK(e.enabled);
}

TEST_CASE("TooltipEntry valid construction with all fields", "[Phase24][TooltipEntry]") {
    TooltipEntry e{"tip_save", "Save", "Saves the current project", "btn_save",
                   TooltipTrigger::Hover, TooltipPosition::Bottom, true};
    CHECK(e.isValid());
    CHECK(e.id    == "tip_save");
    CHECK(e.title == "Save");
    CHECK(e.body  == "Saves the current project");
    CHECK(e.targetElementId == "btn_save");
    CHECK(e.trigger  == TooltipTrigger::Hover);
    CHECK(e.position == TooltipPosition::Bottom);
    CHECK(e.enabled);
}

TEST_CASE("TooltipEntry invalid without id", "[Phase24][TooltipEntry]") {
    TooltipEntry e{"", "Title", "Body", "", TooltipTrigger::Manual, TooltipPosition::Auto, true};
    CHECK_FALSE(e.isValid());
}

TEST_CASE("TooltipEntry invalid without body", "[Phase24][TooltipEntry]") {
    TooltipEntry e{"t1", "Title", "", "", TooltipTrigger::Hover, TooltipPosition::Auto, true};
    CHECK_FALSE(e.isValid());
}

TEST_CASE("TooltipEntry equality by id", "[Phase24][TooltipEntry]") {
    TooltipEntry a{"id_1", "A", "body_a", "", TooltipTrigger::Hover, TooltipPosition::Auto, true};
    TooltipEntry b{"id_1", "B", "body_b", "", TooltipTrigger::Focus, TooltipPosition::Top, false};
    TooltipEntry c{"id_2", "A", "body_a", "", TooltipTrigger::Hover, TooltipPosition::Auto, true};
    CHECK(a == b);
    CHECK(a != c);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — TooltipState
// ═════════════════════════════════════════════════════════════════

TEST_CASE("TooltipState default is invalid", "[Phase24][TooltipState]") {
    TooltipState s;
    CHECK_FALSE(s.isValid());
    CHECK(s.entryId.empty());
    CHECK_FALSE(s.visible);
    CHECK(s.showTimestamp == 0);
}

TEST_CASE("TooltipState valid with entryId", "[Phase24][TooltipState]") {
    TooltipState s{"tip_1", true, 42};
    CHECK(s.isValid());
    CHECK(s.entryId == "tip_1");
    CHECK(s.visible);
    CHECK(s.showTimestamp == 42);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — TooltipManager
// ═════════════════════════════════════════════════════════════════

TEST_CASE("TooltipManager empty state", "[Phase24][TooltipManager]") {
    TooltipManager m;
    CHECK(m.tooltipCount() == 0);
    CHECK(m.allTooltipIds().empty());
    CHECK_FALSE(m.currentVisible().visible);
}

TEST_CASE("TooltipManager registerTooltip", "[Phase24][TooltipManager]") {
    TooltipManager m;
    TooltipEntry e{"t1", "T", "Body text", "", TooltipTrigger::Hover, TooltipPosition::Auto, true};
    CHECK(m.registerTooltip(e));
    CHECK(m.isRegistered("t1"));
    CHECK(m.tooltipCount() == 1);
}

TEST_CASE("TooltipManager duplicate id rejected", "[Phase24][TooltipManager]") {
    TooltipManager m;
    TooltipEntry e{"t1", "T", "B", "", TooltipTrigger::Hover, TooltipPosition::Auto, true};
    CHECK(m.registerTooltip(e));
    CHECK_FALSE(m.registerTooltip(e));
    CHECK(m.tooltipCount() == 1);
}

TEST_CASE("TooltipManager invalid entry rejected", "[Phase24][TooltipManager]") {
    TooltipManager m;
    TooltipEntry e;  // invalid
    CHECK_FALSE(m.registerTooltip(e));
}

TEST_CASE("TooltipManager unregisterTooltip", "[Phase24][TooltipManager]") {
    TooltipManager m;
    TooltipEntry e{"t1", "T", "B", "", TooltipTrigger::Hover, TooltipPosition::Auto, true};
    m.registerTooltip(e);
    CHECK(m.unregisterTooltip("t1"));
    CHECK_FALSE(m.isRegistered("t1"));
}

TEST_CASE("TooltipManager unregister unknown fails", "[Phase24][TooltipManager]") {
    TooltipManager m;
    CHECK_FALSE(m.unregisterTooltip("nope"));
}

TEST_CASE("TooltipManager findTooltip", "[Phase24][TooltipManager]") {
    TooltipManager m;
    TooltipEntry e{"t1", "Title", "Body", "btn", TooltipTrigger::Focus, TooltipPosition::Top, true};
    m.registerTooltip(e);
    const TooltipEntry* found = m.findTooltip("t1");
    CHECK(found != nullptr);
    CHECK(found->title == "Title");
    CHECK(found->trigger == TooltipTrigger::Focus);
}

TEST_CASE("TooltipManager show makes tooltip visible", "[Phase24][TooltipManager]") {
    TooltipManager m;
    TooltipEntry e{"t1", "T", "B", "", TooltipTrigger::Hover, TooltipPosition::Auto, true};
    m.registerTooltip(e);
    CHECK(m.show("t1"));
    CHECK(m.isVisible("t1"));
    CHECK(m.currentVisible().entryId == "t1");
    CHECK(m.currentVisible().visible);
}

TEST_CASE("TooltipManager show unknown fails", "[Phase24][TooltipManager]") {
    TooltipManager m;
    CHECK_FALSE(m.show("nope"));
}

TEST_CASE("TooltipManager show disabled tooltip fails", "[Phase24][TooltipManager]") {
    TooltipManager m;
    TooltipEntry e{"t1", "T", "B", "", TooltipTrigger::Hover, TooltipPosition::Auto, false};
    m.registerTooltip(e);
    CHECK_FALSE(m.show("t1"));
    CHECK_FALSE(m.isVisible("t1"));
}

TEST_CASE("TooltipManager hide makes tooltip invisible", "[Phase24][TooltipManager]") {
    TooltipManager m;
    TooltipEntry e{"t1", "T", "B", "", TooltipTrigger::Hover, TooltipPosition::Auto, true};
    m.registerTooltip(e);
    m.show("t1");
    CHECK(m.hide("t1"));
    CHECK_FALSE(m.isVisible("t1"));
}

TEST_CASE("TooltipManager hide non-visible tooltip fails", "[Phase24][TooltipManager]") {
    TooltipManager m;
    TooltipEntry e{"t1", "T", "B", "", TooltipTrigger::Hover, TooltipPosition::Auto, true};
    m.registerTooltip(e);
    CHECK_FALSE(m.hide("t1"));
}

TEST_CASE("TooltipManager showing second tooltip hides first", "[Phase24][TooltipManager]") {
    TooltipManager m;
    m.registerTooltip({"t1", "T1", "B1", "", TooltipTrigger::Hover, TooltipPosition::Auto, true});
    m.registerTooltip({"t2", "T2", "B2", "", TooltipTrigger::Hover, TooltipPosition::Auto, true});
    m.show("t1");
    CHECK(m.isVisible("t1"));
    m.show("t2");
    CHECK_FALSE(m.isVisible("t1"));
    CHECK(m.isVisible("t2"));
}

TEST_CASE("TooltipManager hideAll clears visibility", "[Phase24][TooltipManager]") {
    TooltipManager m;
    m.registerTooltip({"t1", "T", "B", "", TooltipTrigger::Hover, TooltipPosition::Auto, true});
    m.show("t1");
    m.hideAll();
    CHECK_FALSE(m.isVisible("t1"));
    CHECK_FALSE(m.currentVisible().visible);
}

TEST_CASE("TooltipManager enableTooltip / disableTooltip", "[Phase24][TooltipManager]") {
    TooltipManager m;
    m.registerTooltip({"t1", "T", "B", "", TooltipTrigger::Hover, TooltipPosition::Auto, true});
    m.disableTooltip("t1");
    CHECK_FALSE(m.show("t1"));
    m.enableTooltip("t1");
    CHECK(m.show("t1"));
}

TEST_CASE("TooltipManager disableTooltip hides if visible", "[Phase24][TooltipManager]") {
    TooltipManager m;
    m.registerTooltip({"t1", "T", "B", "", TooltipTrigger::Hover, TooltipPosition::Auto, true});
    m.show("t1");
    CHECK(m.isVisible("t1"));
    m.disableTooltip("t1");
    CHECK_FALSE(m.isVisible("t1"));
}

TEST_CASE("TooltipManager unregister hides visible tooltip", "[Phase24][TooltipManager]") {
    TooltipManager m;
    m.registerTooltip({"t1", "T", "B", "", TooltipTrigger::Hover, TooltipPosition::Auto, true});
    m.show("t1");
    m.unregisterTooltip("t1");
    CHECK_FALSE(m.isVisible("t1"));
    CHECK_FALSE(m.isRegistered("t1"));
}

TEST_CASE("TooltipManager observer fires on show", "[Phase24][TooltipManager]") {
    TooltipManager m;
    m.registerTooltip({"t1", "T", "B", "", TooltipTrigger::Hover, TooltipPosition::Auto, true});
    std::string lastId; bool lastVisible = false;
    m.addObserver([&](const TooltipEntry& e, bool v) { lastId = e.id; lastVisible = v; });
    m.show("t1");
    CHECK(lastId == "t1");
    CHECK(lastVisible);
}

TEST_CASE("TooltipManager observer fires on hide", "[Phase24][TooltipManager]") {
    TooltipManager m;
    m.registerTooltip({"t1", "T", "B", "", TooltipTrigger::Hover, TooltipPosition::Auto, true});
    std::string lastId; bool lastVisible = true;
    m.addObserver([&](const TooltipEntry& e, bool v) { lastId = e.id; lastVisible = v; });
    m.show("t1");
    m.hide("t1");
    CHECK(lastId == "t1");
    CHECK_FALSE(lastVisible);
}

TEST_CASE("TooltipManager removeObserver stops notifications", "[Phase24][TooltipManager]") {
    TooltipManager m;
    m.registerTooltip({"t1", "T", "B", "", TooltipTrigger::Hover, TooltipPosition::Auto, true});
    int calls = 0;
    uint32_t id = m.addObserver([&](const TooltipEntry&, bool) { ++calls; });
    m.removeObserver(id);
    m.show("t1");
    CHECK(calls == 0);
}

TEST_CASE("TooltipManager allTooltipIds", "[Phase24][TooltipManager]") {
    TooltipManager m;
    m.registerTooltip({"t1", "T", "B", "", TooltipTrigger::Hover, TooltipPosition::Auto, true});
    m.registerTooltip({"t2", "T", "B", "", TooltipTrigger::Hover, TooltipPosition::Auto, true});
    m.registerTooltip({"t3", "T", "B", "", TooltipTrigger::Hover, TooltipPosition::Auto, true});
    CHECK(m.allTooltipIds().size() == 3);
}

TEST_CASE("TooltipManager clear removes all entries", "[Phase24][TooltipManager]") {
    TooltipManager m;
    m.registerTooltip({"t1", "T", "B", "", TooltipTrigger::Hover, TooltipPosition::Auto, true});
    m.show("t1");
    m.clear();
    CHECK(m.tooltipCount() == 0);
    CHECK_FALSE(m.currentVisible().visible);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 6 — Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Full tooltip pipeline: register → show → observer → hide", "[Phase24][Integration]") {
    TooltipManager m;
    m.registerTooltip({"tip_undo", "Undo", "Undoes the last action (Ctrl+Z)",
                        "btn_undo", TooltipTrigger::Hover, TooltipPosition::Bottom, true});

    std::vector<std::pair<std::string,bool>> events;
    m.addObserver([&](const TooltipEntry& e, bool v) {
        events.push_back({e.id, v});
    });

    CHECK(m.show("tip_undo"));
    CHECK(m.isVisible("tip_undo"));
    CHECK(m.hide("tip_undo"));
    CHECK_FALSE(m.isVisible("tip_undo"));

    CHECK(events.size() == 2);
    CHECK(events[0] == std::make_pair(std::string("tip_undo"), true));
    CHECK(events[1] == std::make_pair(std::string("tip_undo"), false));
}

TEST_CASE("Multiple tooltips: only one visible at a time", "[Phase24][Integration]") {
    TooltipManager m;
    for (int i = 1; i <= 5; ++i) {
        std::string id = "t" + std::to_string(i);
        m.registerTooltip({id, "T", "Body", "", TooltipTrigger::Hover, TooltipPosition::Auto, true});
    }
    m.show("t1");
    CHECK(m.isVisible("t1"));
    m.show("t3");
    CHECK_FALSE(m.isVisible("t1"));
    CHECK(m.isVisible("t3"));
    m.show("t5");
    CHECK_FALSE(m.isVisible("t3"));
    CHECK(m.isVisible("t5"));
}

TEST_CASE("Observer receives show-then-hide for auto-replaced tooltip", "[Phase24][Integration]") {
    TooltipManager m;
    m.registerTooltip({"t1", "T1", "B1", "", TooltipTrigger::Hover, TooltipPosition::Auto, true});
    m.registerTooltip({"t2", "T2", "B2", "", TooltipTrigger::Hover, TooltipPosition::Auto, true});

    std::vector<std::pair<std::string,bool>> events;
    m.addObserver([&](const TooltipEntry& e, bool v) { events.push_back({e.id, v}); });

    m.show("t1");
    m.show("t2");  // should implicitly hide t1

    // t1 shown, t1 hidden (by t2 show), t2 shown
    CHECK(events.size() == 3);
    CHECK(events[0] == std::make_pair(std::string("t1"), true));
    CHECK(events[1] == std::make_pair(std::string("t1"), false));
    CHECK(events[2] == std::make_pair(std::string("t2"), true));
}

TEST_CASE("hideAll fires hide observer", "[Phase24][Integration]") {
    TooltipManager m;
    m.registerTooltip({"t1", "T", "B", "", TooltipTrigger::Manual, TooltipPosition::Auto, true});
    int hideCalls = 0;
    m.addObserver([&](const TooltipEntry&, bool v) { if (!v) ++hideCalls; });
    m.show("t1");
    m.hideAll();
    CHECK(hideCalls == 1);
}

TEST_CASE("Disabled tooltip cannot be shown; re-enable restores", "[Phase24][Integration]") {
    TooltipManager m;
    m.registerTooltip({"t1", "T", "B", "", TooltipTrigger::Hover, TooltipPosition::Auto, true});
    m.disableTooltip("t1");
    CHECK_FALSE(m.show("t1"));
    m.enableTooltip("t1");
    CHECK(m.show("t1"));
    CHECK(m.isVisible("t1"));
}

TEST_CASE("clearObservers stops all notifications", "[Phase24][Integration]") {
    TooltipManager m;
    m.registerTooltip({"t1", "T", "B", "", TooltipTrigger::Hover, TooltipPosition::Auto, true});
    int calls = 0;
    m.addObserver([&](const TooltipEntry&, bool) { ++calls; });
    m.clearObservers();
    m.show("t1");
    m.hide("t1");
    CHECK(calls == 0);
}

TEST_CASE("showTimestamp increments across show calls", "[Phase24][Integration]") {
    TooltipManager m;
    m.registerTooltip({"t1", "T", "B", "", TooltipTrigger::Manual, TooltipPosition::Auto, true});
    m.registerTooltip({"t2", "T", "B", "", TooltipTrigger::Manual, TooltipPosition::Auto, true});
    m.show("t1");
    uint64_t ts1 = m.currentVisible().showTimestamp;
    m.show("t2");
    uint64_t ts2 = m.currentVisible().showTimestamp;
    CHECK(ts2 > ts1);
}
