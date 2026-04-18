//! # Atlas UI
//!
//! Immediate-mode GUI layer built on [`egui`] with Vulkan rendering (M4).
//!
//! ## Architecture
//!
//! | Type | Purpose |
//! |------|---------|
//! | [`UiContext`]   | Owns the egui [`Context`] and the winit input state |
//! | [`UiRenderer`]  | Tessellates egui output; Vulkan path renders it to screen |
//!
//! `UiContext::begin_frame` + `UiContext::end_frame` produce a
//! [`egui::FullOutput`] which is passed to `UiRenderer::render`.

pub mod context;
pub mod log_capture;

pub use context::{UiContext, FrameOutput};
pub use log_capture::UiLogCapture;

/// Convenience re-export so callers don't need a direct `egui` dep for
/// the most common types.
pub use egui::{self, Context as EguiContext};
