// Tests/Workspace/test_phase38_dock_tree_serializer.cpp
// Phase 38 — Dock Tree Serializer
//
// Tests for:
//   1. DockSplitOrientation — enum name helpers
//   2. DockNodeKind         — enum name helpers
//   3. DockTreeNode         — isValid, isSplit, isTabStack, addPanel, removePanel
//   4. DockTree             — addNode, removeNode, findNode, rootId, nodeCount, clear
//   5. DockTreeSerializer   — serialize, deserialize, round-trip
//   6. Integration          — full dock layout serialize → deserialize

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/DockTreeSerializer.h"
#include <string>

using namespace NF;

// Helper: build a TabStack node
static DockTreeNode makeTabStack(uint32_t id, std::initializer_list<std::string> panels,
                                 int activeTab = 0) {
    DockTreeNode n;
    n.id        = id;
    n.kind      = DockNodeKind::TabStack;
    n.activeTab = activeTab;
    for (const auto& p : panels) n.addPanel(p);
    return n;
}

// Helper: build a Split node
static DockTreeNode makeSplit(uint32_t id, DockSplitOrientation o,
                              float ratio, uint32_t first, uint32_t second) {
    DockTreeNode n;
    n.id          = id;
    n.kind        = DockNodeKind::Split;
    n.orientation = o;
    n.splitRatio  = ratio;
    n.firstChild  = first;
    n.secondChild = second;
    return n;
}

// ─────────────────────────────────────────────────────────────────
// 1. DockSplitOrientation
// ─────────────────────────────────────────────────────────────────
TEST_CASE("DockSplitOrientation name helpers", "[DockSplitOrientation]") {
    CHECK(std::string(dockSplitOrientationName(DockSplitOrientation::Horizontal)) == "Horizontal");
    CHECK(std::string(dockSplitOrientationName(DockSplitOrientation::Vertical))   == "Vertical");
}

// ─────────────────────────────────────────────────────────────────
// 2. DockNodeKind
// ─────────────────────────────────────────────────────────────────
TEST_CASE("DockNodeKind name helpers", "[DockNodeKind]") {
    CHECK(std::string(dockNodeKindName(DockNodeKind::Split))    == "Split");
    CHECK(std::string(dockNodeKindName(DockNodeKind::TabStack)) == "TabStack");
}

// ─────────────────────────────────────────────────────────────────
// 3. DockTreeNode
// ─────────────────────────────────────────────────────────────────
TEST_CASE("DockTreeNode default is invalid", "[DockTreeNode]") {
    DockTreeNode n;
    CHECK_FALSE(n.isValid()); // id == 0
}

TEST_CASE("DockTreeNode valid when id is non-zero", "[DockTreeNode]") {
    DockTreeNode n;
    n.id = 1;
    CHECK(n.isValid());
}

TEST_CASE("DockTreeNode default kind is TabStack", "[DockTreeNode]") {
    DockTreeNode n;
    n.id = 1;
    CHECK(n.isTabStack());
    CHECK_FALSE(n.isSplit());
}

TEST_CASE("DockTreeNode isSplit when kind is Split", "[DockTreeNode]") {
    DockTreeNode n;
    n.id   = 1;
    n.kind = DockNodeKind::Split;
    CHECK(n.isSplit());
    CHECK_FALSE(n.isTabStack());
}

TEST_CASE("DockTreeNode addPanel appends panel id", "[DockTreeNode]") {
    auto n = makeTabStack(1, {"scene", "audio"});
    REQUIRE(n.panelIds.size() == 2u);
    CHECK(n.panelIds[0] == "scene");
    CHECK(n.panelIds[1] == "audio");
}

TEST_CASE("DockTreeNode removePanel removes existing panel", "[DockTreeNode]") {
    auto n = makeTabStack(1, {"scene", "audio"});
    CHECK(n.removePanel("scene"));
    CHECK(n.panelIds.size() == 1u);
    CHECK(n.panelIds[0] == "audio");
}

TEST_CASE("DockTreeNode removePanel fails for unknown panel", "[DockTreeNode]") {
    auto n = makeTabStack(1, {"scene"});
    CHECK_FALSE(n.removePanel("nope"));
    CHECK(n.panelIds.size() == 1u);
}

// ─────────────────────────────────────────────────────────────────
// 4. DockTree
// ─────────────────────────────────────────────────────────────────
TEST_CASE("DockTree default is empty", "[DockTree]") {
    DockTree t;
    CHECK(t.nodeCount() == 0u);
    CHECK(t.rootId()    == 0u);
}

