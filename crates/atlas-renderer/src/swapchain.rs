//! Swapchain and frame-synchronisation primitives.

use crate::types::{RendererError, RendererResult};

/// Presentation mode preference.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum PresentMode {
    /// VSync — never tears but may add latency.
    Fifo,
    /// Immediate — lowest latency but may tear.
    Immediate,
    /// Mailbox — triple-buffered VSync approximation.
    Mailbox,
}

impl PresentMode {
    /// Map to a Vulkan `VK_PRESENT_MODE_*` constant name (for logging).
    pub fn vk_name(self) -> &'static str {
        match self {
            Self::Fifo      => "VK_PRESENT_MODE_FIFO_KHR",
            Self::Immediate => "VK_PRESENT_MODE_IMMEDIATE_KHR",
            Self::Mailbox   => "VK_PRESENT_MODE_MAILBOX_KHR",
        }
    }
}

/// Configuration for a swapchain.
#[derive(Clone, Debug)]
pub struct SwapchainConfig {
    pub width:          u32,
    pub height:         u32,
    pub image_count:    u32,
    pub present_mode:   PresentMode,
    pub hdr:            bool,
}

impl Default for SwapchainConfig {
    fn default() -> Self {
        Self {
            width:        1280,
            height:       720,
            image_count:  2,
            present_mode: PresentMode::Fifo,
            hdr:          false,
        }
    }
}

/// Swapchain handle (owns the Vulkan `VkSwapchainKHR` in a real impl).
pub struct Swapchain {
    config: SwapchainConfig,
    _priv:  (),
}

impl Swapchain {
    pub fn new(config: SwapchainConfig) -> RendererResult<Self> {
        if config.width == 0 || config.height == 0 {
            return Err(RendererError::Other("swapchain extent must be non-zero".into()));
        }
        log::info!("[Renderer] Swapchain {}×{} mode={} images={}",
            config.width, config.height,
            config.present_mode.vk_name(),
            config.image_count,
        );
        Ok(Self { config, _priv: () })
    }

    pub fn config(&self) -> &SwapchainConfig { &self.config }
    pub fn image_count(&self) -> u32 { self.config.image_count }
}
