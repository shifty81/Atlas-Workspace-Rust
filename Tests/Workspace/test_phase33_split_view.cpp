// Tests/Workspace/test_phase33_split_view.cpp
// Phase 33 — Workspace Split View / Tab Groups
//
// Tests for:
//   1. SplitOrientation — enum name helpers
//   2. TabEntry         — isValid; equality
//   3. TabGroup         — add/remove/setActive/hasTab/empty
//   4. SplitPane        — isLeaf/isBranch/isValid
//   5. SplitViewController — root; addTab/removeTab/setActiveTab
//                            setActivePane; splitPane; collapsePane; observers
//   6. Integration      — full split + tab workflow

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceSplitView.h"
#include <string>
#include <vector>

using namespace NF;

// Helper: build a valid TabEntry
static TabEntry makeTab(const std::string& id, const std::string& label = "") {
    TabEntry t;
    t.id        = id;
    t.label     = label.empty() ? id : label;
    t.closeable = true;
    return t;
}

// ─────────────────────────────────────────────────────────────────
// 1. SplitOrientation enum
// ─────────────────────────────────────────────────────────────────
TEST_CASE("SplitOrientation name helpers", "[SplitOrientation]") {
    CHECK(std::string(splitOrientationName(SplitOrientation::None))       == "None");
    CHECK(std::string(splitOrientationName(SplitOrientation::Horizontal)) == "Horizontal");
    CHECK(std::string(splitOrientationName(SplitOrientation::Vertical))   == "Vertical");
}

// ─────────────────────────────────────────────────────────────────
// 2. TabEntry
// ─────────────────────────────────────────────────────────────────
TEST_CASE("TabEntry default is invalid", "[TabEntry]") {
    TabEntry t;
    CHECK_FALSE(t.isValid());
}

TEST_CASE("TabEntry valid with id and label", "[TabEntry]") {
    auto t = makeTab("scene", "Scene Editor");
    CHECK(t.isValid());
}

TEST_CASE("TabEntry invalid without id", "[TabEntry]") {
    TabEntry t;
    t.label = "Label";
    CHECK_FALSE(t.isValid());
}

TEST_CASE("TabEntry invalid without label", "[TabEntry]") {
    TabEntry t;
    t.id = "scene";
    CHECK_FALSE(t.isValid());
}

TEST_CASE("TabEntry equality by id", "[TabEntry]") {
    auto a = makeTab("scene");
    auto b = makeTab("scene", "Different Label");
    CHECK(a == b);
}

// ─────────────────────────────────────────────────────────────────
// 3. TabGroup
// ─────────────────────────────────────────────────────────────────
TEST_CASE("TabGroup default empty state", "[TabGroup]") {
    TabGroup g;
    CHECK(g.empty());
    CHECK(g.tabCount() == 0);
    CHECK(g.activeTabId.empty());
}

TEST_CASE("TabGroup isValid requires groupId", "[TabGroup]") {
    TabGroup g;
    CHECK_FALSE(g.isValid());
    g.groupId = "main";
    CHECK(g.isValid());
}

TEST_CASE("TabGroup addTab sets first as active", "[TabGroup]") {
    TabGroup g;
    g.groupId = "g";
    CHECK(g.addTab(makeTab("scene")));
    CHECK(g.activeTabId == "scene");
    CHECK(g.tabCount() == 1);
}

TEST_CASE("TabGroup addTab duplicate fails", "[TabGroup]") {
    TabGroup g;
    g.groupId = "g";
    g.addTab(makeTab("scene"));
    CHECK_FALSE(g.addTab(makeTab("scene")));
    CHECK(g.tabCount() == 1);
}

TEST_CASE("TabGroup addTab invalid tab fails", "[TabGroup]") {
    TabGroup g;
    g.groupId = "g";
    TabEntry bad; // no id or label
    CHECK_FALSE(g.addTab(bad));
}

TEST_CASE("TabGroup removeTab removes and adjusts active", "[TabGroup]") {
    TabGroup g;
    g.groupId = "g";
    g.addTab(makeTab("a"));
    g.addTab(makeTab("b"));
    g.setActiveTab("b");
    CHECK(g.removeTab("b"));
    CHECK_FALSE(g.hasTab("b"));
    CHECK(g.activeTabId == "a"); // fallback to first
}

TEST_CASE("TabGroup removeTab unknown fails", "[TabGroup]") {
    TabGroup g;
    g.groupId = "g";
    CHECK_FALSE(g.removeTab("nope"));
}

TEST_CASE("TabGroup setActiveTab succeeds for existing tab", "[TabGroup]") {
    TabGroup g;
    g.groupId = "g";
    g.addTab(makeTab("a"));
    g.addTab(makeTab("b"));
    CHECK(g.setActiveTab("b"));
    CHECK(g.activeTabId == "b");
}

TEST_CASE("TabGroup setActiveTab fails for unknown tab", "[TabGroup]") {
    TabGroup g;
    g.groupId = "g";
    g.addTab(makeTab("a"));
    CHECK_FALSE(g.setActiveTab("nope"));
}

