use std::sync::atomic::{AtomicU32, Ordering};
use std::collections::HashMap;

static NEXT_ID: AtomicU32 = AtomicU32::new(1);

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum PbrTextureSlot { Albedo, Normal, Metallic, Roughness, Ao, Emissive, Height }

#[derive(Debug, Clone, PartialEq)]
pub enum AlphaMode { Opaque, Mask, Blend }

#[derive(Debug, Clone)]
pub struct PbrMaterialParams {
    pub albedo_color: [f32; 4],
    pub metallic: f32,
    pub roughness: f32,
    pub ao: f32,
    pub emissive_color: [f32; 3],
    pub emissive_intensity: f32,
    pub normal_scale: f32,
    pub height_scale: f32,
    pub alpha_mode: AlphaMode,
    pub alpha_cutoff: f32,
}

impl Default for PbrMaterialParams {
    fn default() -> Self {
        Self {
            albedo_color: [1.0, 1.0, 1.0, 1.0],
            metallic: 0.0, roughness: 0.5, ao: 1.0,
            emissive_color: [0.0, 0.0, 0.0], emissive_intensity: 0.0,
            normal_scale: 1.0, height_scale: 0.05,
            alpha_mode: AlphaMode::Opaque, alpha_cutoff: 0.5,
        }
    }
}

impl PbrMaterialParams {
    pub fn is_emissive(&self) -> bool { self.emissive_intensity > 0.0 }
}

#[derive(Debug, Clone)]
pub struct PbrTextureBinding {
    pub slot: PbrTextureSlot,
    pub texture_id: u32,
    pub sampler_id: u32,
}

#[derive(Debug)]
pub struct PbrMaterial {
    id: u32,
    name: String,
    params: PbrMaterialParams,
    texture_bindings: HashMap<PbrTextureSlot, PbrTextureBinding>,
}

impl PbrMaterial {
    pub fn new() -> Self {
        Self {
            id: NEXT_ID.fetch_add(1, Ordering::Relaxed),
            name: String::new(),
            params: PbrMaterialParams::default(),
            texture_bindings: HashMap::new(),
        }
    }

    pub fn set_params(&mut self, params: PbrMaterialParams) { self.params = params; }
    pub fn params(&self) -> &PbrMaterialParams { &self.params }

    pub fn bind_texture(&mut self, slot: PbrTextureSlot, texture_id: u32, sampler_id: u32) {
        self.texture_bindings.insert(slot.clone(), PbrTextureBinding { slot, texture_id, sampler_id });
    }

    pub fn unbind_texture(&mut self, slot: &PbrTextureSlot) {
        self.texture_bindings.remove(slot);
    }

    pub fn get_texture_binding(&self, slot: &PbrTextureSlot) -> Option<&PbrTextureBinding> {
        self.texture_bindings.get(slot)
    }

    pub fn has_texture(&self, slot: &PbrTextureSlot) -> bool { self.texture_bindings.contains_key(slot) }
    pub fn texture_binding_count(&self) -> u32 { self.texture_bindings.len() as u32 }
    pub fn id(&self) -> u32 { self.id }
    pub fn set_name(&mut self, name: &str) { self.name = name.into(); }
    pub fn name(&self) -> &str { &self.name }

    pub fn validate(&self) -> bool {
        let p = &self.params;
        p.metallic >= 0.0 && p.metallic <= 1.0
            && p.roughness >= 0.0 && p.roughness <= 1.0
            && p.ao >= 0.0 && p.ao <= 1.0
            && p.alpha_cutoff >= 0.0 && p.alpha_cutoff <= 1.0
    }
}

impl Default for PbrMaterial {
    fn default() -> Self { Self::new() }
}
