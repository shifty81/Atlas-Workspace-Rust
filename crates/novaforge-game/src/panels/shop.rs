//! [`ShopPanel`] — editor panel for NovaForge market / shop authoring.
//!
//! Mirrors the C++ `NovaForge::ShopPanel`.
//!
//! # License
//!
//! GPL v3.0 — this file is part of `novaforge-game`.

use atlas_editor::IEditorPanel;
use serde::{Deserialize, Serialize};

// ── Data types ────────────────────────────────────────────────────────────────

/// A single item listed for sale in a shop.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ShopListing {
    /// Item identifier (e.g. `"frigate_hull"`).
    pub item_id:           String,
    /// Currency used for this listing (e.g. `"isk"`).
    pub currency_id:       String,
    /// Base price in the given currency.
    pub base_price:        f32,
    /// Stock limit.  `0` means unlimited.
    pub stock_limit:       u32,
    /// When `true`, the player must meet a condition to see this listing.
    pub requires_unlock:   bool,
    /// Condition expression (evaluated when `requires_unlock` is `true`).
    pub unlock_condition:  String,
}

/// Configuration for a single shop / market location.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ShopConfig {
    /// Stable shop identifier.
    pub shop_id:            String,
    /// Display name shown in the UI.
    pub display_name:       String,
    /// Faction that operates this shop.
    pub merchant_faction:   String,
    /// Whether the shop is available everywhere (not tied to a location).
    pub globally_available: bool,
    /// Location string (station/region) — empty for global shops.
    pub location:           String,
    /// Items available in this shop.
    pub listings:           Vec<ShopListing>,
}

impl ShopConfig {
    /// Create an empty shop.
    pub fn new(
        shop_id: impl Into<String>,
        display_name: impl Into<String>,
        faction: impl Into<String>,
    ) -> Self {
        Self {
            shop_id:            shop_id.into(),
            display_name:       display_name.into(),
            merchant_faction:   faction.into(),
            globally_available: false,
            location:           String::new(),
            listings:           Vec::new(),
        }
    }
}

// ── ShopPanel ─────────────────────────────────────────────────────────────────

/// Editor panel for authoring NovaForge shops and market listings.
pub struct ShopPanel {
    /// All configured shops.
    pub shops: Vec<ShopConfig>,
}

impl ShopPanel {
    pub const PANEL_ID: &'static str = "nf.shop";

    /// Create the panel with default NovaForge trade hub shops.
    pub fn new() -> Self {
        let mut amarr = ShopConfig::new("amarr_trade_hub", "Amarr Trade Hub", "Amarr Empire");
        amarr.globally_available = false;
        amarr.location = "Amarr VII".into();
        amarr.listings.push(ShopListing {
            item_id:          "frigate_hull".into(),
            currency_id:      "isk".into(),
            base_price:       360_000.0,
            stock_limit:      0,
            requires_unlock:  false,
            unlock_condition: String::new(),
        });

        let mut concord = ShopConfig::new("concord_rewards", "CONCORD Rewards Store", "CONCORD");
        concord.globally_available = true;
        concord.listings.push(ShopListing {
            item_id:          "concord_implant".into(),
            currency_id:      "loyalty_points".into(),
            base_price:       50_000.0,
            stock_limit:      1,
            requires_unlock:  true,
            unlock_condition: "standing.concord >= 5.0".into(),
        });

        Self { shops: vec![amarr, concord] }
    }

    /// Add a shop.
    pub fn add_shop(&mut self, shop: ShopConfig) {
        self.shops.push(shop);
    }

    /// Total number of listings across all shops.
    pub fn total_listing_count(&self) -> usize {
        self.shops.iter().map(|s| s.listings.len()).sum()
    }

    /// Validate shop configuration.
    pub fn validate(&self) -> Vec<String> {
        let mut msgs = Vec::new();
        for shop in &self.shops {
            if shop.shop_id.is_empty() {
                msgs.push("Shop has empty shop_id".into());
            }
            for listing in &shop.listings {
                if listing.base_price < 0.0 {
                    msgs.push(format!(
                        "Shop '{}' listing '{}': base_price must be >= 0",
                        shop.shop_id, listing.item_id
                    ));
                }
            }
        }
        msgs
    }
}

impl Default for ShopPanel {
    fn default() -> Self { Self::new() }
}

impl IEditorPanel for ShopPanel {
    fn panel_id(&self)    -> &str { Self::PANEL_ID }
    fn panel_title(&self) -> &str { "Shop Editor" }
}

// ── Tests ────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn default_panel_has_shops() {
        let p = ShopPanel::new();
        assert!(!p.shops.is_empty());
    }

    #[test]
    fn default_total_listings() {
        let p = ShopPanel::new();
        assert!(p.total_listing_count() > 0);
    }

    #[test]
    fn add_shop_increases_count() {
        let mut p = ShopPanel::new();
        let before = p.shops.len();
        p.add_shop(ShopConfig::new("new_shop", "New Shop", "Minmatar"));
        assert_eq!(p.shops.len(), before + 1);
    }

    #[test]
    fn validate_clean_panel_returns_no_errors() {
        let p = ShopPanel::new();
        assert!(p.validate().is_empty());
    }

    #[test]
    fn validate_empty_shop_id_is_error() {
        let mut p = ShopPanel::new();
        p.shops.push(ShopConfig::new("", "Bad", "Faction"));
        assert!(!p.validate().is_empty());
    }

    #[test]
    fn panel_id_and_title() {
        let p = ShopPanel::new();
        assert_eq!(p.panel_id(), "nf.shop");
        assert_eq!(p.panel_title(), "Shop Editor");
    }
}
