//! [`EconomyPanel`] — editor panel for NovaForge economy authoring.
//!
//! Manages in-game currency definitions, pricing multipliers, reward rates,
//! and economy balance parameters.  Mirrors the C++ `NovaForge::EconomyPanel`.
//!
//! # License
//!
//! GPL v3.0 — this file is part of `novaforge-game`.

use atlas_editor::IEditorPanel;
use serde::{Deserialize, Serialize};

// ── Data types ────────────────────────────────────────────────────────────────

/// A currency that can be earned and optionally traded between players.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CurrencyDefinition {
    /// Stable machine-readable identifier (e.g. `"isk"`).
    pub id:           String,
    /// Human-readable display name (e.g. `"ISK"`).
    pub display_name: String,
    /// Multiplier applied to the base earn rate.  Must be ≥ 0.
    pub earn_rate:    f32,
    /// Maximum amount a player can hold.  `0.0` means unlimited.
    pub spend_cap:    f32,
    /// Whether players can trade this currency directly.
    pub tradeable:    bool,
}

impl CurrencyDefinition {
    /// Create a new tradeable currency with default rates.
    pub fn new(id: impl Into<String>, display_name: impl Into<String>) -> Self {
        Self {
            id:           id.into(),
            display_name: display_name.into(),
            earn_rate:    1.0,
            spend_cap:    0.0,
            tradeable:    true,
        }
    }
}

/// A pricing rule that modifies the base price of an item category.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct EconomyPricingRule {
    /// Item or asset identifier this rule applies to.
    pub item_id:           String,
    /// Broad category grouping for display (e.g. `"ships"`, `"weapons"`).
    pub item_category:     String,
    /// Base market price in the primary currency.
    pub base_price:        f32,
    /// Multiplier applied under high-demand conditions (`1.0` = no effect).
    pub demand_multiplier: f32,
    /// Multiplier applied under high-supply conditions (`1.0` = no effect).
    pub supply_multiplier: f32,
}

// ── EconomyPanel ──────────────────────────────────────────────────────────────

/// Editor panel for authoring NovaForge economic rules.
pub struct EconomyPanel {
    /// Defined currencies.
    pub currencies:            Vec<CurrencyDefinition>,
    /// Item pricing rules.
    pub pricing_rules:         Vec<EconomyPricingRule>,
    /// Annual inflation modifier applied globally to all prices.
    pub global_inflation_rate: f32,
    /// Whether in-game economic simulation is active.
    pub economy_enabled:       bool,
}

impl EconomyPanel {
    pub const PANEL_ID: &'static str = "nf.economy";

    /// Create the panel with built-in NovaForge defaults.
    pub fn new() -> Self {
        Self {
            currencies: vec![
                CurrencyDefinition::new("isk", "ISK"),
                CurrencyDefinition {
                    id:           "loyalty_points".into(),
                    display_name: "Loyalty Points".into(),
                    earn_rate:    0.0,
                    spend_cap:    0.0,
                    tradeable:    false,
                },
            ],
            pricing_rules: vec![
                EconomyPricingRule {
                    item_id:           "frigate_hull".into(),
                    item_category:     "ships".into(),
                    base_price:        360_000.0,
                    demand_multiplier: 1.0,
                    supply_multiplier: 1.0,
                },
                EconomyPricingRule {
                    item_id:           "weapon_turret".into(),
                    item_category:     "weapons".into(),
                    base_price:        25_000.0,
                    demand_multiplier: 1.2,
                    supply_multiplier: 0.8,
                },
                EconomyPricingRule {
                    item_id:           "ore_ferrium".into(),
                    item_category:     "minerals".into(),
                    base_price:        5.0,
                    demand_multiplier: 1.0,
                    supply_multiplier: 1.0,
                },
            ],
            global_inflation_rate: 0.02,
            economy_enabled:       true,
        }
    }

    /// Add a currency definition.
    pub fn add_currency(&mut self, def: CurrencyDefinition) {
        self.currencies.push(def);
    }

    /// Remove a currency by ID.  Returns `true` if found and removed.
    pub fn remove_currency(&mut self, id: &str) -> bool {
        let before = self.currencies.len();
        self.currencies.retain(|c| c.id != id);
        self.currencies.len() < before
    }

    /// Add a pricing rule.
    pub fn add_pricing_rule(&mut self, rule: EconomyPricingRule) {
        self.pricing_rules.push(rule);
    }

    /// Validate the panel data.  Returns a list of validation messages.
    pub fn validate(&self) -> Vec<String> {
        let mut msgs = Vec::new();
        for c in &self.currencies {
            if c.id.is_empty() {
                msgs.push("Currency has an empty id".into());
            }
            if c.earn_rate < 0.0 {
                msgs.push(format!("Currency '{}': earn_rate must be >= 0", c.id));
            }
        }
        if self.global_inflation_rate < 0.0 {
            msgs.push("global_inflation_rate must be >= 0".into());
        }
        msgs
    }
}

impl Default for EconomyPanel {
    fn default() -> Self { Self::new() }
}

impl IEditorPanel for EconomyPanel {
    fn panel_id(&self)    -> &str { Self::PANEL_ID }
    fn panel_title(&self) -> &str { "Economy" }
}

// ── Tests ────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn default_panel_has_two_currencies() {
        let p = EconomyPanel::new();
        assert_eq!(p.currencies.len(), 2);
        assert_eq!(p.currencies[0].id, "isk");
    }

    #[test]
    fn default_panel_has_three_pricing_rules() {
        let p = EconomyPanel::new();
        assert_eq!(p.pricing_rules.len(), 3);
    }

    #[test]
    fn economy_enabled_by_default() {
        let p = EconomyPanel::new();
        assert!(p.economy_enabled);
    }

    #[test]
    fn add_and_remove_currency() {
        let mut p = EconomyPanel::new();
        p.add_currency(CurrencyDefinition::new("gold", "Gold"));
        assert_eq!(p.currencies.len(), 3);
        assert!(p.remove_currency("gold"));
        assert_eq!(p.currencies.len(), 2);
    }

    #[test]
    fn remove_nonexistent_currency_returns_false() {
        let mut p = EconomyPanel::new();
        assert!(!p.remove_currency("no_such_currency"));
    }

    #[test]
    fn validate_clean_panel_returns_no_errors() {
        let p = EconomyPanel::new();
        assert!(p.validate().is_empty());
    }

    #[test]
    fn validate_empty_currency_id_is_error() {
        let mut p = EconomyPanel::new();
        p.currencies.push(CurrencyDefinition::new("", "Bad"));
        assert!(!p.validate().is_empty());
    }

    #[test]
    fn panel_id_and_title() {
        let p = EconomyPanel::new();
        assert_eq!(p.panel_id(), "nf.economy");
        assert_eq!(p.panel_title(), "Economy");
    }
}
