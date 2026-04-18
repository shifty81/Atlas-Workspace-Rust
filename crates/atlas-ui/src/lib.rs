//! # Atlas UI
//!
//! Immediate-mode GUI layer built on [`egui`] with Vulkan rendering (M4 – M14).
//!
//! ## Architecture
//!
//! | Type | Purpose |
//! |------|---------|
//! | [`UiContext`]   | Owns the egui [`Context`] and the winit input state |
//! | [`UiLogCapture`] | Captures `log` records for the Console panel |
//! | [`ScrollList`]  | Virtual scroll state for large item lists |
//! | [`TreeView`]    | Collapsible tree widget state (expand/collapse/select) |
//!
//! `UiContext::begin_frame` + `UiContext::end_frame` produce a
//! [`egui::FullOutput`] which is passed to `UiRenderer::render`.

pub mod context;
pub mod log_capture;
pub mod renderer;
pub mod scroll_list;
pub mod tree_view;

pub use context::{UiContext, FrameOutput};
pub use log_capture::UiLogCapture;
pub use renderer::UiRenderer;
pub use scroll_list::ScrollList;
pub use tree_view::{TreeView, TreeNode, NodeId};

/// Convenience re-export so callers don't need a direct `egui` dep for
/// the most common types.
pub use egui::{self, Context as EguiContext};