TEST_CASE("TabGroup hasTab correctly", "[TabGroup]") {
    TabGroup g;
    g.groupId = "g";
    g.addTab(makeTab("scene"));
    CHECK(g.hasTab("scene"));
    CHECK_FALSE(g.hasTab("audio"));
}

TEST_CASE("TabGroup empty after all tabs removed", "[TabGroup]") {
    TabGroup g;
    g.groupId = "g";
    g.addTab(makeTab("a"));
    g.removeTab("a");
    CHECK(g.empty());
    CHECK(g.activeTabId.empty());
}

// ─────────────────────────────────────────────────────────────────
// 4. SplitPane
// ─────────────────────────────────────────────────────────────────
TEST_CASE("SplitPane default is a leaf", "[SplitPane]") {
    SplitPane p;
    p.id = "root";
    CHECK(p.isLeaf());
    CHECK_FALSE(p.isBranch());
}

TEST_CASE("SplitPane invalid without id", "[SplitPane]") {
    SplitPane p;
    CHECK_FALSE(p.isValid());
}

TEST_CASE("SplitPane valid with id", "[SplitPane]") {
    SplitPane p;
    p.id = "root";
    CHECK(p.isValid());
}

TEST_CASE("SplitPane branch when orientation is set", "[SplitPane]") {
    SplitPane p;
    p.id          = "root";
    p.orientation = SplitOrientation::Horizontal;
    CHECK(p.isBranch());
    CHECK_FALSE(p.isLeaf());
}

// ─────────────────────────────────────────────────────────────────
// 5. SplitViewController
// ─────────────────────────────────────────────────────────────────
TEST_CASE("SplitViewController initialises with root leaf", "[SplitViewController]") {
    SplitViewController svc;
    REQUIRE(svc.root() != nullptr);
    CHECK(svc.root()->isLeaf());
    CHECK(svc.activePaneId() == "root");
}

TEST_CASE("SplitViewController containsPane root", "[SplitViewController]") {
    SplitViewController svc;
    CHECK(svc.containsPane("root"));
    CHECK_FALSE(svc.containsPane("nope"));
}

TEST_CASE("SplitViewController setActivePane succeeds for known pane", "[SplitViewController]") {
    SplitViewController svc;
    CHECK(svc.setActivePane("root"));
    CHECK(svc.activePaneId() == "root");
}

TEST_CASE("SplitViewController setActivePane fails for unknown pane", "[SplitViewController]") {
    SplitViewController svc;
    CHECK_FALSE(svc.setActivePane("missing"));
}

TEST_CASE("SplitViewController addTab to root pane", "[SplitViewController]") {
    SplitViewController svc;
    CHECK(svc.addTab("root", makeTab("scene", "Scene Editor")));
    REQUIRE(svc.root()->tabGroup.hasTab("scene"));
}

TEST_CASE("SplitViewController addTab duplicate fails", "[SplitViewController]") {
    SplitViewController svc;
    svc.addTab("root", makeTab("scene"));
    CHECK_FALSE(svc.addTab("root", makeTab("scene")));
}

TEST_CASE("SplitViewController addTab to unknown pane fails", "[SplitViewController]") {
    SplitViewController svc;
    CHECK_FALSE(svc.addTab("nope", makeTab("scene")));
}

TEST_CASE("SplitViewController removeTab removes correctly", "[SplitViewController]") {
    SplitViewController svc;
    svc.addTab("root", makeTab("scene"));
    CHECK(svc.removeTab("root", "scene"));
    CHECK_FALSE(svc.root()->tabGroup.hasTab("scene"));
}

TEST_CASE("SplitViewController removeTab unknown tab fails", "[SplitViewController]") {
    SplitViewController svc;
    CHECK_FALSE(svc.removeTab("root", "nope"));
}

TEST_CASE("SplitViewController setActiveTab succeeds", "[SplitViewController]") {
    SplitViewController svc;
    svc.addTab("root", makeTab("a"));
    svc.addTab("root", makeTab("b"));
    CHECK(svc.setActiveTab("root", "b"));
    CHECK(svc.root()->tabGroup.activeTabId == "b");
}

TEST_CASE("SplitViewController splitPane horizontally", "[SplitViewController]") {
    SplitViewController svc;
    svc.addTab("root", makeTab("scene"));
    CHECK(svc.splitPane("root", SplitOrientation::Horizontal, "right_pane"));
    SplitPane* r = svc.root();
    CHECK(r->isBranch());
    CHECK(r->orientation == SplitOrientation::Horizontal);
    CHECK(svc.containsPane("right_pane"));
}

TEST_CASE("SplitViewController splitPane vertically", "[SplitViewController]") {
    SplitViewController svc;
    CHECK(svc.splitPane("root", SplitOrientation::Vertical, "bottom_pane"));
    CHECK(svc.root()->orientation == SplitOrientation::Vertical);
}

