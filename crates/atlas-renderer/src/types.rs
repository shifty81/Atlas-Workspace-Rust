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

impl From<ash::vk::Result> for RendererError {
    fn from(r: ash::vk::Result) -> Self {
        Self::Vulkan(r.to_string())
    }
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
    /// Run without a GPU (no window, no GPU calls).
    /// Automatically set when `ATLAS_HEADLESS=1` is in the environment.
    pub headless:           bool,
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
            headless:          std::env::var("ATLAS_HEADLESS").is_ok(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn render_config_default_values() {
        let cfg = RenderConfig::default();
        assert_eq!(cfg.width, 1280);
        assert_eq!(cfg.height, 720);
        assert_eq!(cfg.frames_in_flight, 2);
        assert!(cfg.vsync);
        assert!(!cfg.msaa);
    }

    #[test]
    fn renderer_error_display_vulkan() {
        let e = RendererError::Vulkan("VK_ERROR_OUT_OF_MEMORY".into());
        assert!(e.to_string().contains("Vulkan"));
    }

    #[test]
    fn renderer_error_display_no_gpu() {
        let e = RendererError::NoSuitableGpu;
        assert!(e.to_string().contains("GPU"));
    }

    #[test]
    fn renderer_error_display_shader_load() {
        let e = RendererError::ShaderLoad("missing.spv".into());
        assert!(e.to_string().contains("shader"));
    }
}