TEST_CASE("DockTree addNode succeeds for valid node", "[DockTree]") {
    DockTree t;
    CHECK(t.addNode(makeTabStack(1, {"scene"})));
    CHECK(t.nodeCount() == 1u);
}

TEST_CASE("DockTree addNode sets rootId on first node", "[DockTree]") {
    DockTree t;
    t.addNode(makeTabStack(42, {}));
    CHECK(t.rootId() == 42u);
}

TEST_CASE("DockTree addNode rejects invalid node (id==0)", "[DockTree]") {
    DockTree t;
    DockTreeNode bad; // id == 0
    CHECK_FALSE(t.addNode(bad));
    CHECK(t.nodeCount() == 0u);
}

TEST_CASE("DockTree addNode rejects duplicate id", "[DockTree]") {
    DockTree t;
    t.addNode(makeTabStack(1, {"scene"}));
    CHECK_FALSE(t.addNode(makeTabStack(1, {"audio"})));
    CHECK(t.nodeCount() == 1u);
}

TEST_CASE("DockTree removeNode removes existing node", "[DockTree]") {
    DockTree t;
    t.addNode(makeTabStack(1, {"scene"}));
    CHECK(t.removeNode(1));
    CHECK(t.nodeCount() == 0u);
}

TEST_CASE("DockTree removeNode fails for unknown id", "[DockTree]") {
    DockTree t;
    CHECK_FALSE(t.removeNode(99));
}

TEST_CASE("DockTree findNode returns nullptr for unknown id", "[DockTree]") {
    DockTree t;
    CHECK(t.findNode(99) == nullptr);
}

TEST_CASE("DockTree findNode returns correct node", "[DockTree]") {
    DockTree t;
    t.addNode(makeTabStack(7, {"scene", "prefab"}));
    const DockTreeNode* n = t.findNode(7);
    REQUIRE(n != nullptr);
    CHECK(n->panelIds.size() == 2u);
}

TEST_CASE("DockTree findNodeMut allows mutation", "[DockTree]") {
    DockTree t;
    t.addNode(makeTabStack(3, {}));
    DockTreeNode* n = t.findNodeMut(3);
    REQUIRE(n != nullptr);
    n->addPanel("new.panel");
    CHECK(t.findNode(3)->panelIds.size() == 1u);
}

TEST_CASE("DockTree setRootId changes root", "[DockTree]") {
    DockTree t;
    t.addNode(makeTabStack(1, {}));
    t.addNode(makeTabStack(2, {}));
    t.setRootId(2);
    CHECK(t.rootId() == 2u);
}

TEST_CASE("DockTree clear resets tree", "[DockTree]") {
    DockTree t;
    t.addNode(makeTabStack(1, {"scene"}));
    t.addNode(makeTabStack(2, {"audio"}));
    t.clear();
    CHECK(t.nodeCount() == 0u);
    CHECK(t.rootId()    == 0u);
}

// ─────────────────────────────────────────────────────────────────
// 5. DockTreeSerializer
// ─────────────────────────────────────────────────────────────────
TEST_CASE("DockTreeSerializer serialize produces root line", "[DockTreeSerializer]") {
    DockTree t;
    t.addNode(makeTabStack(1, {"scene"}));
    std::string out = DockTreeSerializer::serialize(t);
    CHECK(out.find("root:1") != std::string::npos);
}

TEST_CASE("DockTreeSerializer serialize encodes TabStack node", "[DockTreeSerializer]") {
    DockTree t;
    t.addNode(makeTabStack(1, {"scene", "audio"}, 0));
    std::string out = DockTreeSerializer::serialize(t);
    CHECK(out.find("node:1|tabs") != std::string::npos);
    CHECK(out.find("scene") != std::string::npos);
    CHECK(out.find("audio") != std::string::npos);
}

TEST_CASE("DockTreeSerializer serialize encodes Split node", "[DockTreeSerializer]") {
    DockTree t;
    t.addNode(makeSplit(1, DockSplitOrientation::Horizontal, 0.5f, 2, 3));
    t.addNode(makeTabStack(2, {"scene"}));
    t.addNode(makeTabStack(3, {"audio"}));
    std::string out = DockTreeSerializer::serialize(t);
    CHECK(out.find("node:1|split|Horizontal") != std::string::npos);
}

TEST_CASE("DockTreeSerializer deserialize fails on empty input", "[DockTreeSerializer]") {
    DockTree t;
    CHECK_FALSE(DockTreeSerializer::deserialize("", t));
}

TEST_CASE("DockTreeSerializer deserialize fails when no root line", "[DockTreeSerializer]") {
    DockTree t;
    std::string bad = "node:1|tabs|0|scene\n";
    CHECK_FALSE(DockTreeSerializer::deserialize(bad, t));
}

