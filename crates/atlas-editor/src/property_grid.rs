//! [`PropertyGrid`] — key/value property inspector for the editor (M13).
//!
//! Provides a flat or sectioned property grid where each entry has a string key
//! and a typed [`PropertyValue`].  Used by the Properties panel to display and
//! edit entity component data.

use std::collections::HashMap;
use serde::{Deserialize, Serialize};

/// A typed value stored in a property entry.
#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
pub enum PropertyValue {
    Bool(bool),
    Int(i64),
    Float(f64),
    Text(String),
    Vec3([f32; 3]),
}

impl PropertyValue {
    pub fn as_bool(&self)  -> Option<bool>    { if let Self::Bool(v)  = self { Some(*v) } else { None } }
    pub fn as_int(&self)   -> Option<i64>     { if let Self::Int(v)   = self { Some(*v) } else { None } }
    pub fn as_float(&self) -> Option<f64>     { if let Self::Float(v) = self { Some(*v) } else { None } }
    pub fn as_text(&self)  -> Option<&str>    { if let Self::Text(v)  = self { Some(v)  } else { None } }
    pub fn as_vec3(&self)  -> Option<[f32;3]> { if let Self::Vec3(v)  = self { Some(*v) } else { None } }
}

/// A single editable property.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct PropertyEntry {
    pub key:      String,
    pub value:    PropertyValue,
    /// Read-only entries are shown but cannot be edited by the user.
    pub readonly: bool,
    /// Optional tooltip / description.
    pub tooltip:  Option<String>,
}

impl PropertyEntry {
    pub fn new(key: impl Into<String>, value: PropertyValue) -> Self {
        Self { key: key.into(), value, readonly: false, tooltip: None }
    }

    pub fn read_only(mut self) -> Self {
        self.readonly = true;
        self
    }

    pub fn with_tooltip(mut self, tip: impl Into<String>) -> Self {
        self.tooltip = Some(tip.into());
        self
    }
}

/// A named section grouping related properties.
#[derive(Clone, Debug, Default, Serialize, Deserialize)]
pub struct PropertySection {
    pub title:    String,
    pub expanded: bool,
    entries:      Vec<PropertyEntry>,
}

impl PropertySection {
    pub fn new(title: impl Into<String>) -> Self {
        Self { title: title.into(), expanded: true, entries: Vec::new() }
    }

    pub fn add(&mut self, entry: PropertyEntry) {
        self.entries.push(entry);
    }

    pub fn get(&self, key: &str) -> Option<&PropertyEntry> {
        self.entries.iter().find(|e| e.key == key)
    }

    pub fn get_mut(&mut self, key: &str) -> Option<&mut PropertyEntry> {
        self.entries.iter_mut().find(|e| e.key == key)
    }

    pub fn remove(&mut self, key: &str) -> bool {
        let before = self.entries.len();
        self.entries.retain(|e| e.key != key);
        self.entries.len() < before
    }

    pub fn entry_count(&self) -> usize { self.entries.len() }

    pub fn iter(&self) -> impl Iterator<Item = &PropertyEntry> {
        self.entries.iter()
    }
}

/// Flat property grid: a named set of sections each containing entries.
#[derive(Clone, Debug, Default, Serialize, Deserialize)]
pub struct PropertyGrid {
    sections: Vec<PropertySection>,
    /// Lookup: section title → index for fast access.
    #[serde(skip)]
    index:    HashMap<String, usize>,
}

impl PropertyGrid {
    pub fn new() -> Self { Self::default() }

    /// Add a new section.  If a section with the same title already exists it
    /// is replaced.
    pub fn add_section(&mut self, section: PropertySection) {
        if let Some(&idx) = self.index.get(&section.title) {
            self.sections[idx] = section;
        } else {
            let idx = self.sections.len();
            self.index.insert(section.title.clone(), idx);
            self.sections.push(section);
        }
    }

    pub fn get_section(&self, title: &str) -> Option<&PropertySection> {
        self.index.get(title).map(|&i| &self.sections[i])
    }

    pub fn get_section_mut(&mut self, title: &str) -> Option<&mut PropertySection> {
        self.index.get(title).copied().map(|i| &mut self.sections[i])
    }

    /// Remove a section by title.
    pub fn remove_section(&mut self, title: &str) -> bool {
        if let Some(&idx) = self.index.get(title) {
            self.sections.remove(idx);
            // Rebuild index after removal.
            self.index.clear();
            for (i, s) in self.sections.iter().enumerate() {
                self.index.insert(s.title.clone(), i);
            }
            true
        } else {
            false
        }
    }

    /// Total number of sections.
    pub fn section_count(&self) -> usize { self.sections.len() }

    /// Total number of entries across all sections.
    pub fn total_entry_count(&self) -> usize {
        self.sections.iter().map(|s| s.entry_count()).sum()
    }

    /// Convenience: look up an entry anywhere in the grid by section title and
    /// entry key.
    pub fn get_entry(&self, section: &str, key: &str) -> Option<&PropertyEntry> {
        self.get_section(section)?.get(key)
    }

    /// Convenience: update the value of an entry.  Returns `false` if not found
    /// or the entry is read-only.
    pub fn set_value(&mut self, section: &str, key: &str, value: PropertyValue) -> bool {
        if let Some(s) = self.get_section_mut(section) {
            if let Some(e) = s.get_mut(key) {
                if e.readonly { return false; }
                e.value = value;
                return true;
            }
        }
        false
    }

    /// Iterate all sections in order.
    pub fn iter_sections(&self) -> impl Iterator<Item = &PropertySection> {
        self.sections.iter()
    }

