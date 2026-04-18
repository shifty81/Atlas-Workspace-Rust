// S143 editor tests: PanelStateSerializer, DockTreeSerializer, LayoutManagerV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;
using Catch::Approx;

// ── PanelStateEntry ───────────────────────────────────────────────────────────

TEST_CASE("PanelStateEntry validity and typed accessors", "[Editor][S143]") {
    PanelStateEntry e;
    REQUIRE(!e.isValid());
    e.panelId   = "panel-1";
    e.panelType = "SceneHierarchy";
    REQUIRE(e.isValid());

    e.set("width", 320);
    e.set("visible", true);
    e.set("scroll", 1.5f);
    e.set("title", std::string("My Panel"));

    REQUIRE(e.getInt("width")          == 320);
    REQUIRE(e.getBool("visible")       == true);
    REQUIRE(e.getFloat("scroll")       == Approx(1.5f));
    REQUIRE(e.get("title")             == "My Panel");
    REQUIRE(e.get("nonexistent", "X")  == "X");
    REQUIRE(e.getInt("nonexistent", 7) == 7);
}

// ── PanelStateSerializer ──────────────────────────────────────────────────────

TEST_CASE("PanelStateSerializer round-trip", "[Editor][S143]") {
    PanelStateEntry e1;
    e1.panelId   = "panel-A";
    e1.panelType = "PropertyGrid";
    e1.set("width", 200);
    e1.set("pinned", true);

    PanelStateEntry e2;
    e2.panelId   = "panel-B";
    e2.panelType = "SceneTree";
    e2.set("height", 400);

    std::string data = PanelStateSerializer::serialize({e1, e2});
    REQUIRE(data.find("panel-A") != std::string::npos);
    REQUIRE(data.find("PropertyGrid") != std::string::npos);
    REQUIRE(data.find("panel-B") != std::string::npos);

    std::vector<PanelStateEntry> out;
    REQUIRE(PanelStateSerializer::deserialize(data, out));
    REQUIRE(out.size() == 2u);

    bool foundA = false, foundB = false;
    for (const auto& e : out) {
        if (e.panelId == "panel-A") { foundA = true; REQUIRE(e.getInt("width") == 200); }
        if (e.panelId == "panel-B") { foundB = true; REQUIRE(e.getInt("height") == 400); }
    }
    REQUIRE(foundA);
    REQUIRE(foundB);
}

TEST_CASE("PanelStateSerializer empty data returns false", "[Editor][S143]") {
    std::vector<PanelStateEntry> out;
    REQUIRE(!PanelStateSerializer::deserialize("", out));
}

TEST_CASE("PanelStateSerializer merge", "[Editor][S143]") {
    PanelStateEntry base;
    base.panelId = "panel-1"; base.panelType = "A"; base.set("x", 0);

    PanelStateEntry patch;
    patch.panelId = "panel-1"; patch.panelType = "A"; patch.set("x", 99); patch.set("y", 50);

    PanelStateEntry newEntry;
    newEntry.panelId = "panel-2"; newEntry.panelType = "B"; newEntry.set("z", 10);

    std::vector<PanelStateEntry> baseVec = {base};
    PanelStateSerializer::merge(baseVec, {patch, newEntry});

    REQUIRE(baseVec.size() == 2u);
    for (const auto& e : baseVec) {
        if (e.panelId == "panel-1") {
            REQUIRE(e.getInt("x") == 99);
            REQUIRE(e.getInt("y") == 50);
        }
        if (e.panelId == "panel-2") {
            REQUIRE(e.getInt("z") == 10);
        }
    }
}

// ── DockTreeNode ──────────────────────────────────────────────────────────────

TEST_CASE("DockSplitOrientation names", "[Editor][S143]") {
    REQUIRE(std::string(dockSplitOrientationName(DockSplitOrientation::Horizontal)) == "Horizontal");
    REQUIRE(std::string(dockSplitOrientationName(DockSplitOrientation::Vertical))   == "Vertical");
}

TEST_CASE("DockNodeKind names", "[Editor][S143]") {
    REQUIRE(std::string(dockNodeKindName(DockNodeKind::Split))     == "Split");
    REQUIRE(std::string(dockNodeKindName(DockNodeKind::TabStack))  == "TabStack");
}