TEST_CASE("DockTreeSerializer round-trip TabStack preserves panels", "[DockTreeSerializer]") {
    DockTree original;
    original.addNode(makeTabStack(1, {"scene", "audio", "prefab"}, 1));
    original.setRootId(1);

    std::string data = DockTreeSerializer::serialize(original);

    DockTree restored;
    REQUIRE(DockTreeSerializer::deserialize(data, restored));
    CHECK(restored.rootId() == 1u);
    CHECK(restored.nodeCount() == 1u);

    const DockTreeNode* n = restored.findNode(1);
    REQUIRE(n != nullptr);
    CHECK(n->isTabStack());
    CHECK(n->panelIds.size() == 3u);
    CHECK(n->activeTab == 1);
}

TEST_CASE("DockTreeSerializer round-trip Split preserves children and ratio", "[DockTreeSerializer]") {
    DockTree original;
    original.addNode(makeSplit(1, DockSplitOrientation::Vertical, 0.3f, 2, 3));
    original.addNode(makeTabStack(2, {"scene"}));
    original.addNode(makeTabStack(3, {"inspector"}));
    original.setRootId(1);

    std::string data = DockTreeSerializer::serialize(original);

    DockTree restored;
    REQUIRE(DockTreeSerializer::deserialize(data, restored));
    CHECK(restored.nodeCount() == 3u);

    const DockTreeNode* root = restored.findNode(1);
    REQUIRE(root != nullptr);
    CHECK(root->isSplit());
    CHECK(root->orientation == DockSplitOrientation::Vertical);
    CHECK(root->firstChild  == 2u);
    CHECK(root->secondChild == 3u);
    // splitRatio should be approximately 0.3
    CHECK(std::abs(root->splitRatio - 0.3f) < 0.01f);
}

// ─────────────────────────────────────────────────────────────────
// 6. Integration — full dock layout serialize → deserialize
// ─────────────────────────────────────────────────────────────────
TEST_CASE("DockTreeSerializer integration: full layout round-trip", "[DockTreeSerializerIntegration]") {
    // Build a 3-node dock: root split → left (scene+prefab) / right (inspector)
    DockTree original;
    original.addNode(makeSplit(1, DockSplitOrientation::Horizontal, 0.7f, 2, 3));
    original.addNode(makeTabStack(2, {"scene", "prefab"}, 0));
    original.addNode(makeTabStack(3, {"inspector"}, 0));
    original.setRootId(1);

    std::string data = DockTreeSerializer::serialize(original);
    REQUIRE_FALSE(data.empty());

    DockTree restored;
    REQUIRE(DockTreeSerializer::deserialize(data, restored));

    CHECK(restored.rootId()    == 1u);
    CHECK(restored.nodeCount() == 3u);

    const DockTreeNode* root  = restored.findNode(1);
    const DockTreeNode* left  = restored.findNode(2);
    const DockTreeNode* right = restored.findNode(3);

    REQUIRE(root  != nullptr);
    REQUIRE(left  != nullptr);
    REQUIRE(right != nullptr);

    CHECK(root->isSplit());
    CHECK(root->orientation == DockSplitOrientation::Horizontal);
    CHECK(left->isTabStack());
    CHECK(left->panelIds.size() == 2u);
    CHECK(right->isTabStack());
    CHECK(right->panelIds[0] == "inspector");
}

TEST_CASE("DockTreeSerializer integration: empty tree serializes and fails deserialization", "[DockTreeSerializerIntegration]") {
    DockTree empty;
    std::string data = DockTreeSerializer::serialize(empty);
    // An empty tree has no nodes → deserialize should fail (nodeCount == 0)
    DockTree restored;
    CHECK_FALSE(DockTreeSerializer::deserialize(data, restored));
}

TEST_CASE("DockTreeSerializer integration: mutate then re-serialize", "[DockTreeSerializerIntegration]") {
    DockTree t;
    t.addNode(makeTabStack(1, {"scene"}));

    // First round-trip
    DockTree r1;
    REQUIRE(DockTreeSerializer::deserialize(DockTreeSerializer::serialize(t), r1));

    // Mutate: add a panel via findNodeMut
    DockTreeNode* n = r1.findNodeMut(1);
    REQUIRE(n != nullptr);
    n->addPanel("audio");

    // Second round-trip
    DockTree r2;
    REQUIRE(DockTreeSerializer::deserialize(DockTreeSerializer::serialize(r1), r2));
    CHECK(r2.findNode(1)->panelIds.size() == 2u);
}
