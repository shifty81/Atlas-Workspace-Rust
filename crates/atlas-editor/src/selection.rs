//! [`SelectionState`] — tracks which ECS entities are currently selected (M7).

use atlas_ecs::EntityId;
use std::collections::HashSet;

/// The editor's selection state.  Supports multi-select.
#[derive(Default)]
pub struct SelectionState {
    selected: HashSet<EntityId>,
    primary:  Option<EntityId>,
}

impl SelectionState {
    pub fn new() -> Self { Self::default() }

    /// Select a single entity, clearing any previous selection.
    pub fn select_one(&mut self, id: EntityId) {
        self.selected.clear();
        self.selected.insert(id);
        self.primary = Some(id);
    }

    /// Toggle an entity in/out of the selection (multi-select).
    pub fn toggle(&mut self, id: EntityId) {
        if self.selected.contains(&id) {
            self.selected.remove(&id);
            if self.primary == Some(id) {
                self.primary = self.selected.iter().next().copied();
            }
        } else {
            self.selected.insert(id);
            if self.primary.is_none() { self.primary = Some(id); }
        }
    }

    /// Add to selection without clearing.
    pub fn add(&mut self, id: EntityId) {
        self.selected.insert(id);
        if self.primary.is_none() { self.primary = Some(id); }
    }

    /// Clear the entire selection.
    pub fn clear(&mut self) {
        self.selected.clear();
        self.primary = None;
    }

    pub fn is_selected(&self, id: EntityId) -> bool { self.selected.contains(&id) }
    pub fn primary(&self)  -> Option<EntityId>      { self.primary }
    pub fn count(&self)    -> usize                 { self.selected.len() }
    pub fn is_empty(&self) -> bool                  { self.selected.is_empty() }

    pub fn iter(&self) -> impl Iterator<Item = EntityId> + '_ {
        self.selected.iter().copied()
    }
}
