//! [`InventoryRulesPanel`] — editor panel for NovaForge inventory / module
//! slot authoring.
//!
//! Mirrors the C++ `NovaForge::InventoryRulesPanel`.
//!
//! # License
//!
//! GPL v3.0 — this file is part of `novaforge-game`.

use atlas_editor::IEditorPanel;
use serde::{Deserialize, Serialize};

// ── Data types ────────────────────────────────────────────────────────────────

/// Configuration for a single inventory slot.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct InventorySlotConfig {
    /// Stable slot identifier (e.g. `"high_0"`, `"med_0"`).
    pub slot_id:           String,
    /// Category of items accepted by this slot (e.g. `"weapons"`, `"modules"`).
    pub accepted_category: String,
    /// Maximum stack size.  `1` means no stacking.
    pub max_stack:         u32,
    /// When `true` the slot is not editable by the player.
    pub locked:            bool,
    /// Slot tier/type (`"high"`, `"med"`, `"low"`, `"rig"`).
    pub slot_type:         String,
}

impl InventorySlotConfig {
    /// Create a simple unlocked slot.
    pub fn new(
        slot_id: impl Into<String>,
        accepted_category: impl Into<String>,
        slot_type: impl Into<String>,
    ) -> Self {
        Self {
            slot_id:           slot_id.into(),
            accepted_category: accepted_category.into(),
            max_stack:         1,
            locked:            false,
            slot_type:         slot_type.into(),
        }
    }
}

/// Storage container rules.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StorageRule {
    /// Container identifier (e.g. `"cargo_hold"`, `"personal_hangar"`).
    pub container_id: String,
    /// Maximum number of item slots.
    pub max_slots:    u32,
    /// Maximum total volume in m³ × 100.  `0` means unlimited.
    pub max_volume:   u32,
    /// Whether items are sorted automatically when added.
    pub auto_sort:    bool,
}

// ── InventoryRulesPanel ───────────────────────────────────────────────────────

/// Editor panel for authoring NovaForge inventory rules.
pub struct InventoryRulesPanel {
    /// Configured inventory slots.
    pub slots:          Vec<InventorySlotConfig>,
    /// Storage container rules.
    pub storage_rules:  Vec<StorageRule>,
}

impl InventoryRulesPanel {
    pub const PANEL_ID: &'static str = "nf.inventory";

    /// Create the panel with default NovaForge ship slot layout.
    pub fn new() -> Self {
        Self {
            slots: vec![
                InventorySlotConfig::new("high_0", "weapons",  "high"),
                InventorySlotConfig::new("high_1", "weapons",  "high"),
                InventorySlotConfig::new("high_2", "weapons",  "high"),
                InventorySlotConfig::new("med_0",  "modules",  "med"),
                InventorySlotConfig::new("med_1",  "modules",  "med"),
                InventorySlotConfig::new("low_0",  "modules",  "low"),
                InventorySlotConfig::new("low_1",  "modules",  "low"),
                InventorySlotConfig::new("rig_0",  "rigs",     "rig"),
            ],
            storage_rules: vec![
                StorageRule { container_id: "cargo_hold".into(),    max_slots: 20, max_volume: 0,     auto_sort: false },
                StorageRule { container_id: "personal_hangar".into(), max_slots: 50, max_volume: 0,   auto_sort: true  },
            ],
        }
    }

    /// Add a slot configuration.
    pub fn add_slot(&mut self, slot: InventorySlotConfig) {
        self.slots.push(slot);
    }

    /// Count slots of a given type.
    pub fn slot_count_by_type(&self, slot_type: &str) -> usize {
        self.slots.iter().filter(|s| s.slot_type == slot_type).count()
    }

    /// Validate slot configuration.
    pub fn validate(&self) -> Vec<String> {
        let mut msgs = Vec::new();
        for s in &self.slots {
            if s.slot_id.is_empty() {
                msgs.push("Slot has empty slot_id".into());
            }
            if s.max_stack == 0 {
                msgs.push(format!("Slot '{}': max_stack must be >= 1", s.slot_id));
            }
        }
        msgs
    }
}

impl Default for InventoryRulesPanel {
    fn default() -> Self { Self::new() }
}

impl IEditorPanel for InventoryRulesPanel {
    fn panel_id(&self)    -> &str { Self::PANEL_ID }
    fn panel_title(&self) -> &str { "Inventory Rules" }
}

// ── Tests ────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn default_panel_has_slots() {
        let p = InventoryRulesPanel::new();
        assert!(!p.slots.is_empty());
    }

    #[test]
    fn high_slot_count() {
        let p = InventoryRulesPanel::new();
        assert_eq!(p.slot_count_by_type("high"), 3);
    }

    #[test]
    fn med_slot_count() {
        let p = InventoryRulesPanel::new();
        assert_eq!(p.slot_count_by_type("med"), 2);
    }

    #[test]
    fn add_slot_increases_count() {
        let mut p = InventoryRulesPanel::new();
        let before = p.slots.len();
        p.add_slot(InventorySlotConfig::new("extra_high", "weapons", "high"));
        assert_eq!(p.slots.len(), before + 1);
    }

    #[test]
    fn validate_clean_panel_returns_no_errors() {
        let p = InventoryRulesPanel::new();
        assert!(p.validate().is_empty());
    }

    #[test]
    fn validate_empty_slot_id_is_error() {
        let mut p = InventoryRulesPanel::new();
        p.slots.push(InventorySlotConfig::new("", "weapons", "high"));
        assert!(!p.validate().is_empty());
    }

    #[test]
    fn panel_id_and_title() {
        let p = InventoryRulesPanel::new();
        assert_eq!(p.panel_id(), "nf.inventory");
        assert_eq!(p.panel_title(), "Inventory Rules");
    }

    #[test]
    fn default_storage_rules_present() {
        let p = InventoryRulesPanel::new();
        assert!(!p.storage_rules.is_empty());
        assert!(p.storage_rules.iter().any(|r| r.container_id == "cargo_hold"));
    }
}