    /// Clear every section and all entries.
    pub fn clear(&mut self) {
        self.sections.clear();
        self.index.clear();
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn transform_section() -> PropertySection {
        let mut s = PropertySection::new("Transform");
        s.add(PropertyEntry::new("position", PropertyValue::Vec3([0.0, 1.0, 2.0])));
        s.add(PropertyEntry::new("rotation", PropertyValue::Vec3([0.0, 0.0, 0.0])));
        s.add(PropertyEntry::new("scale",    PropertyValue::Vec3([1.0, 1.0, 1.0])));
        s
    }

    // ── PropertyValue ─────────────────────────────────────────────────────────

    #[test]
    fn property_value_accessors() {
        assert_eq!(PropertyValue::Bool(true).as_bool(), Some(true));
        assert_eq!(PropertyValue::Int(42).as_int(), Some(42));
        assert!((PropertyValue::Float(3.14).as_float().unwrap() - 3.14).abs() < 1e-9);
        assert_eq!(PropertyValue::Text("hi".into()).as_text(), Some("hi"));
        assert_eq!(PropertyValue::Vec3([1.0, 2.0, 3.0]).as_vec3(), Some([1.0, 2.0, 3.0]));
    }

    #[test]
    fn property_value_wrong_accessor_returns_none() {
        assert!(PropertyValue::Bool(true).as_int().is_none());
        assert!(PropertyValue::Int(1).as_text().is_none());
    }

    // ── PropertyEntry ─────────────────────────────────────────────────────────

    #[test]
    fn entry_read_only_flag() {
        let e = PropertyEntry::new("x", PropertyValue::Int(0)).read_only();
        assert!(e.readonly);
    }

    #[test]
    fn entry_tooltip() {
        let e = PropertyEntry::new("x", PropertyValue::Int(0)).with_tooltip("World X position");
        assert_eq!(e.tooltip.as_deref(), Some("World X position"));
    }

    // ── PropertySection ───────────────────────────────────────────────────────

    #[test]
    fn section_add_and_count() {
        let s = transform_section();
        assert_eq!(s.entry_count(), 3);
    }

    #[test]
    fn section_get_entry() {
        let s = transform_section();
        assert!(s.get("position").is_some());
        assert!(s.get("missing").is_none());
    }

    #[test]
    fn section_remove_entry() {
        let mut s = transform_section();
        assert!(s.remove("rotation"));
        assert_eq!(s.entry_count(), 2);
        assert!(!s.remove("ghost"));
    }

    #[test]
    fn section_iter() {
        let s = transform_section();
        assert_eq!(s.iter().count(), 3);
    }

    // ── PropertyGrid ──────────────────────────────────────────────────────────

    #[test]
    fn grid_starts_empty() {
        let g = PropertyGrid::new();
        assert_eq!(g.section_count(), 0);
        assert_eq!(g.total_entry_count(), 0);
    }

    #[test]
    fn grid_add_section_and_get() {
        let mut g = PropertyGrid::new();
        g.add_section(transform_section());
        assert_eq!(g.section_count(), 1);
        assert!(g.get_section("Transform").is_some());
        assert!(g.get_section("Missing").is_none());
    }

    #[test]
    fn grid_replace_section_same_title() {
        let mut g = PropertyGrid::new();
        g.add_section(transform_section());
        let mut s2 = PropertySection::new("Transform");
        s2.add(PropertyEntry::new("only_one", PropertyValue::Bool(false)));
        g.add_section(s2);
        assert_eq!(g.section_count(), 1);
        assert_eq!(g.get_section("Transform").unwrap().entry_count(), 1);
    }

    #[test]
    fn grid_remove_section() {
        let mut g = PropertyGrid::new();
        g.add_section(transform_section());
        assert!(g.remove_section("Transform"));
        assert_eq!(g.section_count(), 0);
        assert!(!g.remove_section("Transform"));
    }

    #[test]
    fn grid_total_entry_count() {
        let mut g = PropertyGrid::new();
        g.add_section(transform_section()); // 3 entries
        let mut s2 = PropertySection::new("Physics");
        s2.add(PropertyEntry::new("mass", PropertyValue::Float(1.0)));
        g.add_section(s2);
        assert_eq!(g.total_entry_count(), 4);
    }

    #[test]
    fn grid_get_entry() {
        let mut g = PropertyGrid::new();
        g.add_section(transform_section());
        assert!(g.get_entry("Transform", "scale").is_some());
        assert!(g.get_entry("Transform", "nope").is_none());
        assert!(g.get_entry("Nope", "scale").is_none());
    }

    #[test]
    fn grid_set_value() {
        let mut g = PropertyGrid::new();
        g.add_section(transform_section());
        let ok = g.set_value("Transform", "scale", PropertyValue::Vec3([2.0, 2.0, 2.0]));
        assert!(ok);
        let v = g.get_entry("Transform", "scale").unwrap().value.as_vec3().unwrap();
        assert_eq!(v, [2.0, 2.0, 2.0]);
    }

    #[test]
    fn grid_set_value_read_only_rejected() {
        let mut g = PropertyGrid::new();
        let mut s = PropertySection::new("Meta");
        s.add(PropertyEntry::new("id", PropertyValue::Text("abc".into())).read_only());
        g.add_section(s);
        let ok = g.set_value("Meta", "id", PropertyValue::Text("xyz".into()));
        assert!(!ok);
        assert_eq!(g.get_entry("Meta", "id").unwrap().value.as_text(), Some("abc"));
    }

    #[test]
    fn grid_clear() {
        let mut g = PropertyGrid::new();
        g.add_section(transform_section());
        g.clear();
        assert_eq!(g.section_count(), 0);
    }

    #[test]
    fn grid_iter_sections() {
        let mut g = PropertyGrid::new();
        g.add_section(PropertySection::new("A"));
        g.add_section(PropertySection::new("B"));
        assert_eq!(g.iter_sections().count(), 2);
    }
}
