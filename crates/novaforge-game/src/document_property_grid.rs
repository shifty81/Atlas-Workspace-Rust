// SPDX-License-Identifier: GPL-3.0-only
// NovaForge document property grid — port of NovaForge::DocumentPropertyGrid.
// Copyright (C) NovaForge contributors. GNU General Public License v3.0.

// ── PropertyFieldType ─────────────────────────────────────────────────────

#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum PropertyFieldType {
    #[default]
    String = 0,
    Float  = 1,
    Int    = 2,
    Bool   = 3,
    Enum   = 4,
}

pub fn property_field_type_name(t: PropertyFieldType) -> &'static str {
    match t {
        PropertyFieldType::String => "String",
        PropertyFieldType::Float  => "Float",
        PropertyFieldType::Int    => "Int",
        PropertyFieldType::Bool   => "Bool",
        PropertyFieldType::Enum   => "Enum",
    }
}

// ── PropertyField ─────────────────────────────────────────────────────────

/// A field within a [`PropertyCategory`].
///
/// **Note:** The `validator` closure is intentionally **not** preserved when
/// cloning this struct — the cloned instance will have `validator: None`.
/// If you need validation after a clone, re-attach the validator manually.
#[allow(dead_code)]
pub struct PropertyField {
    pub key:           String,
    pub display_name:  String,
    pub field_type:    PropertyFieldType,
    pub value:         String,
    pub default_value: String,
    pub read_only:     bool,
    pub tooltip:       String,
    pub enum_options:  Vec<String>,
    pub validator:     Option<Box<dyn Fn(&str) -> bool>>,
}

impl Default for PropertyField {
    fn default() -> Self {
        Self {
            key:           String::new(),
            display_name:  String::new(),
            field_type:    PropertyFieldType::default(),
            value:         String::new(),
            default_value: String::new(),
            read_only:     false,
            tooltip:       String::new(),
            enum_options:  Vec::new(),
            validator:     None,
        }
    }
}

impl Clone for PropertyField {
    fn clone(&self) -> Self {
        Self {
            key:           self.key.clone(),
            display_name:  self.display_name.clone(),
            field_type:    self.field_type,
            value:         self.value.clone(),
            default_value: self.default_value.clone(),
            read_only:     self.read_only,
            tooltip:       self.tooltip.clone(),
            enum_options:  self.enum_options.clone(),
            validator:     None, // closures are not Clone
        }
    }
}

impl std::fmt::Debug for PropertyField {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "PropertyField {{ key: {:?} }}", self.key)
    }
}

// ── PropertyCategory ──────────────────────────────────────────────────────

#[derive(Default)]
pub struct PropertyCategory {
    pub display_name: String,
    pub fields:       Vec<PropertyField>,
}

// ── DocumentPropertyGrid ──────────────────────────────────────────────────

pub struct DocumentPropertyGrid {
    categories: Vec<PropertyCategory>,
    dirty:      bool,
}

impl DocumentPropertyGrid {
    pub fn new() -> Self {
        Self { categories: Vec::new(), dirty: false }
    }

    pub fn add_category(&mut self, cat: PropertyCategory) {
        self.categories.push(cat);
    }

    pub fn add_field(&mut self, category: &str, field: PropertyField) {
        if let Some(cat) = self.categories.iter_mut().find(|c| c.display_name == category) {
            cat.fields.push(field);
        } else {
            let mut cat = PropertyCategory {
                display_name: category.to_string(),
                fields: Vec::new(),
            };
            cat.fields.push(field);
            self.categories.push(cat);
        }
    }

    pub fn categories(&self) -> &[PropertyCategory] { &self.categories }

    pub fn find_field(&self, key: &str) -> Option<&PropertyField> {
        for cat in &self.categories {
            if let Some(f) = cat.fields.iter().find(|f| f.key == key) {
                return Some(f);
            }
        }
        None
    }

