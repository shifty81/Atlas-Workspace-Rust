//! [`CharacterRulesPanel`] — editor panel for NovaForge character rule
//! authoring.
//!
//! Manages character class presets, base stat caps, starting skills, and
//! appearance configuration.  Mirrors the C++ `NovaForge::CharacterRulesPanel`.
//!
//! # License
//!
//! GPL v3.0 — this file is part of `novaforge-game`.

use atlas_editor::IEditorPanel;
use serde::{Deserialize, Serialize};

// ── Data types ────────────────────────────────────────────────────────────────

/// A min/max cap for a single character stat.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StatCapEntry {
    /// Stat identifier (e.g. `"intelligence"`, `"willpower"`).
    pub stat_id:    String,
    /// Racial or class base value.
    pub base_value: f32,
    /// Minimum allowed value.
    pub min_value:  f32,
    /// Maximum allowed value.
    pub max_value:  f32,
}

impl StatCapEntry {
    /// Create a stat cap with default range `[0, 100]`.
    pub fn new(stat_id: impl Into<String>, base_value: f32) -> Self {
        Self { stat_id: stat_id.into(), base_value, min_value: 0.0, max_value: 100.0 }
    }
}

/// Appearance customisation limits.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AppearanceConfig {
    /// Whether the player can customise body type.
    pub allow_body_type:  bool,
    /// Whether the player can customise facial features.
    pub allow_face_edit:  bool,
    /// Number of preset clothing styles available at creation.
    pub clothing_presets: u32,
}

impl Default for AppearanceConfig {
    fn default() -> Self {
        Self { allow_body_type: true, allow_face_edit: true, clothing_presets: 3 }
    }
}

/// A character class preset that players can choose at creation.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CharacterClassPreset {
    /// Stable class identifier.
    pub class_id:        String,
    /// Human-readable name.
    pub display_name:    String,
    /// Short description shown at character creation.
    pub description:     String,
    /// Base stats for this class.
    pub base_stats:      Vec<StatCapEntry>,
    /// Skill IDs the character starts with.
    pub starting_skills: Vec<String>,
    /// Whether this class is available for player selection.
    pub selectable:      bool,
    /// Race identifier this class belongs to (empty = all races).
    pub race_id:         String,
}

// ── CharacterRulesPanel ───────────────────────────────────────────────────────

/// Editor panel for authoring NovaForge character rules.
pub struct CharacterRulesPanel {
    /// Character class presets.
    pub class_presets:  Vec<CharacterClassPreset>,
    /// Appearance configuration.
    pub appearance:     AppearanceConfig,
    /// Maximum character slots per account.
    pub max_characters: u32,
}

impl CharacterRulesPanel {
    pub const PANEL_ID: &'static str = "nf.characters";

    /// Create the panel with default NovaForge class presets.
    pub fn new() -> Self {
        let caldari_pilot = CharacterClassPreset {
            class_id:     "caldari_pilot".into(),
            display_name: "Caldari Pilot".into(),
            description:  "Expert in missiles and shields. Caldari State ancestry.".into(),
            base_stats: vec![
                StatCapEntry::new("intelligence", 20.0),
                StatCapEntry::new("willpower",    17.0),
                StatCapEntry::new("perception",   19.0),
                StatCapEntry::new("memory",       18.0),
                StatCapEntry::new("charisma",     17.0),
            ],
            starting_skills: vec![
                "small_energy_turret".into(),
                "navigation".into(),
                "spaceship_command".into(),
            ],
            selectable: true,
            race_id:    "caldari".into(),
        };

        let amarr_pilot = CharacterClassPreset {
            class_id:     "amarr_pilot".into(),
            display_name: "Amarr Pilot".into(),
            description:  "Expert in lasers and armor. Amarr Empire ancestry.".into(),
            base_stats: vec![
                StatCapEntry::new("intelligence", 18.0),
                StatCapEntry::new("willpower",    21.0),
                StatCapEntry::new("perception",   17.0),
                StatCapEntry::new("memory",       17.0),
                StatCapEntry::new("charisma",     18.0),
            ],
            starting_skills: vec![
                "small_energy_turret".into(),
                "navigation".into(),
                "spaceship_command".into(),
            ],
            selectable: true,
            race_id:    "amarr".into(),
        };

        Self {
            class_presets:  vec![caldari_pilot, amarr_pilot],
            appearance:     AppearanceConfig::default(),
            max_characters: 3,
        }
    }

