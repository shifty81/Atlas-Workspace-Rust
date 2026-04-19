// SPDX-License-Identifier: GPL-3.0-only
// NovaForge preview world — port of NovaForge::NovaForgePreviewWorld.
// Copyright (C) NovaForge contributors. GNU General Public License v3.0.

// ── EntityId ──────────────────────────────────────────────────────────────

pub type EntityId = u32;
pub const INVALID_ENTITY_ID: EntityId = 0;

// ── PreviewVec3 ───────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Default)]
pub struct PreviewVec3 {
    pub x: f32,
    pub y: f32,
    pub z: f32,
}

// ── PreviewTransform ──────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct PreviewTransform {
    pub position: PreviewVec3,
    pub rotation: PreviewVec3,
    pub scale:    PreviewVec3,
}

impl Default for PreviewTransform {
    fn default() -> Self {
        Self {
            position: PreviewVec3::default(),
            rotation: PreviewVec3::default(),
            scale:    PreviewVec3 { x: 1.0, y: 1.0, z: 1.0 },
        }
    }
}

// ── PreviewEntity ─────────────────────────────────────────────────────────

#[derive(Debug, Clone)]
pub struct PreviewEntity {
    pub id:           EntityId,
    pub name:         String,
    pub parent_id:    EntityId,
    pub transform:    PreviewTransform,
    pub mesh_tag:     String,
    pub material_tag: String,
    pub visible:      bool,
}

// ── NovaForgePreviewWorld ─────────────────────────────────────────────────

pub struct NovaForgePreviewWorld {
    entities:    Vec<PreviewEntity>,
    next_id:     EntityId,
    selected_id: EntityId,
    dirty:       bool,
}

impl NovaForgePreviewWorld {
    pub const MAX_ENTITIES: u32 = 512;

    pub fn new() -> Self {
        Self {
            entities:    Vec::new(),
            next_id:     1,
            selected_id: INVALID_ENTITY_ID,
            dirty:       false,
        }
    }

    // ── Entity lifecycle ──────────────────────────────────────────────────

    pub fn create_entity(&mut self, name: impl Into<String>, parent_id: EntityId) -> EntityId {
        if self.entities.len() >= Self::MAX_ENTITIES as usize {
            return INVALID_ENTITY_ID;
        }
        let id = self.next_id;
        self.next_id += 1;
        self.entities.push(PreviewEntity {
            id,
            name: name.into(),
            parent_id,
            transform: PreviewTransform::default(),
            mesh_tag: String::new(),
            material_tag: String::new(),
            visible: true,
        });
        self.dirty = true;
        id
    }

    pub fn destroy_entity(&mut self, id: EntityId) -> bool {
        if let Some(pos) = self.entities.iter().position(|e| e.id == id) {
            if self.selected_id == id {
                self.selected_id = INVALID_ENTITY_ID;
            }
            self.entities.remove(pos);
            self.dirty = true;
            true
        } else {
            false
        }
    }

    pub fn find_entity(&self, id: EntityId) -> Option<&PreviewEntity> {
        self.entities.iter().find(|e| e.id == id)
    }

    pub fn find_entity_mut(&mut self, id: EntityId) -> Option<&mut PreviewEntity> {
        self.entities.iter_mut().find(|e| e.id == id)
    }

    pub fn entity_count(&self) -> u32 { self.entities.len() as u32 }

    // ── Transform ─────────────────────────────────────────────────────────

    pub fn set_transform(&mut self, id: EntityId, transform: PreviewTransform) -> bool {
        if let Some(e) = self.find_entity_mut(id) {
            e.transform = transform;
            self.dirty = true;
            true
        } else {
            false
        }
    }

    // ── Mesh / Material / Visibility ──────────────────────────────────────

    pub fn set_mesh_tag(&mut self, id: EntityId, tag: impl Into<String>) -> bool {
        if let Some(e) = self.find_entity_mut(id) {
            e.mesh_tag = tag.into();
            self.dirty = true;
            true
        } else {
            false
        }
    }

    pub fn set_material_tag(&mut self, id: EntityId, tag: impl Into<String>) -> bool {
        if let Some(e) = self.find_entity_mut(id) {
            e.material_tag = tag.into();
            self.dirty = true;
            true
        } else {
            false
        }
    }

