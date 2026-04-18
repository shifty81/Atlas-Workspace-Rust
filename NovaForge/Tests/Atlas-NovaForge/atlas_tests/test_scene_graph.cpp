/**
 * Tests for SceneGraphPanel — hierarchical scene tree editor panel.
 */

#include <cassert>
#include <string>
#include "../editor/tools/SceneGraphPanel.h"

using namespace atlas::editor;

// ══════════════════════════════════════════════════════════════════
// Defaults and construction
// ══════════════════════════════════════════════════════════════════

void test_sg_defaults() {
    SceneGraphPanel panel;
    assert(panel.NodeCount() == 0);
    assert(panel.SelectedNode() == 0);
    assert(panel.SearchFilter().empty());
    assert(std::string(panel.Name()) == "Scene Graph");
}

// ══════════════════════════════════════════════════════════════════
// Node management
// ══════════════════════════════════════════════════════════════════

void test_sg_add_node() {
    SceneGraphPanel panel;
    uint32_t id = panel.AddNode("Ship_01", "Entity");
    assert(id != 0);
    assert(panel.NodeCount() == 1);
    const SceneNode* node = panel.GetNode(id);
    assert(node != nullptr);
    assert(node->name == "Ship_01");
    assert(node->type == "Entity");
    assert(node->parentId == 0);
}

void test_sg_add_child_node() {
    SceneGraphPanel panel;
    uint32_t parent = panel.AddNode("Root", "Group");
    uint32_t child  = panel.AddNode("Child", "Entity", parent);
    assert(child != 0);
    assert(panel.NodeCount() == 2);
    assert(panel.GetNode(child)->parentId == parent);
}

void test_sg_add_child_invalid_parent() {
    SceneGraphPanel panel;
    uint32_t id = panel.AddNode("Orphan", "Entity", 999);
    assert(id == 0); // invalid parent → reject
    assert(panel.NodeCount() == 0);
}

void test_sg_remove_node() {
    SceneGraphPanel panel;
    uint32_t id = panel.AddNode("ToRemove", "Entity");
    assert(panel.RemoveNode(id));
    assert(panel.NodeCount() == 0);
}

void test_sg_remove_cascades_children() {
    SceneGraphPanel panel;
    uint32_t root  = panel.AddNode("Root", "Group");
    uint32_t child = panel.AddNode("Child", "Entity", root);
    uint32_t grandchild = panel.AddNode("GC", "Entity", child);
    assert(panel.NodeCount() == 3);
    (void)grandchild;

    assert(panel.RemoveNode(root));
    assert(panel.NodeCount() == 0);
}

void test_sg_remove_nonexistent() {
    SceneGraphPanel panel;
    assert(!panel.RemoveNode(999));
}

void test_sg_rename_node() {
    SceneGraphPanel panel;
    uint32_t id = panel.AddNode("OldName", "Entity");
    assert(panel.RenameNode(id, "NewName"));
    assert(panel.GetNode(id)->name == "NewName");
}

void test_sg_rename_nonexistent() {
    SceneGraphPanel panel;
    assert(!panel.RenameNode(999, "Name"));
}

// ══════════════════════════════════════════════════════════════════
// Reparenting
// ══════════════════════════════════════════════════════════════════

void test_sg_reparent() {
    SceneGraphPanel panel;
    uint32_t a = panel.AddNode("A", "Group");
    uint32_t b = panel.AddNode("B", "Entity");
    assert(panel.GetNode(b)->parentId == 0);

    assert(panel.Reparent(b, a));
    assert(panel.GetNode(b)->parentId == a);
}

void test_sg_reparent_to_root() {
    SceneGraphPanel panel;
    uint32_t parent = panel.AddNode("Parent", "Group");
    uint32_t child  = panel.AddNode("Child", "Entity", parent);
    assert(panel.Reparent(child, 0));
    assert(panel.GetNode(child)->parentId == 0);
}

void test_sg_reparent_self_rejected() {
    SceneGraphPanel panel;
    uint32_t id = panel.AddNode("Self", "Entity");
    assert(!panel.Reparent(id, id));
}

void test_sg_reparent_cycle_prevented() {
    SceneGraphPanel panel;
    uint32_t a = panel.AddNode("A", "Group");
    uint32_t b = panel.AddNode("B", "Entity", a);
    // Try to make A a child of B (cycle) → should fail
    assert(!panel.Reparent(a, b));
}

void test_sg_reparent_nonexistent_parent() {
    SceneGraphPanel panel;
    uint32_t id = panel.AddNode("Node", "Entity");
    assert(!panel.Reparent(id, 999));
}

// ══════════════════════════════════════════════════════════════════
// Tree queries
// ══════════════════════════════════════════════════════════════════

void test_sg_root_nodes() {
    SceneGraphPanel panel;
    uint32_t a = panel.AddNode("A", "Group");
    uint32_t b = panel.AddNode("B", "Group");
    panel.AddNode("C", "Entity", a); // child of A
    auto roots = panel.RootNodes();
    assert(roots.size() == 2);
    assert(roots[0] == a);
    assert(roots[1] == b);
}

void test_sg_children() {
    SceneGraphPanel panel;
    uint32_t parent = panel.AddNode("Parent", "Group");
    uint32_t c1 = panel.AddNode("C1", "Entity", parent);
    uint32_t c2 = panel.AddNode("C2", "Entity", parent);
    auto kids = panel.Children(parent);
    assert(kids.size() == 2);
    assert(kids[0] == c1);
    assert(kids[1] == c2);
}