TEST_CASE("SplitViewController splitPane with None orientation fails", "[SplitViewController]") {
    SplitViewController svc;
    CHECK_FALSE(svc.splitPane("root", SplitOrientation::None, "second"));
}

TEST_CASE("SplitViewController splitPane with duplicate secondId fails", "[SplitViewController]") {
    SplitViewController svc;
    svc.splitPane("root", SplitOrientation::Horizontal, "right");
    CHECK_FALSE(svc.splitPane("root", SplitOrientation::Vertical, "right"));
}

TEST_CASE("SplitViewController splitPane on branch fails", "[SplitViewController]") {
    SplitViewController svc;
    svc.splitPane("root", SplitOrientation::Horizontal, "right");
    CHECK_FALSE(svc.splitPane("root", SplitOrientation::Vertical, "bottom"));
}

TEST_CASE("SplitViewController splitPane preserves existing tabs in first child", "[SplitViewController]") {
    SplitViewController svc;
    svc.addTab("root", makeTab("scene", "Scene"));
    svc.splitPane("root", SplitOrientation::Horizontal, "right");
    SplitPane* firstChild = svc.root()->first.get();
    REQUIRE(firstChild != nullptr);
    CHECK(firstChild->tabGroup.hasTab("scene"));
}

TEST_CASE("SplitViewController collapsePane reverts to leaf", "[SplitViewController]") {
    SplitViewController svc;
    svc.addTab("root", makeTab("scene"));
    svc.splitPane("root", SplitOrientation::Horizontal, "right");
    CHECK(svc.collapsePane("root"));
    CHECK(svc.root()->isLeaf());
    CHECK_FALSE(svc.containsPane("right"));
}

TEST_CASE("SplitViewController collapsePane on leaf fails", "[SplitViewController]") {
    SplitViewController svc;
    CHECK_FALSE(svc.collapsePane("root"));
}

TEST_CASE("SplitViewController observer fires on addTab", "[SplitViewController]") {
    SplitViewController svc;
    int calls = 0;
    svc.addObserver([&](){ ++calls; });
    svc.addTab("root", makeTab("scene"));
    CHECK(calls == 1);
}

TEST_CASE("SplitViewController observer fires on splitPane", "[SplitViewController]") {
    SplitViewController svc;
    int calls = 0;
    svc.addObserver([&](){ ++calls; });
    svc.splitPane("root", SplitOrientation::Horizontal, "right");
    CHECK(calls == 1);
}

TEST_CASE("SplitViewController clearObservers removes observers", "[SplitViewController]") {
    SplitViewController svc;
    int calls = 0;
    svc.addObserver([&](){ ++calls; });
    svc.clearObservers();
    svc.addTab("root", makeTab("scene"));
    CHECK(calls == 0);
}

// ─────────────────────────────────────────────────────────────────
// 6. Integration — full split + tab workflow
// ─────────────────────────────────────────────────────────────────
TEST_CASE("SplitView integration: split root, add tabs to both panes", "[SplitViewIntegration]") {
    SplitViewController svc;

    svc.addTab("root", makeTab("scene",  "Scene Editor"));
    svc.splitPane("root", SplitOrientation::Horizontal, "right");

    // add tab to right pane (the new leaf)
    CHECK(svc.addTab("right", makeTab("audio", "Audio Editor")));

    SplitPane* r = svc.root();
    CHECK(r->isBranch());
    CHECK(r->first->tabGroup.hasTab("scene"));
    CHECK(svc.findPane("right")->tabGroup.hasTab("audio"));
}

TEST_CASE("SplitView integration: switch active pane", "[SplitViewIntegration]") {
    SplitViewController svc;
    svc.splitPane("root", SplitOrientation::Vertical, "bottom");
    CHECK(svc.setActivePane("bottom"));
    CHECK(svc.activePaneId() == "bottom");
}

TEST_CASE("SplitView integration: collapse after adding tabs keeps first-child tabs", "[SplitViewIntegration]") {
    SplitViewController svc;
    svc.addTab("root", makeTab("scene"));
    svc.addTab("root", makeTab("prefab"));
    svc.splitPane("root", SplitOrientation::Horizontal, "right");
    svc.addTab("right", makeTab("audio"));
    svc.collapsePane("root");

    // after collapse, root is a leaf again with tabs from its first child
    SplitPane* r = svc.root();
    CHECK(r->isLeaf());
    CHECK(r->tabGroup.hasTab("scene"));
    CHECK(r->tabGroup.hasTab("prefab"));
}

TEST_CASE("SplitView integration: multiple observers", "[SplitViewIntegration]") {
    SplitViewController svc;
    int obs1 = 0, obs2 = 0;
    svc.addObserver([&](){ ++obs1; });
    svc.addObserver([&](){ ++obs2; });
    svc.addTab("root", makeTab("scene"));
    svc.splitPane("root", SplitOrientation::Horizontal, "right");
    CHECK(obs1 == 2);
    CHECK(obs2 == 2);
}
