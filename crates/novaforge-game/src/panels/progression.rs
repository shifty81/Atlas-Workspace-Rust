//! [`ProgressionPanel`] — editor panel for NovaForge XP / leveling / skill
//! tree authoring.
//!
//! Mirrors the C++ `NovaForge::ProgressionPanel`.
//!
//! # License
//!
//! GPL v3.0 — this file is part of `novaforge-game`.

use atlas_editor::IEditorPanel;
use serde::{Deserialize, Serialize};

// ── Data types ────────────────────────────────────────────────────────────────

/// XP required to advance from one level to the next.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct LevelThreshold {
    /// Level number (1-based).
    pub level:          u32,
    /// Total XP required to reach this level.
    pub xp_required:    f32,
    /// XP multiplier applied to all gains at this level.
    pub xp_multiplier:  f32,
}

impl LevelThreshold {
    /// Create a threshold entry.
    pub fn new(level: u32, xp_required: f32) -> Self {
        Self { level, xp_required, xp_multiplier: 1.0 }
    }
}

/// A single skill in the skill tree.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SkillUnlock {
    /// Stable skill identifier.
    pub skill_id:              String,
    /// Human-readable skill name.
    pub display_name:          String,
    /// Broad category (e.g. `"combat"`, `"exploration"`, `"trade"`).
    pub category:              String,
    /// Player level required to unlock this skill.
    pub required_level:        u32,
    /// Maximum trainable level for this skill.
    pub max_level:             u32,
    /// ID of the skill that must be unlocked first.  Empty = no prerequisite.
    pub prerequisite_id:       String,
    /// Training time multiplier (higher = longer to train).
    pub training_multiplier:   u32,
}

// ── ProgressionPanel ──────────────────────────────────────────────────────────

/// Editor panel for authoring NovaForge progression rules.
pub struct ProgressionPanel {
    /// Level → XP threshold table.
    pub level_thresholds: Vec<LevelThreshold>,
    /// Skill definitions.
    pub skills:           Vec<SkillUnlock>,
    /// Maximum player level.
    pub max_level:        u32,
}

impl ProgressionPanel {
    pub const PANEL_ID: &'static str = "nf.progression";

    /// Create the panel with default progression curve and starter skills.
    pub fn new() -> Self {
        let level_thresholds = (1u32..=5)
            .map(|l| LevelThreshold::new(l, 100.0 * (l as f32).powi(2)))
            .collect();

        let skills = vec![
            SkillUnlock {
                skill_id:            "small_energy_turret".into(),
                display_name:        "Small Energy Turret".into(),
                category:            "combat".into(),
                required_level:      1,
                max_level:           5,
                prerequisite_id:     String::new(),
                training_multiplier: 1,
            },
            SkillUnlock {
                skill_id:            "navigation".into(),
                display_name:        "Navigation".into(),
                category:            "navigation".into(),
                required_level:      1,
                max_level:           5,
                prerequisite_id:     String::new(),
                training_multiplier: 1,
            },
            SkillUnlock {
                skill_id:            "trade".into(),
                display_name:        "Trade".into(),
                category:            "trade".into(),
                required_level:      2,
                max_level:           5,
                prerequisite_id:     String::new(),
                training_multiplier: 2,
            },
        ];

        Self { level_thresholds, skills, max_level: 5 }
    }

    /// Add a level threshold entry.
    pub fn add_threshold(&mut self, threshold: LevelThreshold) {
        self.level_thresholds.push(threshold);
    }

    /// Add a skill unlock.
    pub fn add_skill(&mut self, skill: SkillUnlock) {
        self.skills.push(skill);
    }

    /// Find a skill by ID.
    pub fn get_skill(&self, skill_id: &str) -> Option<&SkillUnlock> {
        self.skills.iter().find(|s| s.skill_id == skill_id)
    }

    /// XP required to reach `level` (returns `None` if level not in table).
    pub fn xp_for_level(&self, level: u32) -> Option<f32> {
        self.level_thresholds.iter()
            .find(|t| t.level == level)
            .map(|t| t.xp_required)
    }

    /// Validate the progression configuration.
    pub fn validate(&self) -> Vec<String> {
        let mut msgs = Vec::new();
        for t in &self.level_thresholds {
            if t.xp_required < 0.0 {
                msgs.push(format!("Level {}: xp_required must be >= 0", t.level));
            }
        }
        for s in &self.skills {
            if s.skill_id.is_empty() {
                msgs.push("Skill has empty skill_id".into());
            }
            if s.max_level == 0 {
                msgs.push(format!("Skill '{}': max_level must be >= 1", s.skill_id));
            }
        }
        msgs
    }
}

impl Default for ProgressionPanel {
    fn default() -> Self { Self::new() }
}

impl IEditorPanel for ProgressionPanel {
    fn panel_id(&self)    -> &str { Self::PANEL_ID }
    fn panel_title(&self) -> &str { "Progression" }
}

// ── Tests ────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn default_panel_has_thresholds_and_skills() {
        let p = ProgressionPanel::new();
        assert!(!p.level_thresholds.is_empty());
        assert!(!p.skills.is_empty());
    }

    #[test]
    fn xp_for_level_one() {
        let p = ProgressionPanel::new();
        assert_eq!(p.xp_for_level(1), Some(100.0));
    }

    #[test]
    fn xp_for_unknown_level_is_none() {
        let p = ProgressionPanel::new();
        assert!(p.xp_for_level(99).is_none());
    }

    #[test]
    fn get_skill_by_id() {
        let p = ProgressionPanel::new();
        assert!(p.get_skill("navigation").is_some());
        assert!(p.get_skill("no_such_skill").is_none());
    }

    #[test]
    fn validate_clean_panel_returns_no_errors() {
        let p = ProgressionPanel::new();
        assert!(p.validate().is_empty());
    }

    #[test]
    fn validate_negative_xp_is_error() {
        let mut p = ProgressionPanel::new();
        p.add_threshold(LevelThreshold { level: 99, xp_required: -1.0, xp_multiplier: 1.0 });
        assert!(!p.validate().is_empty());
    }

    #[test]
    fn panel_id_and_title() {
        let p = ProgressionPanel::new();
        assert_eq!(p.panel_id(), "nf.progression");
        assert_eq!(p.panel_title(), "Progression");
    }
}