    pub fn find_field_mut(&mut self, key: &str) -> Option<&mut PropertyField> {
        for cat in &mut self.categories {
            if let Some(f) = cat.fields.iter_mut().find(|f| f.key == key) {
                return Some(f);
            }
        }
        None
    }

    pub fn set_value(&mut self, key: &str, value: &str) -> bool {
        if let Some(f) = self.find_field_mut(key) {
            if f.read_only { return false; }
            if let Some(ref v) = f.validator {
                if !v(value) { return false; }
            }
            f.value = value.to_string();
            self.dirty = true;
            true
        } else {
            false
        }
    }

    pub fn get_value(&self, key: &str, fallback: &str) -> String {
        self.find_field(key)
            .map(|f| f.value.clone())
            .unwrap_or_else(|| fallback.to_string())
    }

    pub fn reset_to_defaults(&mut self) {
        for cat in &mut self.categories {
            for f in &mut cat.fields {
                f.value = f.default_value.clone();
            }
        }
        self.dirty = true;
    }

    pub fn is_dirty(&self) -> bool { self.dirty }
    pub fn clear_dirty(&mut self) { self.dirty = false; }

    /// Returns `(field_key, message, severity)` for validation failures.
    pub fn validate_all(&self) -> Vec<(String, String, &'static str)> {
        let mut errors = Vec::new();
        for cat in &self.categories {
            for f in &cat.fields {
                if let Some(ref v) = f.validator {
                    if !v(&f.value) {
                        errors.push((f.key.clone(), format!("Invalid value: {}", f.value), "error"));
                    }
                }
                if f.field_type == PropertyFieldType::Enum && !f.enum_options.is_empty() {
                    if !f.enum_options.iter().any(|o| o == &f.value) {
                        errors.push((
                            f.key.clone(),
                            format!("Value '{}' not in enum options", f.value),
                            "warning",
                        ));
                    }
                }
            }
        }
        errors
    }

    pub fn to_flat_map(&self) -> Vec<(String, String)> {
        let mut pairs = Vec::new();
        for cat in &self.categories {
            for f in &cat.fields {
                pairs.push((f.key.clone(), f.value.clone()));
            }
        }
        pairs
    }

    pub fn field_count(&self) -> usize {
        self.categories.iter().map(|c| c.fields.len()).sum()
    }
}

impl Default for DocumentPropertyGrid {
    fn default() -> Self { Self::new() }
}

// ── DocumentPropertyGridBuilder ───────────────────────────────────────────

pub struct DocumentPropertyGridBuilder<'a> {
    grid:             &'a mut DocumentPropertyGrid,
    current_category: String,
}

impl<'a> DocumentPropertyGridBuilder<'a> {
    pub fn new(grid: &'a mut DocumentPropertyGrid) -> Self {
        Self { grid, current_category: String::new() }
    }

    pub fn category(&mut self, name: &str) -> &mut Self {
        self.current_category = name.to_string();
        self
    }

    pub fn field(
        &mut self,
        key: &str,
        display_name: &str,
        field_type: PropertyFieldType,
        default_val: &str,
    ) -> &mut Self {
        let f = PropertyField {
            key:           key.to_string(),
            display_name:  display_name.to_string(),
            field_type,
            value:         default_val.to_string(),
            default_value: default_val.to_string(),
            ..Default::default()
        };
        self.grid.add_field(&self.current_category, f);
        self
    }

    pub fn read_only_field(&mut self, key: &str, display_name: &str, value: &str) -> &mut Self {
        let f = PropertyField {
            key:           key.to_string(),
            display_name:  display_name.to_string(),
            field_type:    PropertyFieldType::String,
            value:         value.to_string(),
            default_value: value.to_string(),
            read_only:     true,
            ..Default::default()
        };
        self.grid.add_field(&self.current_category, f);
        self
    }
}

