//! Vulkan swapchain, image views, render pass, and framebuffers (M2 / M3).
//!
//! [`Swapchain`] wraps `VkSwapchainKHR` together with per-image views and a
//! simple clear render pass with one `VkFramebuffer` per image.  On headless
//! builds every GPU call is skipped and sensible stub values are returned.

use crate::types::{RendererError, RendererResult};

/// Presentation mode preference.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum PresentMode {
    Fifo,
    Immediate,
    Mailbox,
}

impl PresentMode {
    pub fn vk_name(self) -> &'static str {
        match self {
            Self::Fifo      => "VK_PRESENT_MODE_FIFO_KHR",
            Self::Immediate => "VK_PRESENT_MODE_IMMEDIATE_KHR",
            Self::Mailbox   => "VK_PRESENT_MODE_MAILBOX_KHR",
        }
    }

    #[cfg(feature = "vulkan")]
    pub fn to_vk(self) -> ash::vk::PresentModeKHR {
        match self {
            Self::Fifo      => ash::vk::PresentModeKHR::FIFO,
            Self::Immediate => ash::vk::PresentModeKHR::IMMEDIATE,
            Self::Mailbox   => ash::vk::PresentModeKHR::MAILBOX,
        }
    }
}

/// Swapchain creation configuration.
#[derive(Clone, Debug)]
pub struct SwapchainConfig {
    pub width:        u32,
    pub height:       u32,
    pub image_count:  u32,
    pub present_mode: PresentMode,
    pub hdr:          bool,
}

impl Default for SwapchainConfig {
    fn default() -> Self {
        Self { width: 1280, height: 720, image_count: 2, present_mode: PresentMode::Fifo, hdr: false }
    }
}

// ── Vulkan swapchain inner state ────────────────────────────────────────────

#[cfg(feature = "vulkan")]
pub struct SwapchainHandles {
    pub swapchain_fn: ash::extensions::khr::Swapchain,
    pub swapchain:    ash::vk::SwapchainKHR,
    pub images:       Vec<ash::vk::Image>,
    pub image_views:  Vec<ash::vk::ImageView>,
    pub format:       ash::vk::Format,
    pub extent:       ash::vk::Extent2D,
    pub render_pass:  ash::vk::RenderPass,
    pub framebuffers: Vec<ash::vk::Framebuffer>,
    // depth resources
    pub depth_image:  ash::vk::Image,
    pub depth_view:   ash::vk::ImageView,
    pub depth_memory: ash::vk::DeviceMemory,
    device:           ash::Device,
}

#[cfg(feature = "vulkan")]
impl Drop for SwapchainHandles {
    fn drop(&mut self) {
        unsafe {
            self.device.device_wait_idle().ok();
            for &fb in &self.framebuffers { self.device.destroy_framebuffer(fb, None); }
            self.device.destroy_render_pass(self.render_pass, None);
            self.device.destroy_image_view(self.depth_view, None);
            self.device.free_memory(self.depth_memory, None);
            self.device.destroy_image(self.depth_image, None);
            for &iv in &self.image_views  { self.device.destroy_image_view(iv, None); }
            self.swapchain_fn.destroy_swapchain(self.swapchain, None);
        }
    }
}

// ── public API ──────────────────────────────────────────────────────────────

pub struct Swapchain {
    config: SwapchainConfig,
    #[cfg(feature = "vulkan")]
    pub handles: Option<Box<SwapchainHandles>>,
}

impl Swapchain {
    /// Create a headless stub.
    pub fn new_headless(config: SwapchainConfig) -> RendererResult<Self> {
        if config.width == 0 || config.height == 0 {
            return Err(RendererError::Other("swapchain extent must be non-zero".into()));
        }
        log::info!("[Renderer] Swapchain (headless) {}×{}", config.width, config.height);
        Ok(Self {
            config,
            #[cfg(feature = "vulkan")]
            handles: None,
        })
    }

