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
