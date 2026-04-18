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

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn default_is_empty() {
        let s = SelectionState::new();
        assert!(s.is_empty());
        assert_eq!(s.count(), 0);
        assert!(s.primary().is_none());
    }

    #[test]
    fn select_one_clears_previous() {
        let mut s = SelectionState::new();
        s.select_one(1);
        s.select_one(2);
        assert_eq!(s.count(), 1);
        assert!(s.is_selected(2));
        assert!(!s.is_selected(1));
        assert_eq!(s.primary(), Some(2));
    }

    #[test]
    fn add_multiple() {
        let mut s = SelectionState::new();
        s.add(1);
        s.add(2);
        s.add(3);
        assert_eq!(s.count(), 3);
        assert!(s.is_selected(1) && s.is_selected(2) && s.is_selected(3));
    }

    #[test]
    fn add_sets_primary_on_first() {
        let mut s = SelectionState::new();
        s.add(5);
        assert_eq!(s.primary(), Some(5));
        s.add(6);
        assert_eq!(s.primary(), Some(5)); // doesn't change
    }

    #[test]
    fn toggle_adds_and_removes() {
        let mut s = SelectionState::new();
        s.toggle(10);
        assert!(s.is_selected(10));
        s.toggle(10);
        assert!(!s.is_selected(10));
    }

    #[test]
    fn toggle_primary_cleared_when_removed() {
        let mut s = SelectionState::new();
        s.toggle(1);
        s.toggle(2);
        assert_eq!(s.primary(), Some(1));
        s.toggle(1); // remove primary
        // primary should now be the other selected entity
        assert!(s.primary().is_some());
        assert!(s.is_selected(2));
    }

    #[test]
    fn clear_empties_selection() {
        let mut s = SelectionState::new();
        s.add(1);
        s.add(2);
        s.clear();
        assert!(s.is_empty());
        assert!(s.primary().is_none());
    }

    #[test]
    fn iter_yields_all_selected() {
        let mut s = SelectionState::new();
        s.add(10);
        s.add(20);
        s.add(30);
        let mut collected: Vec<_> = s.iter().collect();
        collected.sort();
        assert_eq!(collected, vec![10, 20, 30]);
    }
}