    /// Create a real Vulkan swapchain from an initialised `VulkanContext`.
    #[cfg(feature = "vulkan")]
    pub fn new_vulkan(
        config: SwapchainConfig,
        ctx:    &crate::context::VulkanContext,
    ) -> RendererResult<Self> {
        if config.width == 0 || config.height == 0 {
            return Err(RendererError::Other("swapchain extent must be non-zero".into()));
        }

        let instance   = ctx.instance().ok_or_else(|| RendererError::Other("no instance".into()))?;
        let device     = ctx.device().ok_or_else(|| RendererError::Other("no device".into()))?;
        let surface    = ctx.surface().ok_or_else(|| RendererError::Other("no surface".into()))?;
        let sf_fn      = ctx.surface_fn().ok_or_else(|| RendererError::Other("no surface_fn".into()))?;
        let phys_dev   = ctx.physical_device();
        let qf         = ctx.queue_families().ok_or_else(|| RendererError::Other("no queues".into()))?;

        let handles = build_swapchain(instance, device, sf_fn, surface, phys_dev, &config, qf)?;

        log::info!("[Renderer] Swapchain {}×{} format={:?} images={}",
            handles.extent.width, handles.extent.height,
            handles.format, handles.images.len());

        Ok(Self { config, handles: Some(Box::new(handles)) })
    }

    pub fn config(&self)      -> &SwapchainConfig { &self.config }
    pub fn image_count(&self) -> u32 {
        #[cfg(feature = "vulkan")]
        if let Some(h) = &self.handles { return h.images.len() as u32; }
        self.config.image_count
    }
    pub fn extent(&self) -> (u32, u32) {
        #[cfg(feature = "vulkan")]
        if let Some(h) = &self.handles { return (h.extent.width, h.extent.height); }
        (self.config.width, self.config.height)
    }
}

// ── builder (Vulkan feature only) ───────────────────────────────────────────

