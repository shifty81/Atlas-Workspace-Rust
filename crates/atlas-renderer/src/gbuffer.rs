#[derive(Debug, Clone, PartialEq)]
pub enum GBufferFormat { Rgba8, Rgba16F, Rgba32F, Depth24Stencil8, Depth32F, Rg16F, R8 }

#[derive(Debug, Clone)]
pub struct GBufferAttachment {
    pub name: String,
    pub format: GBufferFormat,
    pub width: u32,
    pub height: u32,
    pub id: u32,
}

#[derive(Debug, Clone, Default)]
pub struct GBufferConfig {
    pub width: u32,
    pub height: u32,
    pub attachments: Vec<GBufferAttachment>,
    pub enable_depth: bool,
    pub enable_stencil: bool,
}

#[derive(Debug, Default)]
pub struct GBuffer {
    config: GBufferConfig,
    initialized: bool,
    bound: bool,
}

impl GBuffer {
    pub fn new() -> Self { Self::default() }

    pub fn default_pbr_config(w: u32, h: u32) -> GBufferConfig {
        GBufferConfig {
            width: w,
            height: h,
            attachments: vec![
                GBufferAttachment { name: "position".into(), format: GBufferFormat::Rgba32F, width: w, height: h, id: 0 },
                GBufferAttachment { name: "normal".into(), format: GBufferFormat::Rgba16F, width: w, height: h, id: 1 },
                GBufferAttachment { name: "albedo".into(), format: GBufferFormat::Rgba8, width: w, height: h, id: 2 },
                GBufferAttachment { name: "metallic_roughness".into(), format: GBufferFormat::Rg16F, width: w, height: h, id: 3 },
                GBufferAttachment { name: "depth".into(), format: GBufferFormat::Depth24Stencil8, width: w, height: h, id: 4 },
            ],
            enable_depth: true,
            enable_stencil: true,
        }
    }

    pub fn init(&mut self, config: GBufferConfig) {
        self.config = config;
        self.initialized = true;
    }

    pub fn shutdown(&mut self) {
        self.initialized = false;
        self.bound = false;
    }

    pub fn bind(&mut self) { self.bound = true; }
    pub fn unbind(&mut self) { self.bound = false; }

    pub fn resize(&mut self, w: u32, h: u32) {
        self.config.width = w;
        self.config.height = h;
        for att in &mut self.config.attachments {
            att.width = w;
            att.height = h;
        }
    }

    pub fn get_attachment(&self, name: &str) -> Option<&GBufferAttachment> {
        self.config.attachments.iter().find(|a| a.name == name)
    }

    pub fn attachment_count(&self) -> u32 { self.config.attachments.len() as u32 }
    pub fn width(&self) -> u32 { self.config.width }
    pub fn height(&self) -> u32 { self.config.height }
    pub fn is_initialized(&self) -> bool { self.initialized }
    pub fn is_bound(&self) -> bool { self.bound }
}