TEST_CASE("DockTreeNode validity and panel management", "[Editor][S143]") {
    DockTreeNode n;
    REQUIRE(!n.isValid());
    n.id   = 1;
    n.kind = DockNodeKind::TabStack;
    REQUIRE(n.isValid());
    REQUIRE(n.isTabStack());
    REQUIRE(!n.isSplit());

    n.addPanel("panel-1");
    n.addPanel("panel-2");
    REQUIRE(n.panelIds.size() == 2u);
    REQUIRE(n.removePanel("panel-1"));
    REQUIRE(n.panelIds.size() == 1u);
    REQUIRE(!n.removePanel("nonexistent"));
}

// ── DockTree ──────────────────────────────────────────────────────────────────

TEST_CASE("DockTree addNode and setRootId", "[Editor][S143]") {
    DockTree tree;
    DockTreeNode n; n.id = 1; n.kind = DockNodeKind::TabStack;
    REQUIRE(tree.addNode(n));
    REQUIRE(tree.nodeCount() == 1u);
    REQUIRE(tree.rootId() == 1u);  // first added becomes root
    tree.setRootId(1);
    REQUIRE(tree.rootId() == 1u);
}

TEST_CASE("DockTree reject duplicate node id", "[Editor][S143]") {
    DockTree tree;
    DockTreeNode n; n.id = 1; n.kind = DockNodeKind::TabStack;
    REQUIRE(tree.addNode(n));
    REQUIRE(!tree.addNode(n));
}

TEST_CASE("DockTree removeNode", "[Editor][S143]") {
    DockTree tree;
    DockTreeNode n; n.id = 1; n.kind = DockNodeKind::TabStack;
    tree.addNode(n);
    REQUIRE(tree.removeNode(1));
    REQUIRE(tree.nodeCount() == 0u);
}

// ── DockTreeSerializer ────────────────────────────────────────────────────────

TEST_CASE("DockTreeSerializer round-trip with split node", "[Editor][S143]") {
    DockTree tree;

    DockTreeNode split;
    split.id = 1; split.kind = DockNodeKind::Split;
    split.orientation = DockSplitOrientation::Horizontal;
    split.splitRatio  = 0.4f;
    split.firstChild  = 2;
    split.secondChild = 3;
    tree.addNode(split);

    DockTreeNode tabs1; tabs1.id = 2; tabs1.kind = DockNodeKind::TabStack;
    tabs1.addPanel("scene"); tabs1.addPanel("hierarchy");
    tree.addNode(tabs1);

    DockTreeNode tabs2; tabs2.id = 3; tabs2.kind = DockNodeKind::TabStack;
    tabs2.addPanel("properties"); tabs2.activeTab = 0;
    tree.addNode(tabs2);

    std::string data = DockTreeSerializer::serialize(tree);
    REQUIRE(data.find("root:1") != std::string::npos);
    REQUIRE(data.find("split") != std::string::npos);
    REQUIRE(data.find("tabs") != std::string::npos);
    REQUIRE(data.find("scene") != std::string::npos);

    DockTree restored;
    REQUIRE(DockTreeSerializer::deserialize(data, restored));
    REQUIRE(restored.rootId() == 1u);
    REQUIRE(restored.nodeCount() == 3u);

    const auto* splitNode = restored.findNode(1);
    REQUIRE(splitNode != nullptr);
    REQUIRE(splitNode->isSplit());
    REQUIRE(splitNode->splitRatio == Approx(0.4f));
    REQUIRE(splitNode->firstChild  == 2u);
    REQUIRE(splitNode->secondChild == 3u);

    const auto* tabNode = restored.findNode(2);
    REQUIRE(tabNode != nullptr);
    REQUIRE(tabNode->isTabStack());
    REQUIRE(tabNode->panelIds.size() == 2u);
}

TEST_CASE("DockTreeSerializer empty data returns false", "[Editor][S143]") {
    DockTree out;
    REQUIRE(!DockTreeSerializer::deserialize("", out));
}

// ── LayoutManagerV1 ───────────────────────────────────────────────────────────

TEST_CASE("LayoutSlot validity and isEmpty", "[Editor][S143]") {
    LayoutSlot s;
    REQUIRE(!s.isValid());
    s.id = 1; s.name = "Default";
    REQUIRE(s.isValid());
    REQUIRE(s.isEmpty());
    s.dockTreeData = "data";
    REQUIRE(!s.isEmpty());
}

