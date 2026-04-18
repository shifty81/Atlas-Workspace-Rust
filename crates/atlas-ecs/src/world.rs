//! Top-level ECS world container.

use crate::entity::{EntityId, EntityManager};
use crate::component::ComponentStore;
use crate::scene_graph::SceneGraph;
use crate::system::SystemRegistry;

/// The ECS world: owns entities, components, the scene graph, and systems.
pub struct World {
    pub entities:   EntityManager,
    pub components: ComponentStore,
    pub graph:      SceneGraph,
    pub systems:    SystemRegistry,
}

impl World {
    pub fn new() -> Self {
        Self {
            entities:   EntityManager::new(),
            components: ComponentStore::new(),
            graph:      SceneGraph::new(),
            systems:    SystemRegistry::new(),
        }
    }

    /// Spawn a new entity.
    pub fn spawn(&mut self) -> EntityId {
        self.entities.create_entity()
    }

    /// Despawn an entity, removing all its components and scene-graph links.
    pub fn despawn(&mut self, id: EntityId) {
        self.components.remove_all(id);
        self.graph.remove_entity(id);
        self.entities.destroy_entity(id);
    }

    /// Tick all systems forward by `dt` seconds.
    pub fn update(&mut self, dt: f32) {
        self.systems.update_all(dt);
    }
}

impl Default for World {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[derive(Clone, PartialEq, Debug)]
    struct Tag(u32);

    #[test]
    fn spawn_and_despawn() {
        let mut w = World::new();
        let e = w.spawn();
        assert!(w.entities.is_alive(e));
        w.despawn(e);
        assert!(!w.entities.is_alive(e));
    }

    #[test]
    fn add_and_get_component() {
        let mut w = World::new();
        let e = w.spawn();
        w.components.add(e, Tag(42));
        assert_eq!(w.components.get::<Tag>(e).unwrap().0, 42);
    }

    #[test]
    fn despawn_removes_components() {
        let mut w = World::new();
        let e = w.spawn();
        w.components.add(e, Tag(1));
        w.despawn(e);
        assert!(w.components.get::<Tag>(e).is_none());
    }

    #[test]
    fn default_world_is_empty() {
        let w = World::default();
        assert_eq!(w.entities.count(), 0);
    }
}
