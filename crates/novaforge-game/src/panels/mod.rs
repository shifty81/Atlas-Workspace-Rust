//! NovaForge gameplay editor panels (M16 — stub implementations).
//!
//! These panels are the Rust port of the C++ `NovaForge::EditorAdapter::Panels`
//! classes.  Each struct holds the panel's data model and exposes the
//! [`IEditorPanel`] interface so that the Atlas Workspace editor can display
//! them in the tool layout.
//!
//! Current status: **data model + `IEditorPanel` impl only**.
//! egui rendering will be added in a future milestone.
//!
//! # License
//!
//! GPL v3.0 — this file is part of `novaforge-game`.

pub mod character;
pub mod economy;
pub mod inventory;
pub mod missions;
pub mod progression;
pub mod shop;

pub use character::CharacterRulesPanel;
pub use economy::EconomyPanel;
pub use inventory::InventoryRulesPanel;
pub use missions::MissionRulesPanel;
pub use progression::ProgressionPanel;
pub use shop::ShopPanel;
