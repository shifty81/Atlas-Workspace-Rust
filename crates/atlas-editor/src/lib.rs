//! # Atlas Editor
//!
//! Editor application layer (M5 – M7).
//!
//! ## Crate structure
//!
//! | Module | Contents |
//! |--------|----------|
//! | [`app`]            | [`EditorApp`] — main event loop + panel orchestration |
//! | [`command`]        | [`EditorCommand`] trait + [`CommandStack`] (Ctrl-Z / Ctrl-Shift-Z) |
//! | [`selection`]      | [`SelectionState`] — which entities are selected |
//! | [`scene_renderer`] | [`SceneRenderer`] — renders the ECS world to an offscreen texture |
//! | [`panels`]         | Five editor panels |

pub mod app;
pub mod command;
pub mod panels;
pub mod scene_renderer;
pub mod selection;

pub use app::EditorApp;
pub use command::{EditorCommand, CommandStack};
pub use scene_renderer::SceneRenderer;
pub use selection::SelectionState;