    /// Add a class preset.
    pub fn add_class(&mut self, preset: CharacterClassPreset) {
        self.class_presets.push(preset);
    }

    /// Find a class preset by ID.
    pub fn get_class(&self, class_id: &str) -> Option<&CharacterClassPreset> {
        self.class_presets.iter().find(|c| c.class_id == class_id)
    }

    /// Number of selectable classes.
    pub fn selectable_count(&self) -> usize {
        self.class_presets.iter().filter(|c| c.selectable).count()
    }

    /// Validate the panel data.
    pub fn validate(&self) -> Vec<String> {
        let mut msgs = Vec::new();
        if self.class_presets.is_empty() {
            msgs.push("No character class presets defined".into());
        }
        for c in &self.class_presets {
            if c.class_id.is_empty() {
                msgs.push("Class has empty class_id".into());
            }
        }
        if self.max_characters == 0 {
            msgs.push("max_characters must be >= 1".into());
        }
        msgs
    }
}

impl Default for CharacterRulesPanel {
    fn default() -> Self { Self::new() }
}

impl IEditorPanel for CharacterRulesPanel {
    fn panel_id(&self)    -> &str { Self::PANEL_ID }
    fn panel_title(&self) -> &str { "Character Rules" }
}

// ── Tests ────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn default_panel_has_class_presets() {
        let p = CharacterRulesPanel::new();
        assert!(!p.class_presets.is_empty());
    }

    #[test]
    fn selectable_count_matches_defaults() {
        let p = CharacterRulesPanel::new();
        assert_eq!(p.selectable_count(), 2);
    }

    #[test]
    fn get_class_by_id() {
        let p = CharacterRulesPanel::new();
        assert!(p.get_class("caldari_pilot").is_some());
        assert!(p.get_class("no_such_class").is_none());
    }

    #[test]
    fn add_class_increases_count() {
        let mut p = CharacterRulesPanel::new();
        let before = p.class_presets.len();
        p.add_class(CharacterClassPreset {
            class_id:        "test_class".into(),
            display_name:    "Test".into(),
            description:     String::new(),
            base_stats:      vec![],
            starting_skills: vec![],
            selectable:      true,
            race_id:         String::new(),
        });
        assert_eq!(p.class_presets.len(), before + 1);
    }

    #[test]
    fn validate_clean_panel_returns_no_errors() {
        let p = CharacterRulesPanel::new();
        assert!(p.validate().is_empty());
    }

    #[test]
    fn validate_empty_class_id_is_error() {
        let mut p = CharacterRulesPanel::new();
        p.class_presets.push(CharacterClassPreset {
            class_id:        String::new(),
            display_name:    "Bad".into(),
            description:     String::new(),
            base_stats:      vec![],
            starting_skills: vec![],
            selectable:      true,
            race_id:         String::new(),
        });
        assert!(!p.validate().is_empty());
    }

    #[test]
    fn panel_id_and_title() {
        let p = CharacterRulesPanel::new();
        assert_eq!(p.panel_id(), "nf.characters");
        assert_eq!(p.panel_title(), "Character Rules");
    }

    #[test]
    fn stat_cap_entry_defaults() {
        let s = StatCapEntry::new("intelligence", 20.0);
        assert_eq!(s.min_value, 0.0);
        assert_eq!(s.max_value, 100.0);
    }
}
