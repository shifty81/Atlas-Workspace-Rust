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

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn default_params_not_emissive() {
        let p = PbrMaterialParams::default();
        assert!(!p.is_emissive());
        assert_eq!(p.metallic, 0.0);
        assert_eq!(p.roughness, 0.5);
        assert_eq!(p.ao, 1.0);
    }

    #[test]
    fn emissive_intensity_makes_material_emissive() {
        let mut p = PbrMaterialParams::default();
        p.emissive_intensity = 1.0;
        assert!(p.is_emissive());
    }

    #[test]
    fn material_id_is_unique() {
        let m1 = PbrMaterial::new();
        let m2 = PbrMaterial::new();
        assert_ne!(m1.id(), m2.id());
    }

    #[test]
    fn set_and_get_name() {
        let mut m = PbrMaterial::new();
        m.set_name("Stone");
        assert_eq!(m.name(), "Stone");
    }

    #[test]
    fn set_params_stored_correctly() {
        let mut m = PbrMaterial::new();
        let mut p = PbrMaterialParams::default();
        p.metallic = 0.8;
        m.set_params(p.clone());
        assert_eq!(m.params().metallic, 0.8);
    }

    #[test]
    fn bind_and_get_texture() {
        let mut m = PbrMaterial::new();
        m.bind_texture(PbrTextureSlot::Albedo, 100, 1);
        assert!(m.has_texture(&PbrTextureSlot::Albedo));
        let binding = m.get_texture_binding(&PbrTextureSlot::Albedo).unwrap();
        assert_eq!(binding.texture_id, 100);
        assert_eq!(binding.sampler_id, 1);
    }

    #[test]
    fn unbind_texture() {
        let mut m = PbrMaterial::new();
        m.bind_texture(PbrTextureSlot::Normal, 5, 1);
        m.unbind_texture(&PbrTextureSlot::Normal);
        assert!(!m.has_texture(&PbrTextureSlot::Normal));
        assert_eq!(m.texture_binding_count(), 0);
    }

    #[test]
    fn validate_default_material() {
        let m = PbrMaterial::new();
        assert!(m.validate());
    }

    #[test]
    fn validate_fails_out_of_range_metallic() {
        let mut m = PbrMaterial::new();
        let mut p = PbrMaterialParams::default();
        p.metallic = 1.5;
        m.set_params(p);
        assert!(!m.validate());
    }
}
