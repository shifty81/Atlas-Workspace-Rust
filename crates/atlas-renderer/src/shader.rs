//! SPIR-V shader module loading.

use crate::types::{RendererError, RendererResult};

/// Shader stage (maps to `VkShaderStageFlagBits`).
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum ShaderStage {
    Vertex,
    Fragment,
    Compute,
    Geometry,
    TessellationControl,
    TessellationEvaluation,
}

impl ShaderStage {
    pub fn vk_flags_name(self) -> &'static str {
        match self {
            Self::Vertex                 => "VK_SHADER_STAGE_VERTEX_BIT",
            Self::Fragment               => "VK_SHADER_STAGE_FRAGMENT_BIT",
            Self::Compute                => "VK_SHADER_STAGE_COMPUTE_BIT",
            Self::Geometry               => "VK_SHADER_STAGE_GEOMETRY_BIT",
            Self::TessellationControl    => "VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT",
            Self::TessellationEvaluation => "VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT",
        }
    }
}

/// A loaded SPIR-V shader module.
#[derive(Clone, Debug)]
pub struct ShaderModule {
    pub stage: ShaderStage,
    pub entry: String,
    pub spirv: Vec<u8>,
}

impl ShaderModule {
    /// Load a shader from raw SPIR-V bytes.
    pub fn from_bytes(stage: ShaderStage, entry: impl Into<String>, spirv: Vec<u8>) -> RendererResult<Self> {
        if spirv.len() < 4 {
            return Err(RendererError::ShaderLoad("SPIR-V too short".into()));
        }
        // SPIR-V magic: 0x07230203
        let magic = u32::from_le_bytes([spirv[0], spirv[1], spirv[2], spirv[3]]);
        if magic != 0x07230203 {
            return Err(RendererError::ShaderLoad(format!("invalid SPIR-V magic: {:#010x}", magic)));
        }
        Ok(Self { stage, entry: entry.into(), spirv })
    }

    /// Load a shader from a `.spv` file path.
    pub fn from_file(stage: ShaderStage, entry: impl Into<String>, path: &std::path::Path) -> RendererResult<Self> {
        let spirv = std::fs::read(path).map_err(|e| {
            RendererError::ShaderLoad(format!("cannot read {}: {}", path.display(), e))
        })?;
        Self::from_bytes(stage, entry, spirv)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn invalid_magic_rejected() {
        let bad: Vec<u8> = vec![0xDE, 0xAD, 0xBE, 0xEF];
        assert!(ShaderModule::from_bytes(ShaderStage::Vertex, "main", bad).is_err());
    }

    #[test]
    fn valid_magic_accepted() {
        // Minimal valid SPIR-V header (magic + version + generator + bound + schema)
        let mut spirv = vec![0u8; 20];
        spirv[0..4].copy_from_slice(&0x07230203_u32.to_le_bytes());
        assert!(ShaderModule::from_bytes(ShaderStage::Vertex, "main", spirv).is_ok());
    }
}
