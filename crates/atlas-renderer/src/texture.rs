//! Image and sampler types.

/// Sampler filtering mode.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum FilterMode {
    Nearest,
    Linear,
}

/// Sampler address mode.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum AddressMode {
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder,
}

/// A sampler configuration.
#[derive(Clone, Debug)]
pub struct SamplerDesc {
    pub min_filter:  FilterMode,
    pub mag_filter:  FilterMode,
    pub mip_filter:  FilterMode,
    pub address_u:   AddressMode,
    pub address_v:   AddressMode,
    pub address_w:   AddressMode,
    pub anisotropy:  Option<f32>,
}

impl Default for SamplerDesc {
    fn default() -> Self {
        Self {
            min_filter: FilterMode::Linear,
            mag_filter: FilterMode::Linear,
            mip_filter: FilterMode::Linear,
            address_u:  AddressMode::Repeat,
            address_v:  AddressMode::Repeat,
            address_w:  AddressMode::Repeat,
            anisotropy: Some(16.0),
        }
    }
}

/// A 2-D texture descriptor.
#[derive(Clone, Debug)]
pub struct TextureDesc {
    pub width:      u32,
    pub height:     u32,
    pub mip_levels: u32,
    pub format:     TextureFormat,
    pub sampler:    SamplerDesc,
    pub label:      Option<String>,
}

/// Texture pixel format.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum TextureFormat {
    R8G8B8A8Unorm,
    R8G8B8A8Srgb,
    Bc1RgbUnorm,
    Bc3RgbaUnorm,
    Bc7RgbaUnorm,
    R32G32B32A32Sfloat,
}

impl TextureDesc {
    pub fn rgba_2d(width: u32, height: u32) -> Self {
        Self {
            width,
            height,
            mip_levels: 1,
            format: TextureFormat::R8G8B8A8Unorm,
            sampler: SamplerDesc::default(),
            label: None,
        }
    }

    pub fn with_label(mut self, label: impl Into<String>) -> Self {
        self.label = Some(label.into());
        self
    }
}
