//! [`TreeView`] — collapsible tree widget state for the editor (M14).
//!
//! Manages tree node expand/collapse state, selection, and depth-first
//! traversal.  Rendering is handled by the caller; this module is purely the
//! data/state layer.

use std::collections::{HashMap, HashSet};

/// Unique identifier for a tree node.
pub type NodeId = u64;

/// A single node in the tree.
#[derive(Clone, Debug)]
pub struct TreeNode {
    pub id:       NodeId,
    pub label:    String,
    pub parent:   Option<NodeId>,
    pub children: Vec<NodeId>,
}

impl TreeNode {
    pub fn new(id: NodeId, label: impl Into<String>, parent: Option<NodeId>) -> Self {
        Self { id, label: label.into(), parent, children: Vec::new() }
    }

    pub fn is_leaf(&self) -> bool { self.children.is_empty() }
}

/// State and data for a flat-stored tree (all nodes stored in one map).
///
/// Supports:
/// - Adding / removing nodes.
/// - Expand / collapse per node.
/// - Single-selection by node id.
/// - Depth-first ordered traversal (skipping children of collapsed nodes).
#[derive(Default, Debug)]
pub struct TreeView {
    nodes:     HashMap<NodeId, TreeNode>,
    roots:     Vec<NodeId>,
    expanded:  HashSet<NodeId>,
    selected:  Option<NodeId>,
}

impl TreeView {
    pub fn new() -> Self { Self::default() }

    /// Add a root-level node.
    pub fn add_root(&mut self, id: NodeId, label: impl Into<String>) {
        let node = TreeNode::new(id, label, None);
        self.nodes.insert(id, node);
        if !self.roots.contains(&id) {
            self.roots.push(id);
        }
    }

    /// Add a child node under `parent_id`.  Returns `false` if the parent does
    /// not exist.
    pub fn add_child(&mut self, parent_id: NodeId, id: NodeId, label: impl Into<String>) -> bool {
        if !self.nodes.contains_key(&parent_id) { return false; }
        let node = TreeNode::new(id, label, Some(parent_id));
        self.nodes.insert(id, node);
        if let Some(parent) = self.nodes.get_mut(&parent_id) {
            if !parent.children.contains(&id) {
                parent.children.push(id);
            }
        }
        true
    }

    /// Remove a node and all its descendants.  Returns `false` if not found.
    pub fn remove(&mut self, id: NodeId) -> bool {
        let Some(node) = self.nodes.remove(&id) else { return false };

        // Unparent from parent or roots
        if let Some(pid) = node.parent {
            if let Some(p) = self.nodes.get_mut(&pid) {
                p.children.retain(|&c| c != id);
            }
        } else {
            self.roots.retain(|&r| r != id);
        }

        // Recursively remove children (clone to avoid borrow)
        let children = node.children.clone();
        for child in children { self.remove(child); }

        self.expanded.remove(&id);
        if self.selected == Some(id) { self.selected = None; }
        true
    }

    /// Expand a node.
    pub fn expand(&mut self, id: NodeId) { self.expanded.insert(id); }

    /// Collapse a node.
    pub fn collapse(&mut self, id: NodeId) { self.expanded.remove(&id); }

    /// Toggle expand/collapse.
    pub fn toggle(&mut self, id: NodeId) {
        if self.expanded.contains(&id) { self.collapse(id); } else { self.expand(id); }
    }

    /// Returns `true` if the node is currently expanded.
    pub fn is_expanded(&self, id: NodeId) -> bool { self.expanded.contains(&id) }

    /// Set the selected node.
    pub fn select(&mut self, id: NodeId) {
        if self.nodes.contains_key(&id) { self.selected = Some(id); }
    }

    /// Clear selection.
    pub fn deselect(&mut self) { self.selected = None; }

    /// Currently selected node id.
    pub fn selected(&self) -> Option<NodeId> { self.selected }

    /// Look up a node.
    pub fn get(&self, id: NodeId) -> Option<&TreeNode> { self.nodes.get(&id) }

    /// Total number of nodes (all depths).
    pub fn node_count(&self) -> usize { self.nodes.len() }

    /// Root-level node ids in insertion order.
    pub fn roots(&self) -> &[NodeId] { &self.roots }

    /// Depth-first visible ordering of node ids, skipping children of
    /// collapsed nodes.  Returns `(NodeId, depth)` pairs.
    pub fn visible_order(&self) -> Vec<(NodeId, usize)> {
        let mut result = Vec::new();
        for &root in &self.roots {
            self.visit(root, 0, &mut result);
        }
        result
    }

