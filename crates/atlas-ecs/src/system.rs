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

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{EntityId, EntityManager, ComponentStore};
    use std::sync::{Arc, Mutex};

    struct CounterSystem {
        name: &'static str,
        updates: Arc<Mutex<u32>>,
        shutdowns: Arc<Mutex<u32>>,
    }

    impl SystemBase for CounterSystem {
        fn name(&self) -> &str { self.name }
        fn update(&mut self, _dt: f32) {
            *self.updates.lock().unwrap() += 1;
        }
        fn shutdown(&mut self) {
            *self.shutdowns.lock().unwrap() += 1;
        }
    }

    #[test]
    fn empty_registry_has_zero_systems() {
        let reg = SystemRegistry::new();
        assert_eq!(reg.count(), 0);
    }

    #[test]
    fn add_boxed_increments_count() {
        let mut reg = SystemRegistry::new();
        let updates = Arc::new(Mutex::new(0u32));
        reg.add_boxed(Box::new(CounterSystem { name: "A", updates: updates.clone(), shutdowns: Arc::new(Mutex::new(0)) }));
        reg.add_boxed(Box::new(CounterSystem { name: "B", updates: updates.clone(), shutdowns: Arc::new(Mutex::new(0)) }));
        assert_eq!(reg.count(), 2);
    }

    #[test]
    fn update_all_calls_each_system() {
        let mut reg = SystemRegistry::new();
        let updates = Arc::new(Mutex::new(0u32));
        for name in ["A", "B", "C"] {
            reg.add_boxed(Box::new(CounterSystem {
                name,
                updates: updates.clone(),
                shutdowns: Arc::new(Mutex::new(0)),
            }));
        }
        reg.update_all(0.016);
        assert_eq!(*updates.lock().unwrap(), 3);
    }

    #[test]
    fn shutdown_all_calls_each_system() {
        let mut reg = SystemRegistry::new();
        let shutdowns = Arc::new(Mutex::new(0u32));
        for name in ["X", "Y"] {
            reg.add_boxed(Box::new(CounterSystem {
                name,
                updates: Arc::new(Mutex::new(0)),
                shutdowns: shutdowns.clone(),
            }));
        }
        reg.shutdown_all();
        assert_eq!(*shutdowns.lock().unwrap(), 2);
    }

    #[test]
    fn init_all_does_not_panic() {
        let mut reg = SystemRegistry::new();
        reg.add_boxed(Box::new(CounterSystem {
            name: "init",
            updates: Arc::new(Mutex::new(0)),
            shutdowns: Arc::new(Mutex::new(0)),
        }));
        let mut em = EntityManager::new();
        let mut cs = ComponentStore::new();
        reg.init_all(&mut em, &mut cs); // default init() is a no-op, should not panic
    }
}
