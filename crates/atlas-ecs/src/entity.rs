//! Entity identity and management.
//!
//! Entities are lightweight 32-bit handles.  The [`EntityManager`] hands them
//! out, recycles them via a free-list, and tracks which are alive.

/// A unique entity identifier.  The value `0` is reserved as [`INVALID_ENTITY`].
pub type EntityId = u32;

/// Sentinel value for a null / invalid entity reference.
pub const INVALID_ENTITY: EntityId = 0;

/// Allocates, recycles, and tracks entity identifiers.
#[derive(Default)]
pub struct EntityManager {
    next_id:   EntityId,
    alive:     Vec<EntityId>,
    free_list: Vec<EntityId>,
}

impl EntityManager {
    pub fn new() -> Self {
        Self {
            next_id:   1, // 0 is reserved
            alive:     Vec::new(),
            free_list: Vec::new(),
        }
    }

    /// Create a new entity, reusing a recycled ID if available.
    pub fn create_entity(&mut self) -> EntityId {
        let id = if let Some(recycled) = self.free_list.pop() {
            recycled
        } else {
            let id = self.next_id;
            self.next_id += 1;
            id
        };
        self.alive.push(id);
        id
    }

    /// Destroy an entity, returning its ID to the free-list.
    pub fn destroy_entity(&mut self, id: EntityId) {
        if let Some(pos) = self.alive.iter().position(|&e| e == id) {
            self.alive.swap_remove(pos);
            self.free_list.push(id);
        }
    }

    /// Returns `true` if the entity is currently alive.
    pub fn is_alive(&self, id: EntityId) -> bool {
        self.alive.contains(&id)
    }

    /// Number of live entities.
    pub fn count(&self) -> usize {
        self.alive.len()
    }

    /// Slice of all live entity IDs.
    pub fn alive(&self) -> &[EntityId] {
        &self.alive
    }

    /// Destroy every entity and reset internal state.
    pub fn clear(&mut self) {
        self.alive.clear();
        self.free_list.clear();
        self.next_id = 1;
    }
}
