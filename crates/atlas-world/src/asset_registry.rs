//! Universe asset catalogue.
//!
//! The [`AssetRegistry`] tracks every procedurally-generated object (planets,
//! asteroids, stations, etc.) with a globally unique UUID.

use std::collections::HashMap;
use uuid::Uuid;
use serde::{Deserialize, Serialize};

/// Type of universe asset.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum AssetType {
    Planet,
    Asteroid,
    Station,
    Ship,
    Galaxy,
    StarSystem,
    NebulaPatch,
    AnomalyZone,
}

/// A single registered universe asset.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct AssetEntry {
    pub id:         Uuid,
    pub asset_type: AssetType,
    pub name:       String,
    pub seed:       u64,
    pub tags:       Vec<String>,
}

impl AssetEntry {
    pub fn new(asset_type: AssetType, name: impl Into<String>, seed: u64) -> Self {
        Self {
            id:         Uuid::new_v4(),
            asset_type,
            name:       name.into(),
            seed,
            tags:       Vec::new(),
        }
    }

    pub fn with_tag(mut self, tag: impl Into<String>) -> Self {
        self.tags.push(tag.into());
        self
    }
}

/// Registry of all universe assets.
#[derive(Default, Debug, Serialize, Deserialize)]
pub struct AssetRegistry {
    assets: HashMap<Uuid, AssetEntry>,
}

impl AssetRegistry {
    pub fn new() -> Self { Self::default() }

    /// Register an asset; returns its assigned UUID.
    pub fn register(&mut self, entry: AssetEntry) -> Uuid {
        let id = entry.id;
        self.assets.insert(id, entry);
        id
    }

    /// Look up an asset by UUID.
    pub fn get(&self, id: &Uuid) -> Option<&AssetEntry> {
        self.assets.get(id)
    }

    /// All assets of a given type.
    pub fn by_type(&self, asset_type: AssetType) -> Vec<&AssetEntry> {
        self.assets.values()
            .filter(|e| e.asset_type == asset_type)
            .collect()
    }

    /// Total number of registered assets.
    pub fn count(&self) -> usize { self.assets.len() }

    /// Serialize to JSON.
    pub fn to_json(&self) -> Result<String, serde_json::Error> {
        serde_json::to_string_pretty(self)
    }

    /// Deserialize from JSON.
    pub fn from_json(json: &str) -> Result<Self, serde_json::Error> {
        serde_json::from_str(json)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn register_and_lookup() {
        let mut reg = AssetRegistry::new();
        let entry = AssetEntry::new(AssetType::Planet, "Kepler-7b", 12345);
        let id = reg.register(entry);
        let found = reg.get(&id).unwrap();
        assert_eq!(found.name, "Kepler-7b");
        assert_eq!(found.asset_type, AssetType::Planet);
    }

    #[test]
    fn by_type_filter() {
        let mut reg = AssetRegistry::new();
        reg.register(AssetEntry::new(AssetType::Planet, "A", 1));
        reg.register(AssetEntry::new(AssetType::Planet, "B", 2));
        reg.register(AssetEntry::new(AssetType::Station, "C", 3));
        assert_eq!(reg.by_type(AssetType::Planet).len(), 2);
        assert_eq!(reg.by_type(AssetType::Station).len(), 1);
    }

    #[test]
    fn json_round_trip() {
        let mut reg = AssetRegistry::new();
        reg.register(AssetEntry::new(AssetType::Asteroid, "Rock-1", 99));
        let json = reg.to_json().unwrap();
        let reg2 = AssetRegistry::from_json(&json).unwrap();
        assert_eq!(reg.count(), reg2.count());
    }
}