void test_sg_depth() {
    SceneGraphPanel panel;
    uint32_t root  = panel.AddNode("Root", "Group");
    uint32_t child = panel.AddNode("Child", "Entity", root);
    uint32_t gc    = panel.AddNode("GC", "Entity", child);
    assert(panel.Depth(root) == 0);
    assert(panel.Depth(child) == 1);
    assert(panel.Depth(gc) == 2);
}

// ══════════════════════════════════════════════════════════════════
// Selection
// ══════════════════════════════════════════════════════════════════

void test_sg_select_node() {
    SceneGraphPanel panel;
    uint32_t id = panel.AddNode("Node", "Entity");
    panel.SelectNode(id);
    assert(panel.SelectedNode() == id);
}

void test_sg_select_nonexistent_ignored() {
    SceneGraphPanel panel;
    panel.SelectNode(999);
    assert(panel.SelectedNode() == 0);
}

void test_sg_clear_selection() {
    SceneGraphPanel panel;
    uint32_t id = panel.AddNode("Node", "Entity");
    panel.SelectNode(id);
    panel.ClearSelection();
    assert(panel.SelectedNode() == 0);
}

void test_sg_remove_clears_selection() {
    SceneGraphPanel panel;
    uint32_t id = panel.AddNode("Node", "Entity");
    panel.SelectNode(id);
    panel.RemoveNode(id);
    assert(panel.SelectedNode() == 0);
}

void test_sg_select_locked_ignored() {
    SceneGraphPanel panel;
    uint32_t id = panel.AddNode("Locked", "Entity");
    panel.SetLocked(id, true);
    panel.SelectNode(id);
    assert(panel.SelectedNode() == 0);
}

// ══════════════════════════════════════════════════════════════════
// Visibility and lock
// ══════════════════════════════════════════════════════════════════

void test_sg_set_visible() {
    SceneGraphPanel panel;
    uint32_t id = panel.AddNode("Node", "Entity");
    assert(panel.GetNode(id)->visible == true);
    assert(panel.SetVisible(id, false));
    assert(panel.GetNode(id)->visible == false);
}

void test_sg_set_locked() {
    SceneGraphPanel panel;
    uint32_t id = panel.AddNode("Node", "Entity");
    assert(panel.GetNode(id)->locked == false);
    assert(panel.SetLocked(id, true));
    assert(panel.GetNode(id)->locked == true);
}

void test_sg_lock_deselects() {
    SceneGraphPanel panel;
    uint32_t id = panel.AddNode("Node", "Entity");
    panel.SelectNode(id);
    assert(panel.SelectedNode() == id);
    panel.SetLocked(id, true);
    assert(panel.SelectedNode() == 0);
}

// ══════════════════════════════════════════════════════════════════
// Expand/collapse
// ══════════════════════════════════════════════════════════════════

void test_sg_toggle_expanded() {
    SceneGraphPanel panel;
    uint32_t id = panel.AddNode("Node", "Group");
    assert(panel.GetNode(id)->expanded == false);
    assert(panel.ToggleExpanded(id));
    assert(panel.GetNode(id)->expanded == true);
    assert(panel.ToggleExpanded(id));
    assert(panel.GetNode(id)->expanded == false);
}

void test_sg_expand_all() {
    SceneGraphPanel panel;
    panel.AddNode("A", "Group");
    panel.AddNode("B", "Group");
    panel.ExpandAll();
    assert(panel.GetNode(1)->expanded == true);
    assert(panel.GetNode(2)->expanded == true);
}

void test_sg_collapse_all() {
    SceneGraphPanel panel;
    panel.AddNode("A", "Group");
    panel.AddNode("B", "Group");
    panel.ExpandAll();
    panel.CollapseAll();
    assert(panel.GetNode(1)->expanded == false);
    assert(panel.GetNode(2)->expanded == false);
}

// ══════════════════════════════════════════════════════════════════
// Search filter
// ══════════════════════════════════════════════════════════════════

void test_sg_search_filter() {
    SceneGraphPanel panel;
    panel.AddNode("ShipA", "Entity");
    panel.AddNode("StationB", "Entity");
    panel.AddNode("ShipC", "Entity");
    assert(panel.FilteredCount() == 3);

    panel.SetSearchFilter("Ship");
    assert(panel.FilteredCount() == 2);

    panel.SetSearchFilter("station");
    assert(panel.FilteredCount() == 1);

    panel.SetSearchFilter("");
    assert(panel.FilteredCount() == 3);
}

void test_sg_search_by_type() {
    SceneGraphPanel panel;
    panel.AddNode("A", "Entity");
    panel.AddNode("B", "Group");
    panel.AddNode("C", "Light");
    panel.SetSearchFilter("light");
    assert(panel.FilteredCount() == 1);
}

// ══════════════════════════════════════════════════════════════════
// Draw and log
// ══════════════════════════════════════════════════════════════════

void test_sg_draw_does_not_crash() {
    SceneGraphPanel panel;
    panel.AddNode("A", "Entity");
    panel.SelectNode(1);
    panel.Draw(); // headless — no context
}

void test_sg_log_after_actions() {
    SceneGraphPanel panel;
    panel.AddNode("A", "Entity");
    panel.RenameNode(1, "B");
    panel.RemoveNode(1);
    assert(panel.Log().size() >= 3); // init + add + rename + remove
}
