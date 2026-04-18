//! # Atlas World
//!
//! Universe-scale world and asset generation for the Atlas Workspace.
//!
//! Builds on [`atlas_pcg`] to produce fully-populated universe hierarchies:
//! galaxies, star systems, planets, asteroids, stations, and more — all
//! driven from a single universe seed.
//!
//! ## Modules
//!
//! | Module | Purpose |
//! |--------|---------|
//! | [`universe`] | Top-level universe container |
//! | [`galaxy`] | Galaxy layout (spiral arms, star density) |
//! | [`star_system`] | Star + planet system generation |
//! | [`planet`] | Planet surface, atmosphere, biome |
//! | [`asteroid`] | Asteroid belt / field generation |
//! | [`asset_registry`] | Universe asset catalogue |
//! | [`world_state`] | Mutable runtime world state |

pub mod asset_registry;
pub mod asteroid;
pub mod galaxy;
pub mod planet;
pub mod star_system;
pub mod universe;
pub mod world_state;

pub use asset_registry::{AssetEntry, AssetRegistry, AssetType};
pub use asteroid::{Asteroid, AsteroidBelt, AsteroidConfig};
pub use galaxy::{Galaxy, GalaxyConfig, StarCluster};
pub use planet::{Atmosphere, Biome, Planet, PlanetConfig, PlanetType};
pub use star_system::{StarSystem, StarSystemConfig, StarType};
pub use universe::{Universe, UniverseConfig};
pub use world_state::WorldState;
