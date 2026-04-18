//! Mutable runtime world state.
//!
//! Holds the live game/simulation state layered on top of the procedurally-
//! generated universe — tracking player position, discovered systems,
//! applied delta edits, and AI-wallet balances.

use std::collections::{HashMap, HashSet};
use uuid::Uuid;
use atlas_ecs::DeltaEditStore;

/// Runtime world state for a single play session.
pub struct WorldState {
    /// Universe seed driving procedural generation.
    pub universe_seed: u64,
    /// UUIDs of systems the player has discovered / visited.
    pub discovered_systems: HashSet<Uuid>,
    /// Per-entity delta edits (layered on top of PCG).
    pub delta_edits: HashMap<Uuid, DeltaEditStore>,
    /// Tick count since session start.
    pub tick: u64,
    /// Simulated time elapsed (seconds).
    pub time_elapsed: f64,
}

impl WorldState {
    pub fn new(universe_seed: u64) -> Self {
        Self {
            universe_seed,
            discovered_systems: HashSet::new(),
            delta_edits:        HashMap::new(),
            tick:               0,
            time_elapsed:       0.0,
        }
    }

    /// Advance one tick by `delta_seconds`.
    pub fn advance(&mut self, delta_seconds: f64) {
        self.tick += 1;
        self.time_elapsed += delta_seconds;
    }

    /// Mark a system as discovered.
    pub fn discover(&mut self, system_id: Uuid) {
        self.discovered_systems.insert(system_id);
    }

    /// Returns `true` if the system has been visited.
    pub fn is_discovered(&self, system_id: &Uuid) -> bool {
        self.discovered_systems.contains(system_id)
    }

    /// Get or create the delta-edit store for an entity.
    pub fn edits_for(&mut self, entity_id: Uuid) -> &mut DeltaEditStore {
        self.delta_edits
            .entry(entity_id)
            .or_insert_with(|| DeltaEditStore::new(self.universe_seed))
    }

    /// Number of discovered systems.
    pub fn discovered_count(&self) -> usize {
        self.discovered_systems.len()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn advance_increments_tick() {
        let mut ws = WorldState::new(42);
        ws.advance(1.0 / 30.0);
        assert_eq!(ws.tick, 1);
    }

    #[test]
    fn discover_and_check() {
        let mut ws = WorldState::new(1);
        let id = Uuid::new_v4();
        assert!(!ws.is_discovered(&id));
        ws.discover(id);
        assert!(ws.is_discovered(&id));
    }

    #[test]
    fn advance_accumulates_time() {
        let mut ws = WorldState::new(0);
        ws.advance(0.5);
        ws.advance(0.5);
        assert!((ws.time_elapsed - 1.0).abs() < 1e-10);
        assert_eq!(ws.tick, 2);
    }

    #[test]
    fn discovered_count() {
        let mut ws = WorldState::new(0);
        assert_eq!(ws.discovered_count(), 0);
        ws.discover(Uuid::new_v4());
        ws.discover(Uuid::new_v4());
        assert_eq!(ws.discovered_count(), 2);
    }

    #[test]
    fn discover_same_id_twice_does_not_duplicate() {
        let mut ws = WorldState::new(0);
        let id = Uuid::new_v4();
        ws.discover(id);
        ws.discover(id);
        assert_eq!(ws.discovered_count(), 1);
    }

    #[test]
    fn edits_for_creates_store_on_demand() {
        let mut ws = WorldState::new(7);
        let id = Uuid::new_v4();
        let store = ws.edits_for(id);
        // Just verify we got a store back without panicking.
        drop(store);
        assert_eq!(ws.delta_edits.len(), 1);
    }

    #[test]
    fn edits_for_same_id_returns_same_store() {
        let mut ws = WorldState::new(7);
        let id = Uuid::new_v4();
        ws.edits_for(id);
        ws.edits_for(id);
        // Should still be exactly one entry.
        assert_eq!(ws.delta_edits.len(), 1);
    }

    #[test]
    fn universe_seed_stored() {
        let ws = WorldState::new(0xDEAD_BEEF);
        assert_eq!(ws.universe_seed, 0xDEAD_BEEF);
    }
}