// ── Tests ─────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    fn make_field(key: &str, default: &str) -> PropertyField {
        PropertyField {
            key:           key.to_string(),
            display_name:  key.to_string(),
            value:         default.to_string(),
            default_value: default.to_string(),
            ..Default::default()
        }
    }

    #[test]
    fn add_field_creates_category() {
        let mut grid = DocumentPropertyGrid::new();
        grid.add_field("Transform", make_field("pos_x", "0.0"));
        assert_eq!(grid.categories().len(), 1);
        assert_eq!(grid.categories()[0].display_name, "Transform");
        assert_eq!(grid.field_count(), 1);
    }

    #[test]
    fn set_value_updates_field() {
        let mut grid = DocumentPropertyGrid::new();
        grid.add_field("Cat", make_field("speed", "5"));
        assert!(grid.set_value("speed", "10"));
        assert_eq!(grid.get_value("speed", ""), "10");
    }

    #[test]
    fn get_value_returns_fallback_for_missing() {
        let grid = DocumentPropertyGrid::new();
        assert_eq!(grid.get_value("missing", "default"), "default");
    }

    #[test]
    fn read_only_cannot_be_set() {
        let mut grid = DocumentPropertyGrid::new();
        let f = PropertyField {
            key:       "ro".to_string(),
            value:     "x".to_string(),
            read_only: true,
            ..Default::default()
        };
        grid.add_field("Cat", f);
        assert!(!grid.set_value("ro", "y"));
        assert_eq!(grid.get_value("ro", ""), "x");
    }

    #[test]
    fn validator_blocks_invalid_values() {
        let mut grid = DocumentPropertyGrid::new();
        let f = PropertyField {
            key:       "age".to_string(),
            value:     "18".to_string(),
            validator: Some(Box::new(|v: &str| v.parse::<u32>().is_ok())),
            ..Default::default()
        };
        grid.add_field("Cat", f);
        assert!(!grid.set_value("age", "abc"));
        assert!(grid.set_value("age", "21"));
        assert_eq!(grid.get_value("age", ""), "21");
    }

    #[test]
    fn reset_to_defaults() {
        let mut grid = DocumentPropertyGrid::new();
        grid.add_field("Cat", make_field("hp", "100"));
        grid.set_value("hp", "50");
        grid.reset_to_defaults();
        assert_eq!(grid.get_value("hp", ""), "100");
    }

    #[test]
    fn validate_all_errors() {
        let mut grid = DocumentPropertyGrid::new();
        let f = PropertyField {
            key:       "level".to_string(),
            value:     "bad".to_string(),
            validator: Some(Box::new(|v| v.parse::<u32>().is_ok())),
            ..Default::default()
        };
        grid.add_field("Cat", f);
        let errs = grid.validate_all();
        assert_eq!(errs.len(), 1);
        assert_eq!(errs[0].2, "error");
    }

    #[test]
    fn enum_validation_warning() {
        let mut grid = DocumentPropertyGrid::new();
        let f = PropertyField {
            key:          "mode".to_string(),
            field_type:   PropertyFieldType::Enum,
            value:        "unknown".to_string(),
            enum_options: vec!["fast".to_string(), "slow".to_string()],
            ..Default::default()
        };
        grid.add_field("Cat", f);
        let errs = grid.validate_all();
        assert_eq!(errs.len(), 1);
        assert_eq!(errs[0].2, "warning");
    }

    #[test]
    fn field_count() {
        let mut grid = DocumentPropertyGrid::new();
        grid.add_field("A", make_field("f1", ""));
        grid.add_field("A", make_field("f2", ""));
        grid.add_field("B", make_field("f3", ""));
        assert_eq!(grid.field_count(), 3);
    }

    #[test]
    fn to_flat_map() {
        let mut grid = DocumentPropertyGrid::new();
        grid.add_field("Cat", make_field("a", "1"));
        grid.add_field("Cat", make_field("b", "2"));
        let map = grid.to_flat_map();
        assert_eq!(map.len(), 2);
        assert_eq!(map[0], ("a".to_string(), "1".to_string()));
        assert_eq!(map[1], ("b".to_string(), "2".to_string()));
    }
}
