//! # Atlas PCG
//!
//! Procedural Content Generation framework for the Atlas Workspace.
//!
//! ## Architecture
//!
//! All procedural generation flows through a single seed authority:
//! [`PcgManager`].  Each of the 16 isolated [`PcgDomain`]s receives its own
//! deterministic RNG stream derived from the universe seed.
//!
//! ### Modules
//!
//! | Module | Purpose |
//! |--------|---------|
//! | [`rng`] | Deterministic xorshift64* RNG |
//! | [`domain`] | PCG domain enum and seed hierarchy |
//! | [`manager`] | Universe-seed authority |
//! | [`constraint`] | GA-based constraint / fitting solver |
//! | [`mesh_graph`] | Procedural mesh node graph |
//! | [`material_graph`] | Procedural material node graph |
//! | [`planetary`] | Planetary-base zone layout |
//! | [`build_queue`] | Timed build / upgrade queue |
//! | [`terrain`] | Heightmap / terrain generation |
//! | [`noise_util`] | Noise helpers (fbm, ridged, etc.) |
//! | [`lod`] | LOD baking graph |

pub mod build_queue;
pub mod constraint;
pub mod domain;
pub mod lod;
pub mod manager;
pub mod material_graph;
pub mod mesh_graph;
pub mod noise_util;
pub mod planetary;
pub mod rng;
pub mod terrain;

pub use build_queue::{BuildOrder, BuildOrderType, BuildQueue};
pub use constraint::{ConstraintConfig, ConstraintResult, ConstraintSolver, FitItem};
pub use domain::{PcgContext, PcgDomain, SeedLevel};
pub use lod::{LodBakingGraph, LodNode};
pub use manager::PcgManager;
pub use material_graph::{MaterialGraph, MaterialNode, MaterialNodeType};
pub use mesh_graph::{MeshData, MeshEdge, MeshGraph, MeshNode, MeshNodeType};
pub use noise_util::{fbm, ridged_multifractal};
pub use planetary::{BaseZone, BaseZoneType, PlanetaryBase, PlanetaryBaseConfig};
pub use rng::DeterministicRng;
pub use terrain::{HeightMap, TerrainConfig, TerrainGenerator};
