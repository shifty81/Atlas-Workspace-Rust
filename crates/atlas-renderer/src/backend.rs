#[derive(Debug, Clone, PartialEq)]
pub enum RenderApi { None, OpenGL, Vulkan, Dx11, Null }

#[derive(Debug, Clone)]
pub struct RendererCapabilities {
    pub bindless_textures: bool,
    pub compute_shaders: bool,
    pub ray_tracing: bool,
    pub max_msaa_samples: u32,
    pub hdr_swapchain: bool,
    pub max_texture_size: u32,
    pub max_uniform_buffers: u32,
    pub device_name: String,
    pub driver_version: String,
}

impl Default for RendererCapabilities {
    fn default() -> Self {
        Self {
            bindless_textures: false, compute_shaders: false, ray_tracing: false,
            max_msaa_samples: 1, hdr_swapchain: false, max_texture_size: 4096,
            max_uniform_buffers: 16, device_name: String::new(), driver_version: String::new(),
        }
    }
}

pub trait RendererBackend: Send + Sync {
    fn init(&mut self);
    fn shutdown(&mut self);
    fn begin_frame(&mut self);
    fn end_frame(&mut self);
    fn set_viewport(&mut self, width: i32, height: i32);
    fn get_api(&self) -> RenderApi;
    fn capabilities(&self) -> &RendererCapabilities;
}

pub struct NullRendererBackend {
    caps: RendererCapabilities,
}

impl Default for NullRendererBackend {
    fn default() -> Self { Self { caps: RendererCapabilities::default() } }
}

impl NullRendererBackend {
    pub fn new() -> Self { Self::default() }
}

impl RendererBackend for NullRendererBackend {
    fn init(&mut self) {}
    fn shutdown(&mut self) {}
    fn begin_frame(&mut self) {}
    fn end_frame(&mut self) {}
    fn set_viewport(&mut self, _width: i32, _height: i32) {}
    fn get_api(&self) -> RenderApi { RenderApi::Null }
    fn capabilities(&self) -> &RendererCapabilities { &self.caps }
}
