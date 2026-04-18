//! Built-in [`EditorCommand`] implementations for entity lifecycle operations.

use atlas_ecs::{EntityId, World};
use atlas_math::Transform;

use crate::command::EditorCommand;

// ── SpawnEntityCommand ────────────────────────────────────────────────────────

/// Spawn a new entity with a default [`Transform`] component.
pub struct SpawnEntityCommand {
    /// Filled in when the command is first applied; used for undo.
    spawned_id: Option<EntityId>,
}

impl SpawnEntityCommand {
    pub fn new() -> Self {
        Self { spawned_id: None }
    }
}

impl EditorCommand for SpawnEntityCommand {
    fn description(&self) -> &str { "Spawn Entity" }

    fn apply(&mut self, world: &mut World) {
        let id = world.spawn();
        world.components.add(id, Transform::default());
        self.spawned_id = Some(id);
        log::info!("[Command] Spawned entity #{id}");
    }

    fn revert(&mut self, world: &mut World) {
        if let Some(id) = self.spawned_id.take() {
            world.despawn(id);
            log::info!("[Command] Reverted spawn of entity #{id}");
        }
    }
}

impl Default for SpawnEntityCommand {
    fn default() -> Self { Self::new() }
}

// ── DeleteEntityCommand ───────────────────────────────────────────────────────

/// Delete an entity (and optionally restore it on undo).
pub struct DeleteEntityCommand {
    entity:          EntityId,
    /// Saved transform for undo restoral.
    saved_transform: Option<Transform>,
    /// Whether the entity was alive when `apply` was called.
    was_alive:       bool,
}

impl DeleteEntityCommand {
    pub fn new(entity: EntityId) -> Self {
        Self { entity, saved_transform: None, was_alive: false }
    }
}

impl EditorCommand for DeleteEntityCommand {
    fn description(&self) -> &str { "Delete Entity" }

    fn apply(&mut self, world: &mut World) {
        if !world.entities.is_alive(self.entity) { return; }
        // Save transform so we can partially restore it on undo
        self.saved_transform = world.components.get::<Transform>(self.entity).copied();
        self.was_alive = true;
        world.despawn(self.entity);
        log::info!("[Command] Deleted entity #{}", self.entity);
    }

    fn revert(&mut self, world: &mut World) {
        if !self.was_alive { return; }
        // Spawn a fresh entity. Note: the new ID will differ from the original,
        // so callers should clear selection state after undo.
        let id = world.spawn();
        if let Some(t) = self.saved_transform {
            world.components.add(id, t);
        }
        log::info!("[Command] Restored deleted entity as new entity #{id}");
        self.was_alive = false;
    }
}
