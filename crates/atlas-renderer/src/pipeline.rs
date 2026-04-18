//! Graphics / compute pipeline builder.

use crate::shader::ShaderModule;

/// Vertex attribute format.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum VertexFormat {
    Float32x2,
    Float32x3,
    Float32x4,
    Uint32,
}

impl VertexFormat {
    pub fn size_bytes(self) -> u32 {
        match self {
            Self::Float32x2 => 8,
            Self::Float32x3 => 12,
            Self::Float32x4 => 16,
            Self::Uint32    => 4,
        }
    }
}

/// A single vertex attribute binding.
#[derive(Clone, Debug)]
pub struct VertexAttribute {
    pub location: u32,
    pub format:   VertexFormat,
    pub offset:   u32,
}

/// Primitive topology.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum PrimitiveTopology {
    TriangleList,
    TriangleStrip,
    LineList,
    PointList,
}

/// Polygon fill mode.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum FillMode {
    Fill,
    Line,
    Point,
}

/// Cull mode.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum CullMode {
    None,
    Front,
    Back,
}

/// Blend mode for colour attachments.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum BlendMode {
    Opaque,
    AlphaBlend,
    Additive,
}

/// Graphics pipeline descriptor.
pub struct PipelineDesc {
    pub vertex_shader:    ShaderModule,
    pub fragment_shader:  ShaderModule,
    pub vertex_attributes: Vec<VertexAttribute>,
    pub vertex_stride:    u32,
    pub topology:         PrimitiveTopology,
    pub fill_mode:        FillMode,
    pub cull_mode:        CullMode,
    pub blend_mode:       BlendMode,
    pub depth_test:       bool,
    pub depth_write:      bool,
    pub label:            Option<String>,
}

impl PipelineDesc {
    pub fn builder(vert: ShaderModule, frag: ShaderModule) -> PipelineBuilder {
        PipelineBuilder::new(vert, frag)
    }
}

/// Fluent builder for [`PipelineDesc`].
pub struct PipelineBuilder {
    vert:        ShaderModule,
    frag:        ShaderModule,
    attrs:       Vec<VertexAttribute>,
    stride:      u32,
    topology:    PrimitiveTopology,
    fill_mode:   FillMode,
    cull_mode:   CullMode,
    blend_mode:  BlendMode,
    depth_test:  bool,
    depth_write: bool,
    label:       Option<String>,
}

impl PipelineBuilder {
    pub fn new(vert: ShaderModule, frag: ShaderModule) -> Self {
        Self {
            vert,
            frag,
            attrs:       Vec::new(),
            stride:      0,
            topology:    PrimitiveTopology::TriangleList,
            fill_mode:   FillMode::Fill,
            cull_mode:   CullMode::Back,
            blend_mode:  BlendMode::Opaque,
            depth_test:  true,
            depth_write: true,
            label:       None,
        }
    }

    pub fn attribute(mut self, location: u32, format: VertexFormat, offset: u32) -> Self {
        self.attrs.push(VertexAttribute { location, format, offset });
        self
    }

    pub fn vertex_stride(mut self, stride: u32) -> Self {
        self.stride = stride;
        self
    }

    pub fn topology(mut self, t: PrimitiveTopology) -> Self {
        self.topology = t;
        self
    }

    pub fn fill_mode(mut self, m: FillMode) -> Self {
        self.fill_mode = m;
        self
    }

    pub fn cull_mode(mut self, m: CullMode) -> Self {
        self.cull_mode = m;
        self
    }

    pub fn blend_mode(mut self, m: BlendMode) -> Self {
        self.blend_mode = m;
        self
    }

    pub fn depth_test(mut self, test: bool, write: bool) -> Self {
        self.depth_test  = test;
        self.depth_write = write;
        self
    }

    pub fn label(mut self, l: impl Into<String>) -> Self {
        self.label = Some(l.into());
        self
    }

    pub fn build(self) -> PipelineDesc {
        PipelineDesc {
            vertex_shader:     self.vert,
            fragment_shader:   self.frag,
            vertex_attributes: self.attrs,
            vertex_stride:     self.stride,
            topology:          self.topology,
            fill_mode:         self.fill_mode,
            cull_mode:         self.cull_mode,
            blend_mode:        self.blend_mode,
            depth_test:        self.depth_test,
            depth_write:       self.depth_write,
            label:             self.label,
        }
    }
}
