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
//!   module.rs          — NovaForgeGameModule (implements atlas-game::GameModule)
//!   adapter.rs         — NovaForgeAdapter (implements atlas-editor::GameProjectAdapter)
//!   bootstrap.rs       — NovaForgeProjectBootstrap (.atlas manifest validation)
//!   catalog.rs         — AssetCatalog (filesystem scan of novaforge-assets/)
//!   data_registry.rs   — DataRegistry (JSON data files from Data/)
//!   document_registry.rs — DocumentRegistry (open document tracking)
//!   panels/            — Six gameplay editor panels (M16 stubs)
//!   systems/           — Game systems (ported from Nova-Forge / Veloren)
//! ```
//!
//! ## License boundary
//!
//! Atlas Workspace core crates (`atlas-*`) are **MIT OR Apache-2.0** and must
//! **never** depend on this crate.  Communication between the editor and NovaForge
//! game logic flows exclusively through the `atlas-game::GameModule` trait and
//! the `atlas-editor::GameProjectAdapter` trait.
//!
//! [`LICENSES/GPL-3.0`]: ../../LICENSES/GPL-3.0

pub mod adapter;
pub mod bootstrap;
pub mod catalog;
pub mod data_registry;
pub mod document_registry;
pub mod module;
pub mod panels;
pub mod systems;

pub use adapter::NovaForgeAdapter;
pub use bootstrap::{NovaForgeProjectBootstrap, BootstrapResult};
pub use catalog::{AssetCatalog, AssetEntry};
pub use data_registry::DataRegistry;
pub use document_registry::{DocumentRegistry, DocumentTypeDescriptor, DocumentHandle};
pub use module::{NovaForgeGameModule, NovaForgeConfig};
pub use panels::{
    CharacterRulesPanel, EconomyPanel, InventoryRulesPanel,
    MissionRulesPanel, ProgressionPanel, ShopPanel,
};
