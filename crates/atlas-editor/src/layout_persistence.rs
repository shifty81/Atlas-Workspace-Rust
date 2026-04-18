//! [`LayoutPersistence`] — saves and restores editor panel layout (M13).
//!
//! Stores per-panel dimensions, visibility, and dock-side as a JSON blob that
//! can be written to disk and loaded back on the next session.

use std::collections::HashMap;
use serde::{Deserialize, Serialize};

/// Which side of the workspace a panel is docked to.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize, Deserialize, Default)]
pub enum DockSide {
    #[default]
    Left,
    Right,
    Bottom,
    Center,
    Floating,
}

/// Persisted state for a single panel.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct PanelLayout {
    /// Logical panel identifier (e.g. `"outliner"`, `"properties"`).
    pub id:        String,
    /// Whether the panel is currently visible.
    pub visible:   bool,
    /// Width in logical pixels (may be 0.0 if not applicable).
    pub width:     f32,
    /// Height in logical pixels.
    pub height:    f32,
    /// Which side the panel is attached to.
    pub dock_side: DockSide,
}

impl PanelLayout {
    /// Create a default layout entry for `id`.
    pub fn new(id: impl Into<String>) -> Self {
        Self {
            id:        id.into(),
            visible:   true,
            width:     300.0,
            height:    400.0,
            dock_side: DockSide::Left,
        }
    }
}

/// Persists the full editor panel layout across sessions.
#[derive(Default, Debug, Serialize, Deserialize)]
pub struct LayoutPersistence {
    panels: HashMap<String, PanelLayout>,
}

impl LayoutPersistence {
    pub fn new() -> Self { Self::default() }

    /// Insert or replace a panel layout.
    pub fn set(&mut self, layout: PanelLayout) {
        self.panels.insert(layout.id.clone(), layout);
    }

    /// Get the layout for a panel by id.
    pub fn get(&self, id: &str) -> Option<&PanelLayout> {
        self.panels.get(id)
    }

    /// Get a mutable reference to a panel layout.
    pub fn get_mut(&mut self, id: &str) -> Option<&mut PanelLayout> {
        self.panels.get_mut(id)
    }

    /// Remove a panel from the persisted layout.
    pub fn remove(&mut self, id: &str) -> bool {
        self.panels.remove(id).is_some()
    }

    /// Number of panels tracked.
    pub fn count(&self) -> usize { self.panels.len() }

    /// Iterate all tracked panel layouts.
    pub fn iter(&self) -> impl Iterator<Item = &PanelLayout> {
        self.panels.values()
    }

    /// Toggle visibility of a panel.  Returns the new state, or `None` if the
    /// panel id was not found.
    pub fn toggle_visibility(&mut self, id: &str) -> Option<bool> {
        let panel = self.panels.get_mut(id)?;
        panel.visible = !panel.visible;
        Some(panel.visible)
    }

    /// Serialize the entire layout to a JSON string.
    pub fn to_json(&self) -> Result<String, serde_json::Error> {
        serde_json::to_string(self)
    }

    /// Restore layout from a JSON string produced by [`to_json`].
    pub fn from_json(json: &str) -> Result<Self, serde_json::Error> {
        serde_json::from_str(json)
    }

    /// Reset all panels to default visibility / position.
    pub fn reset_to_defaults(&mut self) {
        for panel in self.panels.values_mut() {
            panel.visible   = true;
            panel.dock_side = DockSide::Left;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn make_layout(id: &str) -> PanelLayout {
        PanelLayout::new(id)
    }

    #[test]
    fn new_is_empty() {
        let lp = LayoutPersistence::new();
        assert_eq!(lp.count(), 0);
    }

    #[test]
    fn set_and_get() {
        let mut lp = LayoutPersistence::new();
        lp.set(make_layout("outliner"));
        assert!(lp.get("outliner").is_some());
        assert!(lp.get("missing").is_none());
    }

    #[test]
    fn set_overwrites_existing() {
        let mut lp = LayoutPersistence::new();
        let mut pl = make_layout("console");
        pl.width = 200.0;
        lp.set(pl);
        let mut pl2 = make_layout("console");
        pl2.width = 500.0;
        lp.set(pl2);
        assert_eq!(lp.count(), 1);
        assert_eq!(lp.get("console").unwrap().width, 500.0);
    }

    #[test]
    fn remove_existing() {
        let mut lp = LayoutPersistence::new();
        lp.set(make_layout("prop"));
        assert!(lp.remove("prop"));
        assert_eq!(lp.count(), 0);
    }

    #[test]
    fn remove_nonexistent_returns_false() {
        let mut lp = LayoutPersistence::new();
        assert!(!lp.remove("ghost"));
    }

    #[test]
    fn toggle_visibility() {
        let mut lp = LayoutPersistence::new();
        let mut pl = make_layout("viewport");
        pl.visible = true;
        lp.set(pl);
        let v = lp.toggle_visibility("viewport").unwrap();
        assert!(!v);
        let v2 = lp.toggle_visibility("viewport").unwrap();
        assert!(v2);
    }

    #[test]
    fn toggle_visibility_missing_panel() {
        let mut lp = LayoutPersistence::new();
        assert!(lp.toggle_visibility("ghost").is_none());
    }

    #[test]
    fn json_round_trip() {
        let mut lp = LayoutPersistence::new();
        let mut pl = make_layout("browser");
        pl.dock_side = DockSide::Right;
        pl.height = 600.0;
        lp.set(pl);

        let json = lp.to_json().unwrap();
        let lp2 = LayoutPersistence::from_json(&json).unwrap();
        let panel = lp2.get("browser").unwrap();
        assert_eq!(panel.dock_side, DockSide::Right);
        assert_eq!(panel.height, 600.0);
    }

    #[test]
    fn reset_to_defaults_restores_visibility() {
        let mut lp = LayoutPersistence::new();
        let mut pl = make_layout("console");
        pl.visible   = false;
        pl.dock_side = DockSide::Floating;
        lp.set(pl);

        lp.reset_to_defaults();
        let panel = lp.get("console").unwrap();
        assert!(panel.visible);
        assert_eq!(panel.dock_side, DockSide::Left);
    }

    #[test]
    fn iter_covers_all_panels() {
        let mut lp = LayoutPersistence::new();
        lp.set(make_layout("a"));
        lp.set(make_layout("b"));
        lp.set(make_layout("c"));
        assert_eq!(lp.iter().count(), 3);
    }

    #[test]
    fn dock_side_default_is_left() {
        let pl = PanelLayout::new("test");
        assert_eq!(pl.dock_side, DockSide::Left);
    }
}
