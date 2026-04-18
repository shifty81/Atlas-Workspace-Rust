//! Main render loop: acquire → record → present (M3).
//!
//! [`RenderLoop`] ties together [`VulkanContext`], [`Swapchain`], and
//! [`FrameSync`] to produce the classic Vulkan frame loop.  On headless
//! builds every call is a no-op so CI passes without a GPU.

use crate::{
    context::VulkanContext,
    frame::FrameSync,
    swapchain::Swapchain,
    types::{RendererError, RendererResult},
};

/// Clear colour (RGBA, linear).
#[derive(Clone, Copy, Debug)]
pub struct ClearColor {
    pub r: f32,
    pub g: f32,
    pub b: f32,
    pub a: f32,
}

impl Default for ClearColor {
    fn default() -> Self { Self { r: 0.08, g: 0.08, b: 0.08, a: 1.0 } }
}

// ── additional egui paint job type (M4 integration point) ──────────────────

/// Opaque paint commands produced by the UI layer (passed in by `atlas-ui`).
/// In M3 this is always empty; in M4 the egui renderer fills it.
pub struct UiPaintData {
    #[allow(dead_code)]
    pub(crate) inner: Box<dyn std::any::Any + Send>,
}

impl UiPaintData {
    pub fn empty() -> Self {
        Self { inner: Box::new(()) }
    }
}

// ── RenderLoop ──────────────────────────────────────────────────────────────

/// Drives the per-frame GPU work: acquire image, record commands, present.
pub struct RenderLoop {
    ctx:          VulkanContext,
    swapchain:    Swapchain,
    frame_sync:   FrameSync,
    clear_color:  ClearColor,
    surface_w:    u32,
    surface_h:    u32,
    needs_resize: bool,
}

impl RenderLoop {
    /// Construct from a fully initialised context.
    pub fn new(ctx: VulkanContext, swapchain: Swapchain, frame_sync: FrameSync) -> Self {
        let (w, h) = swapchain.extent();
        Self {
            ctx,
            swapchain,
            frame_sync,
            clear_color: ClearColor::default(),
            surface_w: w,
            surface_h: h,
            needs_resize: false,
        }
    }

    pub fn set_clear_color(&mut self, color: ClearColor) { self.clear_color = color; }

    /// Call from `WindowEvent::Resized`.
    pub fn on_resize(&mut self, width: u32, height: u32) {
        if width == 0 || height == 0 { return; }
        self.surface_w    = width;
        self.surface_h    = height;
        self.needs_resize = true;
    }

    /// Submit one frame.  On headless builds this is a no-op.
    pub fn draw_frame(&mut self, _ui: Option<UiPaintData>) -> RendererResult<()> {
        if self.ctx.is_headless() { return Ok(()); }

        #[cfg(feature = "vulkan")]
        return self.draw_frame_vulkan();

        #[cfg(not(feature = "vulkan"))]
        Ok(())
    }

    #[cfg(feature = "vulkan")]
    fn draw_frame_vulkan(&mut self) -> RendererResult<()> {
        use ash::vk;

        if self.needs_resize {
            self.recreate_swapchain()?;
            self.needs_resize = false;
        }

        let device = match self.ctx.device() {
            Some(d) => d,
            None => return Ok(()),
        };

        // 1. Wait for the previous use of this frame slot
        self.frame_sync.wait_for_fence(device)?;

        // 2. Acquire next swapchain image
        let handles = match self.swapchain.handles.as_deref() {
            Some(h) => h,
            None => return Ok(()),
        };

        let image_available = self.frame_sync.image_available_semaphore();
        let (image_index, suboptimal) = unsafe {
            match handles.swapchain_fn.acquire_next_image(
                handles.swapchain,
                u64::MAX,
                image_available,
                vk::Fence::null(),
            ) {
                Ok(r) => r,
                Err(vk::Result::ERROR_OUT_OF_DATE_KHR) => {
                    self.needs_resize = true;
                    return Ok(());
                }
                Err(e) => return Err(RendererError::from(e)),
            }
        };
        if suboptimal { self.needs_resize = true; }

        // 3. Reset fence AFTER successful acquire
        self.frame_sync.reset_fence(device)?;

        // 4. Record command buffer
        let cmd = self.frame_sync.command_buffer();
        self.record_clear(device, cmd, handles, image_index as usize)?;

        // 5. Submit
        let wait_sems    = [image_available];
        let signal_sems  = [self.frame_sync.render_finished_semaphore()];
        let wait_stages  = [vk::PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT];
        let cmds         = [cmd];
        let submit_info  = vk::SubmitInfo {
            wait_semaphore_count:   wait_sems.len() as u32,
            p_wait_semaphores:      wait_sems.as_ptr(),
            p_wait_dst_stage_mask:  wait_stages.as_ptr(),
            command_buffer_count:   cmds.len() as u32,
            p_command_buffers:      cmds.as_ptr(),
            signal_semaphore_count: signal_sems.len() as u32,
            p_signal_semaphores:    signal_sems.as_ptr(),
            ..Default::default()
        };
        let fence = self.frame_sync.in_flight_fence();
        unsafe {
            device.queue_submit(self.ctx.graphics_queue(), &[submit_info], fence)
                .map_err(RendererError::from)?;
        }

        // 6. Present
        let swapchains = [handles.swapchain];
        let indices    = [image_index];
        let present_info = vk::PresentInfoKHR {
            wait_semaphore_count: signal_sems.len() as u32,
            p_wait_semaphores:    signal_sems.as_ptr(),
            swapchain_count:      swapchains.len() as u32,
            p_swapchains:         swapchains.as_ptr(),
            p_image_indices:      indices.as_ptr(),
            ..Default::default()
        };
        match unsafe { handles.swapchain_fn.queue_present(self.ctx.present_queue(), &present_info) } {
            Ok(suboptimal) if suboptimal => { self.needs_resize = true; }
            Err(vk::Result::ERROR_OUT_OF_DATE_KHR) => { self.needs_resize = true; }
            Err(e) => return Err(RendererError::from(e)),
            Ok(_) => {}
        }

        self.frame_sync.advance_frame();
        Ok(())
    }

