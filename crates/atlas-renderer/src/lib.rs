//! # Atlas Renderer
//!
//! Vulkan rendering backend for Atlas Workspace.
//!
//! ## Architecture
//!
//! The renderer is built directly on [`ash`] (raw Vulkan bindings) with
//! [`gpu-allocator`] for GPU memory management.
//!
//! ### Modules
//!
//! | Module | Purpose |
//! |--------|---------|
//! | [`context`] | Vulkan instance, device, queues, surface |
//! | [`swapchain`] | Swapchain + frame synchronisation |
//! | [`pipeline`] | Graphics pipeline builder |
//! | [`buffer`] | GPU buffer wrappers (vertex, index, uniform) |
//! | [`texture`] | Image + sampler |
//! | [`frame`] | Per-frame command recording |
//! | [`shader`] | SPIR-V shader loading |
//! | [`render_pass`] | Render pass / attachment descriptions |
//! | [`viewport`] | Camera and viewport state |
//! | [`types`] | Shared renderer types |

pub mod buffer;
pub mod context;
pub mod frame;
pub mod pipeline;
pub mod render_pass;
pub mod shader;
pub mod swapchain;
pub mod texture;
pub mod types;
pub mod viewport;

pub use context::VulkanContext;
pub use types::{RenderConfig, RendererError, RendererResult};
pub use viewport::{Camera, Viewport};