#[cfg(feature = "vulkan")]
fn build_swapchain(
    instance: &ash::Instance,
    device:   &ash::Device,
    sf_fn:    &ash::extensions::khr::Surface,
    surface:  ash::vk::SurfaceKHR,
    phys_dev: ash::vk::PhysicalDevice,
    cfg:      &SwapchainConfig,
    qf:       crate::context::QueueFamilyIndices,
) -> RendererResult<SwapchainHandles> {
    use ash::{extensions::khr, vk};

    // Query surface capabilities / formats / present modes
    let caps = unsafe {
        sf_fn.get_physical_device_surface_capabilities(phys_dev, surface).map_err(RendererError::from)?
    };
    let formats = unsafe {
        sf_fn.get_physical_device_surface_formats(phys_dev, surface).map_err(RendererError::from)?
    };
    let present_modes = unsafe {
        sf_fn.get_physical_device_surface_present_modes(phys_dev, surface).map_err(RendererError::from)?
    };

    // Pick surface format — prefer B8G8R8A8_SRGB + SRGB_NONLINEAR
    let surface_format = formats.iter()
        .find(|f| {
            f.format == vk::Format::B8G8R8A8_SRGB
            && f.color_space == vk::ColorSpaceKHR::SRGB_NONLINEAR
        })
        .or_else(|| formats.first())
        .cloned()
        .ok_or(RendererError::NoSuitableGpu)?;

    // Pick present mode
    let desired = cfg.present_mode.to_vk();
    let present_mode = if present_modes.contains(&desired) { desired } else { vk::PresentModeKHR::FIFO };

    // Extent
    let extent = if caps.current_extent.width != u32::MAX {
        caps.current_extent
    } else {
        vk::Extent2D {
            width:  cfg.width.clamp(caps.min_image_extent.width,  caps.max_image_extent.width),
            height: cfg.height.clamp(caps.min_image_extent.height, caps.max_image_extent.height),
        }
    };

    // Image count
    let mut image_count = caps.min_image_count + 1;
    if caps.max_image_count > 0 { image_count = image_count.min(caps.max_image_count); }

    // Sharing mode
    let (sharing, q_indices): (vk::SharingMode, Vec<u32>) = if qf.graphics == qf.present {
        (vk::SharingMode::EXCLUSIVE, vec![])
    } else {
        (vk::SharingMode::CONCURRENT, vec![qf.graphics, qf.present])
    };

    let sc_ci = vk::SwapchainCreateInfoKHR {
        surface,
        min_image_count:    image_count,
        image_format:       surface_format.format,
        image_color_space:  surface_format.color_space,
        image_extent:       extent,
        image_array_layers: 1,
        image_usage:        vk::ImageUsageFlags::COLOR_ATTACHMENT,
        image_sharing_mode: sharing,
        queue_family_index_count: q_indices.len() as u32,
        p_queue_family_indices:   if q_indices.is_empty() { std::ptr::null() } else { q_indices.as_ptr() },
        pre_transform:      caps.current_transform,
        composite_alpha:    vk::CompositeAlphaFlagsKHR::OPAQUE,
        present_mode,
        clipped:            vk::TRUE,
        ..Default::default()
    };

    let swapchain_fn = khr::Swapchain::new(instance, device);
    let swapchain = unsafe { swapchain_fn.create_swapchain(&sc_ci, None).map_err(RendererError::from)? };

    // Retrieve swapchain images
    let images = unsafe { swapchain_fn.get_swapchain_images(swapchain).map_err(RendererError::from)? };

    // Create image views
    let image_views: Vec<vk::ImageView> = images.iter().map(|&img| {
        let view_ci = vk::ImageViewCreateInfo {
            image: img,
            view_type:         vk::ImageViewType::TYPE_2D,
            format:            surface_format.format,
            components:        vk::ComponentMapping::default(),
            subresource_range: vk::ImageSubresourceRange {
                aspect_mask:      vk::ImageAspectFlags::COLOR,
                base_mip_level:   0,
                level_count:      1,
                base_array_layer: 0,
                layer_count:      1,
            },
            ..Default::default()
        };
        unsafe { device.create_image_view(&view_ci, None).expect("image view") }
    }).collect();

    // Depth buffer
    let depth_format = vk::Format::D32_SFLOAT;
    let (depth_image, depth_memory) = create_image(
        device, phys_dev, instance,
        extent.width, extent.height, depth_format,
        vk::ImageUsageFlags::DEPTH_STENCIL_ATTACHMENT,
        vk::MemoryPropertyFlags::DEVICE_LOCAL,
    )?;
    let depth_view = unsafe {
        device.create_image_view(&vk::ImageViewCreateInfo {
            image: depth_image,
            view_type: vk::ImageViewType::TYPE_2D,
            format: depth_format,
            subresource_range: vk::ImageSubresourceRange {
                aspect_mask:      vk::ImageAspectFlags::DEPTH,
                base_mip_level:   0,
                level_count:      1,
                base_array_layer: 0,
                layer_count:      1,
            },
            ..Default::default()
        }, None).map_err(RendererError::from)?
    };

    // Render pass (one colour + depth attachment, clear on load)
    let render_pass = create_render_pass(device, surface_format.format, depth_format)?;

    // Framebuffers
    let framebuffers = image_views.iter().map(|&iv| {
        let attachments = [iv, depth_view];
        let fb_ci = vk::FramebufferCreateInfo {
            render_pass,
            attachment_count: attachments.len() as u32,
            p_attachments:    attachments.as_ptr(),
            width:  extent.width,
            height: extent.height,
            layers: 1,
            ..Default::default()
        };
        unsafe { device.create_framebuffer(&fb_ci, None).expect("framebuffer") }
    }).collect();

    Ok(SwapchainHandles {
        swapchain_fn, swapchain, images, image_views,
        format: surface_format.format, extent, render_pass, framebuffers,
        depth_image, depth_view, depth_memory,
        device: device.clone(),
    })
}

