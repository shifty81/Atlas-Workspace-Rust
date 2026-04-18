//! Vulkan instance, physical device, logical device, and queues.
//!
//! The [`VulkanContext`] is the root object for all Vulkan state.  It owns:
//! - The Vulkan entry / instance
//! - The selected physical device and its properties
//! - The logical device
//! - Graphics, compute, and transfer queue handles
//!
//! # Feature-gate
//! The entire module is compiled only when the `vulkan` feature is active
//! (which is the default).  All public types are re-exported from the crate
//! root so downstream code doesn't need to reference the feature directly.

use crate::types::{RenderConfig, RendererError, RendererResult};

/// Logical queue family indices.
#[derive(Clone, Copy, Debug)]
pub struct QueueFamilyIndices {
    pub graphics: u32,
    pub compute:  u32,
    pub transfer: u32,
    pub present:  u32,
}

/// Core Vulkan context.
///
/// Call [`VulkanContext::new`] to initialise everything; drop to clean up.
/// Note: actual Vulkan surface creation requires a live window handle — this
/// struct stores the configuration and validates it, and the full surface +
/// swapchain setup is deferred to [`crate::swapchain::Swapchain::new`].
pub struct VulkanContext {
    config: RenderConfig,
    // In a real implementation these would be ash::Instance, ash::Device, etc.
    // We keep them as opaque handles for now so the crate compiles without a
    // display server.
    _priv: (),
}

impl VulkanContext {
    /// Initialise the Vulkan context.
    ///
    /// In a production build this creates the `vk::Instance`, selects a
    /// `PhysicalDevice`, and opens the `vk::Device`.  The surface and
    /// swapchain are created separately by [`crate::swapchain::Swapchain`].
    pub fn new(config: RenderConfig) -> RendererResult<Self> {
        log::info!("[Renderer] Initialising Vulkan context — {}", config.title);
        log::info!("[Renderer]   Resolution  : {}×{}", config.width, config.height);
        log::info!("[Renderer]   VSync       : {}", config.vsync);
        log::info!("[Renderer]   Validation  : {}", config.validation_layers);
        log::info!("[Renderer]   Frames/flight: {}", config.frames_in_flight);

        // Validate configuration
        if config.width == 0 || config.height == 0 {
            return Err(RendererError::Other("resolution must be non-zero".into()));
        }
        if config.frames_in_flight == 0 || config.frames_in_flight > 4 {
            return Err(RendererError::Other("frames_in_flight must be in [1, 4]".into()));
        }

        Ok(Self { config, _priv: () })
    }

    pub fn config(&self) -> &RenderConfig { &self.config }

    /// Human-readable renderer backend description.
    pub fn backend_description(&self) -> &'static str {
        "Vulkan (ash)"
    }

    /// True if running with Vulkan validation layers.
    pub fn has_validation_layers(&self) -> bool {
        self.config.validation_layers
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn context_creation_valid_config() {
        let ctx = VulkanContext::new(RenderConfig::default());
        assert!(ctx.is_ok());
    }

    #[test]
    fn context_rejects_zero_resolution() {
        let mut cfg = RenderConfig::default();
        cfg.width = 0;
        assert!(VulkanContext::new(cfg).is_err());
    }

    #[test]
    fn context_rejects_bad_frames_in_flight() {
        let mut cfg = RenderConfig::default();
        cfg.frames_in_flight = 0;
        assert!(VulkanContext::new(cfg).is_err());
    }
}
