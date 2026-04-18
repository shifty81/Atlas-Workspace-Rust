//! GPU buffer wrappers.
//!
//! Abstracts over vertex, index, and uniform buffer creation.

/// GPU buffer usage flags.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum BufferUsage {
    Vertex,
    Index,
    Uniform,
    Storage,
    Staging,
}

/// A typed GPU buffer descriptor.
#[derive(Debug)]
pub struct GpuBuffer {
    pub usage:      BufferUsage,
    pub size_bytes: u64,
    /// Optional debug label.
    pub label:      Option<String>,
}

impl GpuBuffer {
    pub fn vertex(size_bytes: u64) -> Self {
        Self { usage: BufferUsage::Vertex, size_bytes, label: None }
    }

    pub fn index(size_bytes: u64) -> Self {
        Self { usage: BufferUsage::Index, size_bytes, label: None }
    }

    pub fn uniform(size_bytes: u64) -> Self {
        Self { usage: BufferUsage::Uniform, size_bytes, label: None }
    }

    pub fn with_label(mut self, label: impl Into<String>) -> Self {
        self.label = Some(label.into());
        self
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn buffer_descriptors() {
        let vb = GpuBuffer::vertex(1024).with_label("terrain_verts");
        assert_eq!(vb.usage, BufferUsage::Vertex);
        assert_eq!(vb.size_bytes, 1024);
        assert_eq!(vb.label.as_deref(), Some("terrain_verts"));
    }
}
