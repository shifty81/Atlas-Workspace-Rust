//! # NovaForge Game
//!
//! **License: GNU General Public License v3.0**
//!
//! This crate is the Rust port of [Nova-Forge](https://github.com/shifty81/Nova-Forge),
//! a fork of [Veloren](https://veloren.net). All code in this crate is
//! **GPL v3.0** — see [`LICENSES/GPL-3.0`] in the repository root.
//!
//! ## Architecture
//!
//! ```text
//! novaforge-game
//!   module.rs    — NovaForgeGameModule (implements atlas-game::GameModule)
//!   adapter.rs   — NovaForgeAdapter (implements the editor IGameProjectAdapter boundary)
//!   systems/     — Game systems (ported from Nova-Forge / Veloren)
//! ```
//!
//! ## License boundary
//!
//! Atlas Workspace core crates (`atlas-*`) are **MIT OR Apache-2.0** and must
//! **never** depend on this crate.  Communication between the editor and NovaForge
//! game logic flows exclusively through the `atlas-game::GameModule` trait and
//! the `IGameProjectAdapter` trait in `atlas-editor`.
//!
//! [`LICENSES/GPL-3.0`]: ../../LICENSES/GPL-3.0

pub mod module;
pub mod adapter;
pub mod systems;

pub use module::{NovaForgeGameModule, NovaForgeConfig};
pub use adapter::NovaForgeAdapter;
