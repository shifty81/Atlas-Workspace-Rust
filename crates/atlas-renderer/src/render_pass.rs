//! Render pass and attachment descriptions.

/// Load operation for an attachment at the start of a render pass.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum LoadOp {
    Load,
    Clear,
    DontCare,
}

/// Store operation for an attachment at the end of a render pass.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum StoreOp {
    Store,
    DontCare,
}

/// Attachment description (colour or depth/stencil).
#[derive(Clone, Debug)]
pub struct AttachmentDesc {
    pub format:     AttachmentFormat,
    pub load_op:    LoadOp,
    pub store_op:   StoreOp,
    pub final_layout: ImageLayout,
}

/// Minimal image format enum.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum AttachmentFormat {
    B8G8R8A8Srgb,
    R8G8B8A8Unorm,
    D32Sfloat,
    D24UnormS8Uint,
}

/// Minimal image layout enum.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ImageLayout {
    Undefined,
    ColorAttachmentOptimal,
    DepthStencilAttachmentOptimal,
    PresentSrc,
    ShaderReadOnlyOptimal,
}

/// A render pass configuration.
#[derive(Clone, Debug, Default)]
pub struct RenderPassConfig {
    pub color_attachments: Vec<AttachmentDesc>,
    pub depth_attachment:  Option<AttachmentDesc>,
}

impl RenderPassConfig {
    pub fn new() -> Self { Self::default() }

    pub fn add_color(mut self, desc: AttachmentDesc) -> Self {
        self.color_attachments.push(desc);
        self
    }

    pub fn with_depth(mut self, desc: AttachmentDesc) -> Self {
        self.depth_attachment = Some(desc);
        self
    }
}
