//! # Atlas Editor
//!
//! Editor application layer (M5 – M7).
//!
//! ## Crate structure
//!
//! | Module | Contents |
//! |--------|----------|
//! | [`app`]                    | [`EditorApp`] — main event loop + panel orchestration |
//! | [`build_system`]           | [`GameBuildSystem`] — background cargo build of atlas-game |
//! | [`command`]                | [`EditorCommand`] trait + [`CommandStack`] (Ctrl-Z / Ctrl-Shift-Z) |
//! | [`entity_commands`]        | [`SpawnEntityCommand`], [`DeleteEntityCommand`] |
//! | [`game_project_adapter`]   | [`GameProjectAdapter`] trait + [`EditorSession`] (PIE) |
//! | [`panels`]                 | Five editor panels |
//! | [`scene_renderer`]         | [`SceneRenderer`] — renders the ECS world to an offscreen texture |
//! | [`scene_serial`]           | Scene JSON save / load |
//! | [`selection`]              | [`SelectionState`] — which entities are selected |

pub mod app;
pub mod build_system;
pub mod command;
pub mod entity_commands;
pub mod game_project_adapter;
pub mod panels;
pub mod scene_renderer;
pub mod scene_serial;
pub mod selection;

pub use app::EditorApp;
pub use build_system::{GameBuildSystem, BuildStatus};
pub use command::{EditorCommand, CommandStack};
pub use entity_commands::{SpawnEntityCommand, DeleteEntityCommand, RenameEntityCommand};
pub use game_project_adapter::{
    GameProjectAdapter, EditorSession, PieState,
    StandaloneGameAdapter,
};
pub use scene_renderer::SceneRenderer;
pub use scene_serial::{serialize_scene, deserialize_scene, save_scene, load_scene};
pub use selection::SelectionState;