    pub fn set_visibility(&mut self, id: EntityId, visible: bool) -> bool {
        if let Some(e) = self.find_entity_mut(id) {
            e.visible = visible;
            self.dirty = true;
            true
        } else {
            false
        }
    }

    // ── Selection ─────────────────────────────────────────────────────────

    pub fn select_entity(&mut self, id: EntityId) -> bool {
        if id == INVALID_ENTITY_ID || self.find_entity(id).is_some() {
            self.selected_id = id;
            true
        } else {
            false
        }
    }

    pub fn deselect(&mut self) { self.selected_id = INVALID_ENTITY_ID; }
    pub fn selected_id(&self) -> EntityId { self.selected_id }

    // ── Dirty tracking ────────────────────────────────────────────────────

    pub fn is_dirty(&self)  -> bool { self.dirty }
    pub fn clear_dirty(&mut self)   { self.dirty = false; }

    /// Remove all entities and clear selection.
    pub fn clear(&mut self) {
        self.entities.clear();
        self.selected_id = INVALID_ENTITY_ID;
        self.dirty = true;
    }
}

impl Default for NovaForgePreviewWorld {
    fn default() -> Self { Self::new() }
}

// ── Tests ─────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn create_entity_returns_valid_id() {
        let mut world = NovaForgePreviewWorld::new();
        let id = world.create_entity("Foo", INVALID_ENTITY_ID);
        assert_ne!(id, INVALID_ENTITY_ID);
        assert_eq!(world.entity_count(), 1);
    }

    #[test]
    fn create_at_cap_returns_invalid() {
        let mut world = NovaForgePreviewWorld::new();
        for i in 0..NovaForgePreviewWorld::MAX_ENTITIES {
            world.create_entity(format!("e{}", i), INVALID_ENTITY_ID);
        }
        let id = world.create_entity("overflow", INVALID_ENTITY_ID);
        assert_eq!(id, INVALID_ENTITY_ID);
    }

    #[test]
    fn destroy_entity() {
        let mut world = NovaForgePreviewWorld::new();
        let id = world.create_entity("A", INVALID_ENTITY_ID);
        assert!(world.destroy_entity(id));
        assert_eq!(world.entity_count(), 0);
        assert!(!world.destroy_entity(id));
    }

    #[test]
    fn set_transform() {
        let mut world = NovaForgePreviewWorld::new();
        let id = world.create_entity("B", INVALID_ENTITY_ID);
        let t = PreviewTransform {
            position: PreviewVec3 { x: 1.0, y: 2.0, z: 3.0 },
            ..Default::default()
        };
        assert!(world.set_transform(id, t));
        assert_eq!(world.find_entity(id).unwrap().transform.position.x, 1.0);
    }

    #[test]
    fn mesh_and_material_tags() {
        let mut world = NovaForgePreviewWorld::new();
        let id = world.create_entity("C", INVALID_ENTITY_ID);
        assert!(world.set_mesh_tag(id, "mesh/tree"));
        assert!(world.set_material_tag(id, "mat/bark"));
        let e = world.find_entity(id).unwrap();
        assert_eq!(e.mesh_tag, "mesh/tree");
        assert_eq!(e.material_tag, "mat/bark");
    }

    #[test]
    fn select_and_deselect() {
        let mut world = NovaForgePreviewWorld::new();
        let id = world.create_entity("D", INVALID_ENTITY_ID);
        assert!(world.select_entity(id));
        assert_eq!(world.selected_id(), id);
        world.deselect();
        assert_eq!(world.selected_id(), INVALID_ENTITY_ID);
    }

    #[test]
    fn select_nonexistent_returns_false() {
        let mut world = NovaForgePreviewWorld::new();
        assert!(!world.select_entity(999));
    }

    #[test]
    fn clear_removes_all_entities() {
        let mut world = NovaForgePreviewWorld::new();
        world.create_entity("E", INVALID_ENTITY_ID);
        world.create_entity("F", INVALID_ENTITY_ID);
        world.clear();
        assert_eq!(world.entity_count(), 0);
    }

    #[test]
    fn dirty_tracking() {
        let mut world = NovaForgePreviewWorld::new();
        assert!(!world.is_dirty());
        world.create_entity("G", INVALID_ENTITY_ID);
        assert!(world.is_dirty());
        world.clear_dirty();
        assert!(!world.is_dirty());
    }
}