/// Create a device-local image + allocate memory for it.
#[cfg(feature = "vulkan")]
fn create_image(
    device:    &ash::Device,
    phys_dev:  ash::vk::PhysicalDevice,
    instance:  &ash::Instance,
    width:     u32,
    height:    u32,
    format:    ash::vk::Format,
    usage:     ash::vk::ImageUsageFlags,
    mem_props: ash::vk::MemoryPropertyFlags,
) -> RendererResult<(ash::vk::Image, ash::vk::DeviceMemory)> {
    use ash::vk;

    let img_ci = vk::ImageCreateInfo {
        image_type:   vk::ImageType::TYPE_2D,
        format,
        extent:       vk::Extent3D { width, height, depth: 1 },
        mip_levels:   1,
        array_layers: 1,
        samples:      vk::SampleCountFlags::TYPE_1,
        tiling:       vk::ImageTiling::OPTIMAL,
        usage,
        sharing_mode: vk::SharingMode::EXCLUSIVE,
        initial_layout: vk::ImageLayout::UNDEFINED,
        ..Default::default()
    };
    let image = unsafe { device.create_image(&img_ci, None).map_err(RendererError::from)? };
    let reqs  = unsafe { device.get_image_memory_requirements(image) };
    let phys_props = unsafe { instance.get_physical_device_memory_properties(phys_dev) };
    let mem_type = (0..phys_props.memory_type_count).find(|&i| {
        reqs.memory_type_bits & (1 << i) != 0
        && phys_props.memory_types[i as usize].property_flags.contains(mem_props)
    }).ok_or(RendererError::AllocationFailed("no suitable memory type".into()))?;

    let alloc_ci = vk::MemoryAllocateInfo {
        allocation_size: reqs.size,
        memory_type_index: mem_type,
        ..Default::default()
    };
    let memory = unsafe { device.allocate_memory(&alloc_ci, None).map_err(RendererError::from)? };
    unsafe { device.bind_image_memory(image, memory, 0).map_err(RendererError::from)? };

    Ok((image, memory))
}

/// Create a simple render pass: clear-colour + depth attachment.
#[cfg(feature = "vulkan")]
fn create_render_pass(
    device:       &ash::Device,
    colour_fmt:   ash::vk::Format,
    depth_fmt:    ash::vk::Format,
) -> RendererResult<ash::vk::RenderPass> {
    use ash::vk;

    let attachments = [
        // Colour
        vk::AttachmentDescription {
            format:          colour_fmt,
            samples:         vk::SampleCountFlags::TYPE_1,
            load_op:         vk::AttachmentLoadOp::CLEAR,
            store_op:        vk::AttachmentStoreOp::STORE,
            stencil_load_op: vk::AttachmentLoadOp::DONT_CARE,
            stencil_store_op:vk::AttachmentStoreOp::DONT_CARE,
            initial_layout:  vk::ImageLayout::UNDEFINED,
            final_layout:    vk::ImageLayout::PRESENT_SRC_KHR,
            ..Default::default()
        },
        // Depth
        vk::AttachmentDescription {
            format:          depth_fmt,
            samples:         vk::SampleCountFlags::TYPE_1,
            load_op:         vk::AttachmentLoadOp::CLEAR,
            store_op:        vk::AttachmentStoreOp::DONT_CARE,
            stencil_load_op: vk::AttachmentLoadOp::DONT_CARE,
            stencil_store_op:vk::AttachmentStoreOp::DONT_CARE,
            initial_layout:  vk::ImageLayout::UNDEFINED,
            final_layout:    vk::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            ..Default::default()
        },
    ];

    let colour_ref = vk::AttachmentReference {
        attachment: 0,
        layout:     vk::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
    };
    let depth_ref = vk::AttachmentReference {
        attachment: 1,
        layout:     vk::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    let subpass = vk::SubpassDescription {
        pipeline_bind_point:        vk::PipelineBindPoint::GRAPHICS,
        color_attachment_count:     1,
        p_color_attachments:        &colour_ref,
        p_depth_stencil_attachment: &depth_ref,
        ..Default::default()
    };

    let dependency = vk::SubpassDependency {
        src_subpass:    vk::SUBPASS_EXTERNAL,
        dst_subpass:    0,
        src_stage_mask: vk::PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT
                      | vk::PipelineStageFlags::EARLY_FRAGMENT_TESTS,
        dst_stage_mask: vk::PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT
                      | vk::PipelineStageFlags::EARLY_FRAGMENT_TESTS,
        src_access_mask: vk::AccessFlags::empty(),
        dst_access_mask: vk::AccessFlags::COLOR_ATTACHMENT_WRITE
                       | vk::AccessFlags::DEPTH_STENCIL_ATTACHMENT_WRITE,
        ..Default::default()
    };

    let rp_ci = vk::RenderPassCreateInfo {
        attachment_count: attachments.len() as u32,
        p_attachments:    attachments.as_ptr(),
        subpass_count:    1,
        p_subpasses:      &subpass,
        dependency_count: 1,
        p_dependencies:   &dependency,
        ..Default::default()
    };

    unsafe { device.create_render_pass(&rp_ci, None).map_err(RendererError::from) }
}
