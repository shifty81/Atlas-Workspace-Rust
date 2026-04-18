//! System trait and registry.
//!
//! Mirrors the C++ `NF::SystemBase` / `NF::SystemRegistry`.

use crate::entity::EntityManager;
use crate::component::ComponentStore;

/// Trait implemented by every simulation system.
pub trait SystemBase: Send + Sync {
    fn name(&self) -> &str;
    fn init(&mut self, _em: &mut EntityManager, _cs: &mut ComponentStore) {}
    fn update(&mut self, dt: f32);
    fn shutdown(&mut self) {}
}

/// Ordered collection of systems.
#[derive(Default)]
pub struct SystemRegistry {
    systems: Vec<Box<dyn SystemBase>>,
}

impl SystemRegistry {
    pub fn new() -> Self {
        Self::default()
    }

    /// Add a boxed system.
    pub fn add_boxed(&mut self, system: Box<dyn SystemBase>) {
        self.systems.push(system);
    }

    /// Number of registered systems.
    pub fn count(&self) -> usize {
        self.systems.len()
    }

    /// Initialise all systems in registration order.
    pub fn init_all(&mut self, em: &mut EntityManager, cs: &mut ComponentStore) {
        for sys in &mut self.systems {
            sys.init(em, cs);
        }
    }

    /// Update all systems in registration order.
    pub fn update_all(&mut self, dt: f32) {
        for sys in &mut self.systems {
            sys.update(dt);
        }
    }

    /// Shut down all systems in reverse registration order.
    pub fn shutdown_all(&mut self) {
        for sys in self.systems.iter_mut().rev() {
            sys.shutdown();
        }
    }
}