TEST_CASE("LayoutManagerV1 addSlot and findSlot", "[Editor][S143]") {
    LayoutManagerV1 mgr;
    LayoutSlot s; s.id = 1; s.name = "Default";
    REQUIRE(mgr.addSlot(s));
    REQUIRE(mgr.slotCount() == 1u);
    REQUIRE(mgr.findSlot(1) != nullptr);
    REQUIRE(mgr.findSlotByName("Default") != nullptr);
    REQUIRE(mgr.findSlot(99) == nullptr);
}

TEST_CASE("LayoutManagerV1 reject duplicate slot id", "[Editor][S143]") {
    LayoutManagerV1 mgr;
    LayoutSlot s; s.id = 1; s.name = "S";
    REQUIRE(mgr.addSlot(s));
    REQUIRE(!mgr.addSlot(s));
}

TEST_CASE("LayoutManagerV1 removeSlot blocks built-in", "[Editor][S143]") {
    LayoutManagerV1 mgr;
    LayoutSlot s; s.id = 1; s.name = "BuiltIn"; s.isBuiltIn = true;
    mgr.addSlot(s);
    REQUIRE(!mgr.removeSlot(1));
    REQUIRE(mgr.slotCount() == 1u);
}

TEST_CASE("LayoutManagerV1 removeSlot user slot", "[Editor][S143]") {
    LayoutManagerV1 mgr;
    LayoutSlot s; s.id = 1; s.name = "User"; s.isBuiltIn = false;
    mgr.addSlot(s);
    REQUIRE(mgr.removeSlot(1));
    REQUIRE(mgr.slotCount() == 0u);
}

TEST_CASE("LayoutManagerV1 setActiveSlot", "[Editor][S143]") {
    LayoutManagerV1 mgr;
    LayoutSlot s; s.id = 1; s.name = "S";
    mgr.addSlot(s);
    REQUIRE(mgr.setActiveSlot(1));
    REQUIRE(mgr.activeSlotId() == 1u);
    REQUIRE(!mgr.setActiveSlot(99));
}

TEST_CASE("LayoutManagerV1 saveToSlot and loadFromSlot round-trip", "[Editor][S143]") {
    LayoutManagerV1 mgr;
    LayoutSlot s; s.id = 1; s.name = "Layout1";
    mgr.addSlot(s);

    // Build a dock tree
    DockTree tree;
    DockTreeNode root; root.id = 1; root.kind = DockNodeKind::TabStack;
    root.addPanel("scene"); root.addPanel("properties");
    tree.addNode(root);

    // Build panel states
    PanelStateEntry pe;
    pe.panelId = "scene"; pe.panelType = "SceneTree";
    pe.set("width", 300);
    std::vector<PanelStateEntry> panels = {pe};

    REQUIRE(mgr.saveToSlot(1, tree, panels));
    REQUIRE(mgr.saveCount() == 1u);

    DockTree restoredTree;
    std::vector<PanelStateEntry> restoredPanels;
    REQUIRE(mgr.loadFromSlot(1, restoredTree, restoredPanels));
    REQUIRE(restoredTree.nodeCount() == 1u);
    REQUIRE(restoredPanels.size()    == 1u);
    REQUIRE(restoredPanels[0].panelId == "scene");
}

TEST_CASE("LayoutManagerV1 loadDefaults registers 4 built-in slots", "[Editor][S143]") {
    LayoutManagerV1 mgr;
    mgr.loadDefaults();
    REQUIRE(mgr.slotCount() == 4u);
    REQUIRE(mgr.findSlotByName("Default")      != nullptr);
    REQUIRE(mgr.findSlotByName("Coding")       != nullptr);
    REQUIRE(mgr.findSlotByName("Debugging")    != nullptr);
    REQUIRE(mgr.findSlotByName("AssetBrowser") != nullptr);
    // All built-in = cannot remove
    for (const auto& slot : mgr.slots()) REQUIRE(slot.isBuiltIn);
}

TEST_CASE("LayoutManagerV1 markDirty and dirtySlotCount", "[Editor][S143]") {
    LayoutManagerV1 mgr;
    LayoutSlot a; a.id = 1; a.name = "A";
    LayoutSlot b; b.id = 2; b.name = "B";
    mgr.addSlot(a); mgr.addSlot(b);
    REQUIRE(mgr.dirtySlotCount() == 0u);
    mgr.markDirty(1);
    REQUIRE(mgr.dirtySlotCount() == 1u);
}
