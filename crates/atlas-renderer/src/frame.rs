//! Per-frame command recording and GPU synchronisation primitives (M3).
//!
//! [`FrameSync`] owns command pools, command buffers, semaphores, and fences
//! for N frames-in-flight.  On headless targets every method is a no-op.

use crate::types::{RendererError, RendererResult};

// ── public FrameCommands (lightweight, used by callers) ────────────────────

/// A single frame's render commands.
#[derive(Default, Debug)]
pub struct FrameCommands {
    pub frame_index:   usize,
    pub command_count: usize,
}

impl FrameCommands {
    pub fn new(frame_index: usize) -> Self {
        Self { frame_index, command_count: 0 }
    }

    /// Record a draw call (increments the command counter).
    pub fn draw(&mut self, vertex_count: u32, instance_count: u32) {
        log::trace!("[Frame {}] draw {} verts × {} instances",
            self.frame_index, vertex_count, instance_count);
        self.command_count += 1;
    }

    /// Record an indexed draw call.
    pub fn draw_indexed(&mut self, index_count: u32, instance_count: u32) {
        log::trace!("[Frame {}] draw_indexed {} idxs × {} instances",
            self.frame_index, index_count, instance_count);
        self.command_count += 1;
    }
}

// ── Vulkan sync primitives for N frames-in-flight ──────────────────────────

#[cfg(feature = "vulkan")]
struct FrameSlot {
    command_pool:   ash::vk::CommandPool,
    command_buffer: ash::vk::CommandBuffer,
    image_available: ash::vk::Semaphore,
    render_finished: ash::vk::Semaphore,
    in_flight_fence: ash::vk::Fence,
}

/// Manages per-frame command buffers and GPU synchronisation.
pub struct FrameSync {
    frames_in_flight: usize,
    current_frame:    usize,
    #[cfg(feature = "vulkan")]
    slots:            Vec<FrameSlot>,
    #[cfg(feature = "vulkan")]
    device:           Option<ash::Device>,
}

impl FrameSync {
    /// Create a headless stub.
    pub fn new_headless(frames_in_flight: usize) -> Self {
        Self {
            frames_in_flight,
            current_frame: 0,
            #[cfg(feature = "vulkan")]
            slots: Vec::new(),
            #[cfg(feature = "vulkan")]
            device: None,
        }
    }

    /// Create real GPU sync primitives.
    #[cfg(feature = "vulkan")]
    pub fn new_vulkan(
        frames_in_flight: usize,
        device:           &ash::Device,
        graphics_family:  u32,
    ) -> RendererResult<Self> {
        use ash::vk;

        let mut slots = Vec::with_capacity(frames_in_flight);

        for _ in 0..frames_in_flight {
            // Command pool per frame
            let pool_ci = vk::CommandPoolCreateInfo {
                flags:              vk::CommandPoolCreateFlags::RESET_COMMAND_BUFFER,
                queue_family_index: graphics_family,
                ..Default::default()
            };
            let command_pool = unsafe {
                device.create_command_pool(&pool_ci, None).map_err(RendererError::from)?
            };

            // Allocate one primary command buffer
            let alloc_ci = vk::CommandBufferAllocateInfo {
                command_pool,
                level:                vk::CommandBufferLevel::PRIMARY,
                command_buffer_count: 1,
                ..Default::default()
            };
            let command_buffer = unsafe {
                device.allocate_command_buffers(&alloc_ci).map_err(RendererError::from)?[0]
            };

            // Semaphores
            let sem_ci = vk::SemaphoreCreateInfo::default();
            let image_available = unsafe {
                device.create_semaphore(&sem_ci, None).map_err(RendererError::from)?
            };
            let render_finished = unsafe {
                device.create_semaphore(&sem_ci, None).map_err(RendererError::from)?
            };

            // Fence (starts signalled so first wait succeeds immediately)
            let fence_ci = vk::FenceCreateInfo {
                flags: vk::FenceCreateFlags::SIGNALED,
                ..Default::default()
            };
            let in_flight_fence = unsafe {
                device.create_fence(&fence_ci, None).map_err(RendererError::from)?
            };

            slots.push(FrameSlot { command_pool, command_buffer, image_available, render_finished, in_flight_fence });
        }

        Ok(Self {
            frames_in_flight,
            current_frame: 0,
            slots,
            device: Some(device.clone()),
        })
    }

    pub fn frames_in_flight(&self) -> usize { self.frames_in_flight }
    pub fn current_frame(&self)    -> usize { self.current_frame }

    pub fn advance_frame(&mut self) {
        self.current_frame = (self.current_frame + 1) % self.frames_in_flight;
    }

    // ── Vulkan-only methods ──────────────────────────────────────────────

    #[cfg(feature = "vulkan")]
    pub fn wait_for_fence(&self, device: &ash::Device) -> RendererResult<()> {
        if self.slots.is_empty() { return Ok(()); }
        let fence = self.slots[self.current_frame].in_flight_fence;
        unsafe {
            device.wait_for_fences(&[fence], true, u64::MAX).map_err(RendererError::from)?;
        }
        Ok(())
    }

    #[cfg(feature = "vulkan")]
    pub fn reset_fence(&self, device: &ash::Device) -> RendererResult<()> {
        if self.slots.is_empty() { return Ok(()); }
        let fence = self.slots[self.current_frame].in_flight_fence;
        unsafe { device.reset_fences(&[fence]).map_err(RendererError::from) }
    }

    #[cfg(feature = "vulkan")]
    pub fn image_available_semaphore(&self) -> ash::vk::Semaphore {
        if self.slots.is_empty() { return ash::vk::Semaphore::null(); }
        self.slots[self.current_frame].image_available
    }

    #[cfg(feature = "vulkan")]
    pub fn render_finished_semaphore(&self) -> ash::vk::Semaphore {
        if self.slots.is_empty() { return ash::vk::Semaphore::null(); }
        self.slots[self.current_frame].render_finished
    }

    #[cfg(feature = "vulkan")]
    pub fn in_flight_fence(&self) -> ash::vk::Fence {
        if self.slots.is_empty() { return ash::vk::Fence::null(); }
        self.slots[self.current_frame].in_flight_fence
    }

    #[cfg(feature = "vulkan")]
    pub fn command_buffer(&self) -> ash::vk::CommandBuffer {
        if self.slots.is_empty() { return ash::vk::CommandBuffer::null(); }
        self.slots[self.current_frame].command_buffer
    }

    #[cfg(feature = "vulkan")]
    pub fn command_pool(&self) -> ash::vk::CommandPool {
        if self.slots.is_empty() { return ash::vk::CommandPool::null(); }
        self.slots[self.current_frame].command_pool
    }
}

#[cfg(feature = "vulkan")]
impl Drop for FrameSync {
    fn drop(&mut self) {
        if let Some(device) = &self.device {
            unsafe {
                device.device_wait_idle().ok();
                for slot in &self.slots {
                    device.destroy_semaphore(slot.image_available, None);
                    device.destroy_semaphore(slot.render_finished, None);
                    device.destroy_fence(slot.in_flight_fence, None);
                    device.destroy_command_pool(slot.command_pool, None);
                }
            }
        }
    }
}
