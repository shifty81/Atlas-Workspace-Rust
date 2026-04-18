//! [`MissionRulesPanel`] — editor panel for NovaForge mission / quest authoring.
//!
//! Mirrors the C++ `NovaForge::MissionRulesPanel`.
//!
//! # License
//!
//! GPL v3.0 — this file is part of `novaforge-game`.

use atlas_editor::IEditorPanel;
use serde::{Deserialize, Serialize};

// ── Data types ────────────────────────────────────────────────────────────────

/// Classification of a mission objective.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum ObjectiveType {
    Kill,
    Collect,
    Reach,
    Interact,
    Escort,
    Defend,
    Survive,
    Mine,
    Custom,
}

impl ObjectiveType {
    pub fn as_str(self) -> &'static str {
        match self {
            Self::Kill     => "Kill",
            Self::Collect  => "Collect",
            Self::Reach    => "Reach",
            Self::Interact => "Interact",
            Self::Escort   => "Escort",
            Self::Defend   => "Defend",
            Self::Survive  => "Survive",
            Self::Mine     => "Mine",
            Self::Custom   => "Custom",
        }
    }
}

/// A single mission objective.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MissionObjective {
    /// Stable objective identifier.
    pub objective_id:     String,
    /// How the objective is classified.
    pub objective_type:   ObjectiveType,
    /// Human-readable description shown to the player.
    pub description:      String,
    /// Target entity/item/location ID (context-dependent).
    pub target_id:        String,
    /// Number of times the objective must be completed.
    pub required_count:   u32,
    /// Whether this objective is optional (bonus).
    pub optional:         bool,
}

/// A mission chain entry (missions can chain into each other).
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MissionChainLink {
    /// ID of the mission that follows this one.
    pub next_mission_id:  String,
    /// Condition that must be met for the chain to continue.
    pub condition:        String,
}

/// A complete mission definition.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MissionRule {
    /// Stable mission identifier.
    pub mission_id:       String,
    /// Human-readable mission name.
    pub display_name:     String,
    /// Faction that issues the mission.
    pub issuing_faction:  String,
    /// Minimum standing with the issuing faction required.
    pub min_standing:     f32,
    /// Mission objectives.
    pub objectives:       Vec<MissionObjective>,
    /// Reward in ISK.
    pub isk_reward:       f32,
    /// Reward in loyalty points.
    pub lp_reward:        u32,
    /// Time limit in seconds.  `0` means no time limit.
    pub time_limit_secs:  u32,
    /// Missions that can follow this one.
    pub chain_links:      Vec<MissionChainLink>,
}

// ── MissionRulesPanel ─────────────────────────────────────────────────────────

/// Editor panel for authoring NovaForge mission rules.
pub struct MissionRulesPanel {
    /// All defined missions.
    pub missions: Vec<MissionRule>,
}

impl MissionRulesPanel {
    pub const PANEL_ID: &'static str = "nf.missions";

    /// Create the panel with a default introductory mission chain.
    pub fn new() -> Self {
        Self {
            missions: vec![
                MissionRule {
                    mission_id:      "intro_001".into(),
                    display_name:    "The Awakening".into(),
                    issuing_faction: "Caldari State".into(),
                    min_standing:    -5.0,
                    objectives: vec![
                        MissionObjective {
                            objective_id:   "kill_pirates".into(),
                            objective_type: ObjectiveType::Kill,
                            description:    "Destroy 5 pirate frigates".into(),
                            target_id:      "guristas_frigate".into(),
                            required_count: 5,
                            optional:       false,
                        },
                    ],
                    isk_reward:      75_000.0,
                    lp_reward:       100,
                    time_limit_secs: 0,
                    chain_links:     vec![
                        MissionChainLink {
                            next_mission_id: "intro_002".into(),
                            condition:       String::new(),
                        },
                    ],
                },
            ],
        }
    }

    /// Add a mission rule.
    pub fn add_mission(&mut self, rule: MissionRule) {
        self.missions.push(rule);
    }

    /// Find a mission by ID.
    pub fn get_mission(&self, id: &str) -> Option<&MissionRule> {
        self.missions.iter().find(|m| m.mission_id == id)
    }

    /// Validate mission configuration.
    pub fn validate(&self) -> Vec<String> {
        let mut msgs = Vec::new();
        for m in &self.missions {
            if m.mission_id.is_empty() {
                msgs.push("Mission has empty mission_id".into());
            }
            if m.objectives.is_empty() {
                msgs.push(format!("Mission '{}' has no objectives", m.mission_id));
            }
            if m.isk_reward < 0.0 {
                msgs.push(format!("Mission '{}': isk_reward must be >= 0", m.mission_id));
            }
        }
        msgs
    }
}

impl Default for MissionRulesPanel {
    fn default() -> Self { Self::new() }
}

impl IEditorPanel for MissionRulesPanel {
    fn panel_id(&self)    -> &str { Self::PANEL_ID }
    fn panel_title(&self) -> &str { "Mission Rules" }
}

// ── Tests ────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn default_panel_has_missions() {
        let p = MissionRulesPanel::new();
        assert!(!p.missions.is_empty());
    }

    #[test]
    fn get_mission_by_id() {
        let p = MissionRulesPanel::new();
        assert!(p.get_mission("intro_001").is_some());
        assert!(p.get_mission("nonexistent").is_none());
    }

    #[test]
    fn add_mission_increases_count() {
        let mut p = MissionRulesPanel::new();
        let before = p.missions.len();
        p.add_mission(MissionRule {
            mission_id:      "test_001".into(),
            display_name:    "Test Mission".into(),
            issuing_faction: "Test Faction".into(),
            min_standing:    0.0,
            objectives:      vec![],
            isk_reward:      1000.0,
            lp_reward:       10,
            time_limit_secs: 0,
            chain_links:     vec![],
        });
        assert_eq!(p.missions.len(), before + 1);
    }

    #[test]
    fn validate_clean_panel_returns_no_errors() {
        let p = MissionRulesPanel::new();
        assert!(p.validate().is_empty());
    }

    #[test]
    fn validate_mission_without_objectives_is_error() {
        let mut p = MissionRulesPanel::new();
        p.missions.push(MissionRule {
            mission_id:      "empty_mission".into(),
            display_name:    "Empty".into(),
            issuing_faction: "X".into(),
            min_standing:    0.0,
            objectives:      vec![],
            isk_reward:      0.0,
            lp_reward:       0,
            time_limit_secs: 0,
            chain_links:     vec![],
        });
        assert!(!p.validate().is_empty());
    }

    #[test]
    fn objective_type_as_str() {
        assert_eq!(ObjectiveType::Kill.as_str(), "Kill");
        assert_eq!(ObjectiveType::Collect.as_str(), "Collect");
        assert_eq!(ObjectiveType::Custom.as_str(), "Custom");
    }

    #[test]
    fn panel_id_and_title() {
        let p = MissionRulesPanel::new();
        assert_eq!(p.panel_id(), "nf.missions");
        assert_eq!(p.panel_title(), "Mission Rules");
    }
}
