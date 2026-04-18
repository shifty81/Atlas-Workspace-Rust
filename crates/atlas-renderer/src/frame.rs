//! Per-frame command recording.

/// A single frame's render commands (placeholder for `VkCommandBuffer`).
#[derive(Default, Debug)]
pub struct FrameCommands {
    pub frame_index:  usize,
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
