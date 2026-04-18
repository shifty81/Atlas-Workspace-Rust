//! # Atlas ECS
//!
//! Entity–Component–System framework for the Atlas Workspace.
//!
//! ## Overview
//!
//! | Concept | Type |
//! |---------|------|
//! | Entity  | [`EntityId`] — a lightweight 32-bit handle |
//! | Components | [`ComponentStore`] — typed sparse storage |
//! | Systems | [`SystemBase`] / [`SystemRegistry`] |
//! | World   | [`World`] — top-level container |
//! | Scene graph | [`SceneGraph`] — hierarchical parent/child |
//! | Delta edits | [`DeltaEditStore`] — PCG-overlay edit log |

pub mod component;
pub mod components;
pub mod delta;
pub mod entity;
pub mod scene_graph;
pub mod system;
pub mod world;

pub use component::ComponentStore;
pub use components::Name;
pub use delta::{DeltaEdit, DeltaEditStore, DeltaEditType};
pub use entity::{EntityId, EntityManager, INVALID_ENTITY};
pub use scene_graph::SceneGraph;
pub use system::{SystemBase, SystemRegistry};
pub use world::World;