    fn visit(&self, id: NodeId, depth: usize, out: &mut Vec<(NodeId, usize)>) {
        out.push((id, depth));
        if self.expanded.contains(&id) {
            if let Some(node) = self.nodes.get(&id) {
                for &child in &node.children {
                    self.visit(child, depth + 1, out);
                }
            }
        }
    }

    /// Expand all nodes.
    pub fn expand_all(&mut self) {
        let ids: Vec<NodeId> = self.nodes.keys().copied().collect();
        for id in ids { self.expanded.insert(id); }
    }

    /// Collapse all nodes.
    pub fn collapse_all(&mut self) {
        self.expanded.clear();
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn build_tree() -> TreeView {
        //  root1
        //    child1a
        //      grandchild1
        //    child1b
        //  root2
        let mut tv = TreeView::new();
        tv.add_root(1, "root1");
        tv.add_root(2, "root2");
        tv.add_child(1, 10, "child1a");
        tv.add_child(1, 11, "child1b");
        tv.add_child(10, 100, "grandchild1");
        tv
    }

    #[test]
    fn node_count() {
        let tv = build_tree();
        assert_eq!(tv.node_count(), 5);
    }

    #[test]
    fn roots_in_insertion_order() {
        let tv = build_tree();
        assert_eq!(tv.roots(), &[1, 2]);
    }

    #[test]
    fn get_existing_node() {
        let tv = build_tree();
        let n = tv.get(10).unwrap();
        assert_eq!(n.label, "child1a");
        assert_eq!(n.parent, Some(1));
    }

    #[test]
    fn is_leaf() {
        let tv = build_tree();
        assert!(!tv.get(1).unwrap().is_leaf());
        assert!(tv.get(100).unwrap().is_leaf());
    }

    #[test]
    fn collapsed_by_default_shows_only_roots() {
        let tv = build_tree();
        let order = tv.visible_order();
        let ids: Vec<NodeId> = order.iter().map(|(id, _)| *id).collect();
        assert_eq!(ids, vec![1, 2]);
    }

    #[test]
    fn expand_shows_direct_children() {
        let mut tv = build_tree();
        tv.expand(1);
        let ids: Vec<NodeId> = tv.visible_order().iter().map(|(id, _)| *id).collect();
        assert_eq!(ids, vec![1, 10, 11, 2]);
    }

    #[test]
    fn expand_nested_shows_grandchildren() {
        let mut tv = build_tree();
        tv.expand(1);
        tv.expand(10);
        let ids: Vec<NodeId> = tv.visible_order().iter().map(|(id, _)| *id).collect();
        assert_eq!(ids, vec![1, 10, 100, 11, 2]);
    }

    #[test]
    fn depth_values_correct() {
        let mut tv = build_tree();
        tv.expand_all();
        let order = tv.visible_order();
        let depth_of = |id: NodeId| order.iter().find(|(i, _)| *i == id).unwrap().1;
        assert_eq!(depth_of(1), 0);
        assert_eq!(depth_of(10), 1);
        assert_eq!(depth_of(100), 2);
    }

    #[test]
    fn toggle_expand_collapse() {
        let mut tv = build_tree();
        tv.toggle(1);
        assert!(tv.is_expanded(1));
        tv.toggle(1);
        assert!(!tv.is_expanded(1));
    }

    #[test]
    fn collapse_all_hides_children() {
        let mut tv = build_tree();
        tv.expand_all();
        tv.collapse_all();
        let ids: Vec<NodeId> = tv.visible_order().iter().map(|(id, _)| *id).collect();
        assert_eq!(ids, vec![1, 2]);
    }

    #[test]
    fn select_and_deselect() {
        let mut tv = build_tree();
        tv.select(10);
        assert_eq!(tv.selected(), Some(10));
        tv.deselect();
        assert_eq!(tv.selected(), None);
    }

    #[test]
    fn select_nonexistent_noop() {
        let mut tv = build_tree();
        tv.select(9999);
        assert_eq!(tv.selected(), None);
    }

    #[test]
    fn remove_node_and_descendants() {
        let mut tv = build_tree();
        assert!(tv.remove(10)); // removes child1a + grandchild1
        assert_eq!(tv.node_count(), 3); // root1, child1b, root2
        assert!(tv.get(10).is_none());
        assert!(tv.get(100).is_none());
    }

    #[test]
    fn remove_clears_selection() {
        let mut tv = build_tree();
        tv.select(10);
        tv.remove(10);
        assert_eq!(tv.selected(), None);
    }

    #[test]
    fn remove_root_updates_roots() {
        let mut tv = build_tree();
        tv.remove(2);
        assert!(!tv.roots().contains(&2));
    }

    #[test]
    fn add_child_to_missing_parent_returns_false() {
        let mut tv = TreeView::new();
        assert!(!tv.add_child(999, 1, "orphan"));
    }
}
