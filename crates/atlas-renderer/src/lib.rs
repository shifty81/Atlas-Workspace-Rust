//! # Atlas Renderer
//!
//! Vulkan rendering backend for Atlas Workspace.
//!
//! ## Architecture
//!
//! | Module | Purpose |
//! |--------|---------|
//! | [`context`]      | Vulkan instance, device, queues, surface |
//! | [`swapchain`]    | Swapchain + image views + render pass + framebuffers |
//! | [`frame`]        | Per-frame command buffers + GPU sync (semaphores/fences) |
//! | [`render_loop`]  | Main acquire → record → present loop |
//! | [`pipeline`]     | Graphics pipeline builder |
//! | [`buffer`]       | GPU buffer wrappers (vertex, index, uniform) |
//! | [`texture`]      | Image + sampler |
//! | [`shader`]       | SPIR-V shader loading |
//! | [`render_pass`]  | Render pass / attachment descriptions |
//! | [`viewport`]     | Camera and viewport state |
//! | [`types`]        | Shared renderer types |

pub mod buffer;
pub mod context;
pub mod frame;
pub mod pipeline;
pub mod render_loop;
pub mod render_pass;
pub mod shader;
pub mod swapchain;
pub mod texture;
pub mod types;
pub mod viewport;
pub mod backend;
pub mod instanced_renderer;
pub mod shader_ir;
pub mod gbuffer;
pub mod pbr_material;
pub mod post_process;
pub mod shadow_map;
pub mod spatial_hash;
pub mod terrain_mesh;

pub use context::{VulkanContext, QueueFamilyIndices};
pub use types::{RenderConfig, RendererError, RendererResult};
pub use viewport::{Camera, Viewport};
pub use backend::{RenderApi, RendererCapabilities, RendererBackend, NullRendererBackend};
pub use instanced_renderer::{InstanceData, InstanceBatch, InstancedRenderer};
pub use shader_ir::{IrShaderStage, ShaderOp, ShaderInstruction, ShaderUniform, ShaderIo, ShaderIrModule, ShaderIrCompiler};
pub use gbuffer::{GBufferFormat, GBufferAttachment, GBufferConfig, GBuffer};
pub use pbr_material::{PbrTextureSlot, AlphaMode, PbrMaterialParams, PbrTextureBinding, PbrMaterial};
pub use post_process::{PostProcessEffect, ToneMapOperator, BloomSettings, ToneMappingSettings, PostProcessSettings, PostProcessPipeline};
pub use shadow_map::{ShadowCascade, LightDirection, ShadowMapConfig, ShadowMap};
pub use spatial_hash::{SpatialEntity, SpatialHash};
pub use swapchain::{Swapchain, SwapchainConfig, PresentMode};
pub use frame::{FrameCommands, FrameSync};
pub use render_loop::{RenderLoop, ClearColor, UiPaintData};
pub use terrain_mesh::{TerrainVertex, TerrainMesh};
