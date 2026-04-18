//! # Atlas Editor
//!
//! Editor application layer (M5 – M14).
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
//! | [`layout_persistence`]     | [`LayoutPersistence`] — save/restore panel layout across sessions |
//! | [`notification`]           | [`NotificationCenter`] — workspace notification queue with severity |
//! | [`panels`]                 | Five editor panels |
//! | [`property_grid`]          | [`PropertyGrid`] — sectioned key/value property inspector |
//! | [`scene_renderer`]         | [`SceneRenderer`] — renders the ECS world to an offscreen texture |
//! | [`scene_serial`]           | Scene JSON save / load |
//! | [`selection`]              | [`SelectionState`] — which entities are selected |
//! | [`viewport_host`]          | [`ViewportHost`] trait + [`ViewportRegistry`] |

pub mod app;
pub mod build_system;
pub mod command;
pub mod entity_commands;
pub mod game_project_adapter;
pub mod layout_persistence;
pub mod notification;
pub mod panels;
pub mod property_grid;
pub mod scene_renderer;
pub mod scene_serial;
pub mod selection;
pub mod viewport_host;

pub use app::EditorApp;
pub use build_system::{GameBuildSystem, BuildStatus};
pub use command::{EditorCommand, CommandStack};
pub use entity_commands::{SpawnEntityCommand, DeleteEntityCommand, RenameEntityCommand};
pub use game_project_adapter::{
    GameProjectAdapter, EditorSession, PieState,
    StandaloneGameAdapter,
};
pub use layout_persistence::{LayoutPersistence, PanelLayout, DockSide};
pub use notification::{Notification, NotificationCenter, NotificationSeverity};
pub use property_grid::{PropertyGrid, PropertySection, PropertyEntry, PropertyValue};
pub use scene_renderer::SceneRenderer;
pub use scene_serial::{serialize_scene, deserialize_scene, save_scene, load_scene};
pub use selection::SelectionState;
pub use viewport_host::{
    ViewportHost, ViewportRegistry, ViewportKind, ViewportRenderMode, ViewportSize,
    StubViewport,
};
