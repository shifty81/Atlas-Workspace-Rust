//! Hierarchical scene graph — parent/child entity relationships.

use std::collections::HashMap;
use crate::entity::{EntityId, INVALID_ENTITY};

/// Tracks parent-child relationships between entities.
#[derive(Default)]
pub struct SceneGraph {
    parent:   HashMap<EntityId, EntityId>,
    children: HashMap<EntityId, Vec<EntityId>>,
}

impl SceneGraph {
    pub fn new() -> Self {
        Self::default()
    }

    /// Set `child`'s parent to `parent`, detaching it from any previous parent.
    pub fn set_parent(&mut self, child: EntityId, parent: EntityId) {
        // Detach from old parent
        self.remove_from_parent(child);
        self.parent.insert(child, parent);
        self.children.entry(parent).or_default().push(child);
    }

    /// Detach `entity` from its current parent, making it a root node.
    pub fn remove_from_parent(&mut self, entity: EntityId) {
        if let Some(old_parent) = self.parent.remove(&entity) {
            if let Some(siblings) = self.children.get_mut(&old_parent) {
                siblings.retain(|&e| e != entity);
            }
        }
    }

    /// Returns the parent of `entity`, or [`INVALID_ENTITY`] if it is a root.
    pub fn parent(&self, entity: EntityId) -> EntityId {
        *self.parent.get(&entity).unwrap_or(&INVALID_ENTITY)
    }

    /// Returns `true` if `entity` has no parent.
    pub fn is_root(&self, entity: EntityId) -> bool {
        !self.parent.contains_key(&entity)
    }

    /// Number of direct children of `entity`.
    pub fn child_count(&self, entity: EntityId) -> usize {
        self.children.get(&entity).map_or(0, |v| v.len())
    }

    /// Slice of direct children of `entity`.
    pub fn children(&self, entity: EntityId) -> &[EntityId] {
        self.children.get(&entity).map(|v| v.as_slice()).unwrap_or(&[])
    }

    /// Returns `true` if `entity` is a descendant (at any depth) of `ancestor`.
    pub fn is_descendant_of(&self, entity: EntityId, ancestor: EntityId) -> bool {
        let mut cur = entity;
        loop {
            let p = self.parent(cur);
            if p == INVALID_ENTITY {
                return false;
            }
            if p == ancestor {
                return true;
            }
            cur = p;
        }
    }

    /// Remove `entity` from the graph.  Its children are re-parented to its
    /// former parent (or made roots if `entity` was a root).
    pub fn remove_entity(&mut self, entity: EntityId) {
        let old_parent = self.parent.remove(&entity).unwrap_or(INVALID_ENTITY);

        // Detach from old_parent's child list
        if old_parent != INVALID_ENTITY {
            if let Some(siblings) = self.children.get_mut(&old_parent) {
                siblings.retain(|&e| e != entity);
            }
        }

        // Re-parent children
        if let Some(kids) = self.children.remove(&entity) {
            for child in kids {
                if old_parent != INVALID_ENTITY {
                    self.parent.insert(child, old_parent);
                    self.children.entry(old_parent).or_default().push(child);
                } else {
                    self.parent.remove(&child);
                }
            }
        }
    }
}
