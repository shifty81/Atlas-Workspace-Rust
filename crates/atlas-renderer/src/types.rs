//! Shared renderer types and error handling.

use thiserror::Error;

/// Error type for the renderer subsystem.
#[derive(Debug, Error)]
pub enum RendererError {
    #[error("Vulkan error: {0}")]
    Vulkan(String),

    #[error("no suitable GPU found")]
    NoSuitableGpu,

    #[error("swapchain out of date")]
    SwapchainOutOfDate,

    #[error("shader load error: {0}")]
    ShaderLoad(String),

    #[error("allocation failed: {0}")]
    AllocationFailed(String),

    #[error("window error: {0}")]
    Window(String),

    #[error("{0}")]
    Other(String),
}

/// Convenience alias.
pub type RendererResult<T> = Result<T, RendererError>;

/// Renderer initialisation configuration.
#[derive(Clone, Debug)]
pub struct RenderConfig {
    /// Window title.
    pub title:              String,
    /// Initial window width.
    pub width:              u32,
    /// Initial window height.
    pub height:             u32,
    /// Enable Vulkan validation layers (debug only).
    pub validation_layers:  bool,
    /// Preferred number of frames in flight.
    pub frames_in_flight:   usize,
    /// VSync / present mode preference.
    pub vsync:              bool,
    /// Enable MSAA (4× if supported).
    pub msaa:               bool,
}

impl Default for RenderConfig {
    fn default() -> Self {
        Self {
            title:             "Atlas Workspace".into(),
            width:             1280,
            height:            720,
            validation_layers: cfg!(debug_assertions),
            frames_in_flight:  2,
            vsync:             true,
            msaa:              false,
        }
    }
}
