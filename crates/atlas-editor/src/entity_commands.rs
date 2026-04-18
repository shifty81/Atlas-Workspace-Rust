//! Built-in [`EditorCommand`] implementations for entity lifecycle operations.

use atlas_ecs::{EntityId, Name, World};
use atlas_math::Transform;

use crate::command::EditorCommand;

// ── SpawnEntityCommand ────────────────────────────────────────────────────────

/// Spawn a new entity with a default [`Name`] and [`Transform`].
pub struct SpawnEntityCommand {
    /// Filled in when the command is first applied; used for undo.
    spawned_id: Option<EntityId>,
    /// Optional label; defaults to "Entity".
    label:      String,
}

impl SpawnEntityCommand {
    pub fn new() -> Self {
        Self { spawned_id: None, label: "Entity".into() }
    }

    pub fn with_name(name: impl Into<String>) -> Self {
        Self { spawned_id: None, label: name.into() }
    }
}

impl EditorCommand for SpawnEntityCommand {
    fn description(&self) -> &str { "Spawn Entity" }

    fn apply(&mut self, world: &mut World) {
        let id = world.spawn();
        world.components.add(id, Name::new(&self.label));
        world.components.add(id, Transform::default());
        self.spawned_id = Some(id);
        log::info!("[Command] Spawned entity #{id} '{}'", self.label);
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

// ── RenameEntityCommand ───────────────────────────────────────────────────────

/// Rename an entity's [`Name`] component (undoable).
pub struct RenameEntityCommand {
    entity:   EntityId,
    new_name: String,
    old_name: Option<String>,
}

impl RenameEntityCommand {
    pub fn new(entity: EntityId, new_name: impl Into<String>) -> Self {
        Self { entity, new_name: new_name.into(), old_name: None }
    }
}

impl EditorCommand for RenameEntityCommand {
    fn description(&self) -> &str { "Rename Entity" }

    fn apply(&mut self, world: &mut World) {
        self.old_name = world.components.get::<Name>(self.entity).map(|n| n.0.clone());
        world.components.add(self.entity, Name::new(&self.new_name));
        log::info!("[Command] Renamed entity #{} → '{}'", self.entity, self.new_name);
    }

    fn revert(&mut self, world: &mut World) {
        match &self.old_name {
            Some(n) => { world.components.add(self.entity, Name::new(n.as_str())); }
            None    => { world.components.remove::<Name>(self.entity); }
        }
        log::info!("[Command] Reverted rename of entity #{}", self.entity);
    }
}


/// Delete an entity (and optionally restore it on undo).
pub struct DeleteEntityCommand {
    entity:          EntityId,
    /// Saved name for undo restoral.
    saved_name:      Option<Name>,
    /// Saved transform for undo restoral.
    saved_transform: Option<Transform>,
    /// Whether the entity was alive when `apply` was called.
    was_alive:       bool,
}

impl DeleteEntityCommand {
    pub fn new(entity: EntityId) -> Self {
        Self { entity, saved_name: None, saved_transform: None, was_alive: false }
    }
}

impl EditorCommand for DeleteEntityCommand {
    fn description(&self) -> &str { "Delete Entity" }

    fn apply(&mut self, world: &mut World) {
        if !world.entities.is_alive(self.entity) { return; }
        self.saved_name      = world.components.get::<Name>(self.entity).cloned();
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
        if let Some(n) = self.saved_name.take() { world.components.add(id, n); }
        if let Some(t) = self.saved_transform { world.components.add(id, t); }
        log::info!("[Command] Restored deleted entity as new entity #{id}");
        self.was_alive = false;
    }
}
