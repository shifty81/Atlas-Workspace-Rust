// Tests/Workspace/test_phase25_status_bar.cpp
// Phase 25 — Workspace Status Bar System
//
// Tests for:
//   1. StatusBarSide    — enum name helpers
//   2. StatusBarItem    — status bar widget descriptor; isValid, equality
//   3. StatusBarSection — ordered priority-sorted item collection
//   4. StatusBarManager — three-section registry with observers
//   5. Integration      — full status bar pipeline

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceStatusBar.h"
#include <string>
#include <vector>

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — StatusBarSide enum
// ═════════════════════════════════════════════════════════════════

TEST_CASE("statusBarSideName returns correct strings", "[Phase25][StatusBarSide]") {
    CHECK(std::string(statusBarSideName(StatusBarSide::Left))   == "Left");
    CHECK(std::string(statusBarSideName(StatusBarSide::Center)) == "Center");
    CHECK(std::string(statusBarSideName(StatusBarSide::Right))  == "Right");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — StatusBarItem
// ═════════════════════════════════════════════════════════════════

TEST_CASE("StatusBarItem default is invalid", "[Phase25][StatusBarItem]") {
    StatusBarItem item;
    CHECK_FALSE(item.isValid());
    CHECK(item.id.empty());
    CHECK(item.enabled);
    CHECK(item.priority == 0);
}

TEST_CASE("StatusBarItem valid construction", "[Phase25][StatusBarItem]") {
    StatusBarItem item{"item_1", "Ready", "Workspace is ready", "icon_ok", 10, true};
    CHECK(item.isValid());
    CHECK(item.id       == "item_1");
    CHECK(item.label    == "Ready");
    CHECK(item.tooltip  == "Workspace is ready");
    CHECK(item.icon     == "icon_ok");
    CHECK(item.priority == 10);
    CHECK(item.enabled);
}

TEST_CASE("StatusBarItem invalid without id", "[Phase25][StatusBarItem]") {
    StatusBarItem item{"", "Label", "", "", 0, true};
    CHECK_FALSE(item.isValid());
}

TEST_CASE("StatusBarItem equality by id", "[Phase25][StatusBarItem]") {
    StatusBarItem a{"id_1", "A", "", "", 0, true};
    StatusBarItem b{"id_1", "B", "", "", 5, false};
    StatusBarItem c{"id_2", "A", "", "", 0, true};
    CHECK(a == b);
    CHECK(a != c);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — StatusBarSection
// ═════════════════════════════════════════════════════════════════

TEST_CASE("StatusBarSection empty state", "[Phase25][StatusBarSection]") {
    StatusBarSection sec;
    CHECK(sec.empty());
    CHECK(sec.count() == 0);
}

TEST_CASE("StatusBarSection add item", "[Phase25][StatusBarSection]") {
    StatusBarSection sec;
    StatusBarItem item{"i1", "L", "", "", 0, true};
    CHECK(sec.add(item));
    CHECK(sec.count() == 1);
    CHECK(sec.contains("i1"));
}

TEST_CASE("StatusBarSection duplicate add fails", "[Phase25][StatusBarSection]") {
    StatusBarSection sec;
    StatusBarItem item{"i1", "L", "", "", 0, true};
    CHECK(sec.add(item));
    CHECK_FALSE(sec.add(item));
}

TEST_CASE("StatusBarSection invalid item rejected", "[Phase25][StatusBarSection]") {
    StatusBarSection sec;
    StatusBarItem item;  // invalid
    CHECK_FALSE(sec.add(item));
}

TEST_CASE("StatusBarSection remove item", "[Phase25][StatusBarSection]") {
    StatusBarSection sec;
    StatusBarItem item{"i1", "L", "", "", 0, true};
    sec.add(item);
    CHECK(sec.remove("i1"));
    CHECK(sec.empty());
}

TEST_CASE("StatusBarSection remove unknown fails", "[Phase25][StatusBarSection]") {
    StatusBarSection sec;
    CHECK_FALSE(sec.remove("nope"));
}

TEST_CASE("StatusBarSection find item", "[Phase25][StatusBarSection]") {
    StatusBarSection sec;
    StatusBarItem item{"i1", "Label", "Tip", "ico", 5, true};
    sec.add(item);
    auto* found = sec.find("i1");
    CHECK(found != nullptr);
    CHECK(found->label == "Label");
}

TEST_CASE("StatusBarSection items sorted by priority", "[Phase25][StatusBarSection]") {
    StatusBarSection sec;
    sec.add({"b", "B", "", "", 20, true});
    sec.add({"a", "A", "", "", 10, true});
    sec.add({"c", "C", "", "", 30, true});
    auto& items = sec.items();
    CHECK(items[0].id == "a");
    CHECK(items[1].id == "b");
    CHECK(items[2].id == "c");
}

TEST_CASE("StatusBarSection update re-sorts", "[Phase25][StatusBarSection]") {
    StatusBarSection sec;
    sec.add({"a", "A", "", "", 10, true});
    sec.add({"b", "B", "", "", 20, true});
    StatusBarItem updated{"a", "A-updated", "", "", 30, true};
    CHECK(sec.update(updated));
    auto& items = sec.items();
    CHECK(items[0].id == "b");  // b now has lower priority
    CHECK(items[1].id == "a");
}

TEST_CASE("StatusBarSection update unknown fails", "[Phase25][StatusBarSection]") {
    StatusBarSection sec;
    StatusBarItem item{"i1", "L", "", "", 0, true};
    CHECK_FALSE(sec.update(item));
}

TEST_CASE("StatusBarSection clear", "[Phase25][StatusBarSection]") {
    StatusBarSection sec;
    sec.add({"i1", "L", "", "", 0, true});
    sec.add({"i2", "L", "", "", 0, true});
    sec.clear();
    CHECK(sec.empty());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — StatusBarManager
// ═════════════════════════════════════════════════════════════════

TEST_CASE("StatusBarManager addItem left section", "[Phase25][StatusBarManager]") {
    StatusBarManager m;
    StatusBarItem item{"i1", "Left Item", "", "", 0, true};
    CHECK(m.addItem(item, StatusBarSide::Left));
    CHECK(m.contains("i1", StatusBarSide::Left));
    CHECK_FALSE(m.contains("i1", StatusBarSide::Right));
}

TEST_CASE("StatusBarManager addItem center and right", "[Phase25][StatusBarManager]") {
    StatusBarManager m;
    m.addItem({"c1", "Center", "", "", 0, true}, StatusBarSide::Center);
    m.addItem({"r1", "Right",  "", "", 0, true}, StatusBarSide::Right);
    CHECK(m.contains("c1", StatusBarSide::Center));
    CHECK(m.contains("r1", StatusBarSide::Right));
}

TEST_CASE("StatusBarManager removeItem", "[Phase25][StatusBarManager]") {
    StatusBarManager m;
    m.addItem({"i1", "L", "", "", 0, true}, StatusBarSide::Left);
    CHECK(m.removeItem("i1", StatusBarSide::Left));
    CHECK_FALSE(m.contains("i1", StatusBarSide::Left));
}

TEST_CASE("StatusBarManager removeItem unknown fails", "[Phase25][StatusBarManager]") {
    StatusBarManager m;
    CHECK_FALSE(m.removeItem("nope", StatusBarSide::Left));
}

TEST_CASE("StatusBarManager updateItem", "[Phase25][StatusBarManager]") {
    StatusBarManager m;
    m.addItem({"i1", "Original", "", "", 0, true}, StatusBarSide::Right);
    StatusBarItem updated{"i1", "Updated", "", "", 0, true};
    CHECK(m.updateItem(updated, StatusBarSide::Right));
    auto* found = m.findItem("i1", StatusBarSide::Right);
    CHECK(found != nullptr);
    CHECK(found->label == "Updated");
}

TEST_CASE("StatusBarManager findItem", "[Phase25][StatusBarManager]") {
    StatusBarManager m;
    m.addItem({"i1", "L", "Tip", "ico", 5, true}, StatusBarSide::Left);
    auto* found = m.findItem("i1", StatusBarSide::Left);
    CHECK(found != nullptr);
    CHECK(found->tooltip == "Tip");
}

TEST_CASE("StatusBarManager enableItem / disableItem", "[Phase25][StatusBarManager]") {
    StatusBarManager m;
    m.addItem({"i1", "L", "", "", 0, true}, StatusBarSide::Left);
    CHECK(m.disableItem("i1", StatusBarSide::Left));
    CHECK_FALSE(m.findItem("i1", StatusBarSide::Left)->enabled);
    CHECK(m.enableItem("i1", StatusBarSide::Left));
    CHECK(m.findItem("i1", StatusBarSide::Left)->enabled);
}

TEST_CASE("StatusBarManager sectionOf returns correct section", "[Phase25][StatusBarManager]") {
    StatusBarManager m;
    m.addItem({"l", "L", "", "", 0, true}, StatusBarSide::Left);
    m.addItem({"c", "C", "", "", 0, true}, StatusBarSide::Center);
    m.addItem({"r", "R", "", "", 0, true}, StatusBarSide::Right);
    CHECK(m.sectionOf(StatusBarSide::Left).count()   == 1);
    CHECK(m.sectionOf(StatusBarSide::Center).count() == 1);
    CHECK(m.sectionOf(StatusBarSide::Right).count()  == 1);
}

TEST_CASE("StatusBarManager observer fires on addItem", "[Phase25][StatusBarManager]") {
    StatusBarManager m;
    std::string lastId; bool lastAdded = false;
    m.addObserver([&](const StatusBarItem& i, StatusBarSide, bool added) {
        lastId = i.id; lastAdded = added;
    });
    m.addItem({"i1", "L", "", "", 0, true}, StatusBarSide::Left);
    CHECK(lastId == "i1");
    CHECK(lastAdded);
}

TEST_CASE("StatusBarManager observer fires on removeItem", "[Phase25][StatusBarManager]") {
    StatusBarManager m;
    bool lastAdded = true;
    m.addObserver([&](const StatusBarItem&, StatusBarSide, bool added) { lastAdded = added; });
    m.addItem({"i1", "L", "", "", 0, true}, StatusBarSide::Left);
    m.removeItem("i1", StatusBarSide::Left);
    CHECK_FALSE(lastAdded);
}

TEST_CASE("StatusBarManager removeObserver stops notifications", "[Phase25][StatusBarManager]") {
    StatusBarManager m;
    int calls = 0;
    uint32_t id = m.addObserver([&](const StatusBarItem&, StatusBarSide, bool) { ++calls; });
    m.removeObserver(id);
    m.addItem({"i1", "L", "", "", 0, true}, StatusBarSide::Left);
    CHECK(calls == 0);
}

TEST_CASE("StatusBarManager clear empties all sections", "[Phase25][StatusBarManager]") {
    StatusBarManager m;
    m.addItem({"l", "L", "", "", 0, true}, StatusBarSide::Left);
    m.addItem({"c", "C", "", "", 0, true}, StatusBarSide::Center);
    m.addItem({"r", "R", "", "", 0, true}, StatusBarSide::Right);
    m.clear();
    CHECK(m.sectionOf(StatusBarSide::Left).empty());
    CHECK(m.sectionOf(StatusBarSide::Center).empty());
    CHECK(m.sectionOf(StatusBarSide::Right).empty());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Full status bar pipeline: add items to all three sides", "[Phase25][Integration]") {
    StatusBarManager m;

    std::vector<std::pair<std::string, bool>> events;
    m.addObserver([&](const StatusBarItem& i, StatusBarSide, bool added) {
        events.push_back({i.id, added});
    });

    m.addItem({"branch",   "main",    "", "", 0, true}, StatusBarSide::Left);
    m.addItem({"errors",   "0 errors","", "", 0, true}, StatusBarSide::Center);
    m.addItem({"line_col", "Ln 1 Col 1","","", 0, true}, StatusBarSide::Right);

    CHECK(events.size() == 3);
    CHECK(m.sectionOf(StatusBarSide::Left).count()   == 1);
    CHECK(m.sectionOf(StatusBarSide::Center).count() == 1);
    CHECK(m.sectionOf(StatusBarSide::Right).count()  == 1);
}

TEST_CASE("Priority sorting preserved across add/remove", "[Phase25][Integration]") {
    StatusBarManager m;
    m.addItem({"c", "C", "", "", 30, true}, StatusBarSide::Left);
    m.addItem({"a", "A", "", "", 10, true}, StatusBarSide::Left);
    m.addItem({"b", "B", "", "", 20, true}, StatusBarSide::Left);

    auto& items = m.sectionOf(StatusBarSide::Left).items();
    CHECK(items[0].id == "a");
    CHECK(items[1].id == "b");
    CHECK(items[2].id == "c");

    m.removeItem("b", StatusBarSide::Left);
    auto& items2 = m.sectionOf(StatusBarSide::Left).items();
    CHECK(items2.size() == 2);
    CHECK(items2[0].id == "a");
    CHECK(items2[1].id == "c");
}

TEST_CASE("Update item label and priority, observer fires", "[Phase25][Integration]") {
    StatusBarManager m;
    int updates = 0;
    m.addObserver([&](const StatusBarItem&, StatusBarSide, bool added) { if (added) ++updates; });

    m.addItem({"i1", "Old", "", "", 0, true}, StatusBarSide::Right);
    updates = 0;
    m.updateItem({"i1", "New", "", "", 99, true}, StatusBarSide::Right);
    CHECK(updates == 1);
    CHECK(m.findItem("i1", StatusBarSide::Right)->label == "New");
}

TEST_CASE("clearObservers stops all notifications", "[Phase25][Integration]") {
    StatusBarManager m;
    int calls = 0;
    m.addObserver([&](const StatusBarItem&, StatusBarSide, bool) { ++calls; });
    m.clearObservers();
    m.addItem({"i1", "L", "", "", 0, true}, StatusBarSide::Left);
    CHECK(calls == 0);
}

TEST_CASE("Multiple observers all fire", "[Phase25][Integration]") {
    StatusBarManager m;
    int c1 = 0, c2 = 0;
    m.addObserver([&](const StatusBarItem&, StatusBarSide, bool) { ++c1; });
    m.addObserver([&](const StatusBarItem&, StatusBarSide, bool) { ++c2; });
    m.addItem({"i1", "L", "", "", 0, true}, StatusBarSide::Left);
    CHECK(c1 == 1);
    CHECK(c2 == 1);
}
