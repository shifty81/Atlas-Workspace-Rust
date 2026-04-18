//! Typed, sparse component storage.
//!
//! Implements the same API as the C++ `NF::ComponentStore` via Rust's
//! `TypeId`-keyed `HashMap` of erased trait objects.

use std::any::{Any, TypeId};
use std::collections::HashMap;

use crate::EntityId;

type ComponentMap = HashMap<EntityId, Box<dyn Any + Send + Sync>>;

/// Heterogeneous, type-safe storage for entity components.
#[derive(Default)]
pub struct ComponentStore {
    stores: HashMap<TypeId, ComponentMap>,
}

impl ComponentStore {
    pub fn new() -> Self {
        Self { stores: HashMap::new() }
    }

    /// Insert a component of type `C` for `entity`.  Overwrites any previous value.
    pub fn add<C: 'static + Send + Sync>(&mut self, entity: EntityId, component: C) {
        self.stores
            .entry(TypeId::of::<C>())
            .or_default()
            .insert(entity, Box::new(component));
    }

    /// Returns a shared reference to the component, or `None`.
    pub fn get<C: 'static + Send + Sync>(&self, entity: EntityId) -> Option<&C> {
        self.stores
            .get(&TypeId::of::<C>())?
            .get(&entity)?
            .downcast_ref::<C>()
    }

    /// Returns an exclusive reference to the component, or `None`.
    pub fn get_mut<C: 'static + Send + Sync>(&mut self, entity: EntityId) -> Option<&mut C> {
        self.stores
            .get_mut(&TypeId::of::<C>())?
            .get_mut(&entity)?
            .downcast_mut::<C>()
    }

    /// Returns `true` if `entity` has a component of type `C`.
    pub fn has<C: 'static + Send + Sync>(&self, entity: EntityId) -> bool {
        self.stores
            .get(&TypeId::of::<C>())
            .is_some_and(|m| m.contains_key(&entity))
    }

    /// Remove the component of type `C` from `entity`.
    pub fn remove<C: 'static + Send + Sync>(&mut self, entity: EntityId) {
        if let Some(map) = self.stores.get_mut(&TypeId::of::<C>()) {
            map.remove(&entity);
        }
    }

    /// Remove all components for `entity` across every type.
    pub fn remove_all(&mut self, entity: EntityId) {
        for map in self.stores.values_mut() {
            map.remove(&entity);
        }
    }

    /// Returns all components of type `C` as a `HashMap<EntityId, C>`.
    pub fn get_all<C: 'static + Send + Sync>(&self) -> HashMap<EntityId, &C> {
        self.stores
            .get(&TypeId::of::<C>())
            .map(|map| {
                map.iter()
                    .filter_map(|(&id, v)| v.downcast_ref::<C>().map(|c| (id, c)))
                    .collect()
            })
            .unwrap_or_default()
    }
}