    #[cfg(feature = "vulkan")]
    fn record_clear(
        &self,
        device:      &ash::Device,
        cmd:         ash::vk::CommandBuffer,
        handles:     &crate::swapchain::SwapchainHandles,
        image_index: usize,
    ) -> RendererResult<()> {
        use ash::vk;

        unsafe {
            device.reset_command_buffer(cmd, vk::CommandBufferResetFlags::empty())
                .map_err(RendererError::from)?;

            device.begin_command_buffer(cmd, &vk::CommandBufferBeginInfo {
                flags: vk::CommandBufferUsageFlags::ONE_TIME_SUBMIT,
                ..Default::default()
            }).map_err(RendererError::from)?;

            let cc = self.clear_color;
            let clear_values = [
                vk::ClearValue { color: vk::ClearColorValue { float32: [cc.r, cc.g, cc.b, cc.a] } },
                vk::ClearValue { depth_stencil: vk::ClearDepthStencilValue { depth: 1.0, stencil: 0 } },
            ];

            let render_pass_begin = vk::RenderPassBeginInfo {
                render_pass:       handles.render_pass,
                framebuffer:       handles.framebuffers[image_index],
                render_area:       vk::Rect2D { offset: vk::Offset2D::default(), extent: handles.extent },
                clear_value_count: clear_values.len() as u32,
                p_clear_values:    clear_values.as_ptr(),
                ..Default::default()
            };
            device.cmd_begin_render_pass(cmd, &render_pass_begin, vk::SubpassContents::INLINE);
            // (M4: egui commands would be recorded here)
            device.cmd_end_render_pass(cmd);
            device.end_command_buffer(cmd).map_err(RendererError::from)?;
        }
        Ok(())
    }

    #[cfg(feature = "vulkan")]
    fn recreate_swapchain(&mut self) -> RendererResult<()> {
        use crate::swapchain::SwapchainConfig;
        if self.ctx.is_headless() { return Ok(()); }
        if let Some(device) = self.ctx.device() {
            unsafe { device.device_wait_idle().ok(); }
        }
        let (mode, hdr) = {
            let cfg = self.swapchain.config();
            (cfg.present_mode, cfg.hdr)
        };
        let new_cfg = SwapchainConfig {
            width:        self.surface_w,
            height:       self.surface_h,
            present_mode: mode,
            hdr,
            ..Default::default()
        };
        // Drop the old swapchain handles first so the native window surface is
        // released before the new VkSwapchainKHR is created.  Without this,
        // Rust's assignment order (rhs evaluated before lhs is dropped) means
        // two swapchains briefly share the same surface, which Vulkan rejects
        // with VK_ERROR_NATIVE_WINDOW_IN_USE_KHR.
        self.swapchain.handles = None;
        self.swapchain = Swapchain::new_vulkan(new_cfg, &self.ctx)?;
        log::info!("[Renderer] Swapchain recreated {}×{}", self.surface_w, self.surface_h);
        Ok(())
    }

    /// Idle wait — call before exit so GPU work completes before teardown.
    pub fn wait_idle(&self) {
        #[cfg(feature = "vulkan")]
        if let Some(device) = self.ctx.device() {
            unsafe { device.device_wait_idle().ok(); }
        }
    }
}
