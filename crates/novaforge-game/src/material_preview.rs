// SPDX-License-Identifier: GPL-3.0-only
// NovaForge material preview — port of NovaForge::NovaForgeMaterialPreview.
// Copyright (C) NovaForge contributors. GNU General Public License v3.0.

use super::preview_world::{EntityId, INVALID_ENTITY_ID, NovaForgePreviewWorld};

// ── PreviewMeshType ───────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum PreviewMeshType {
    #[default]
    Sphere,
    Cube,
    Plane,
}

pub fn preview_mesh_type_name(t: PreviewMeshType) -> &'static str {
    match t {
        PreviewMeshType::Sphere => "Sphere",
        PreviewMeshType::Cube   => "Cube",
        PreviewMeshType::Plane  => "Plane",
    }
}

pub fn preview_mesh_type_tag(t: PreviewMeshType) -> &'static str {
    match t {
        PreviewMeshType::Sphere => "mesh/__preview_sphere",
        PreviewMeshType::Cube   => "mesh/__preview_cube",
        PreviewMeshType::Plane  => "mesh/__preview_plane",
    }
}

// ── MaterialParameterType ─────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum MaterialParameterType {
    #[default]
    Float,
    Vec4,
    Texture,
    Bool,
}

pub fn material_parameter_type_name(t: MaterialParameterType) -> &'static str {
    match t {
        MaterialParameterType::Float   => "Float",
        MaterialParameterType::Vec4    => "Vec4",
        MaterialParameterType::Texture => "Texture",
        MaterialParameterType::Bool    => "Bool",
    }
}

// ── MaterialParameter ─────────────────────────────────────────────────────

#[derive(Debug, Clone, Default)]
pub struct MaterialParameter {
    pub name:          String,
    pub param_type:    MaterialParameterType,
    pub value:         String,
    pub default_value: String,
}

// ── MaterialPreviewDescriptor ─────────────────────────────────────────────

#[derive(Debug, Clone, Default)]
pub struct MaterialPreviewDescriptor {
    pub material_path: String,
    pub shader_tag:    String,
    pub preview_mesh:  PreviewMeshType,
    pub parameters:    Vec<MaterialParameter>,
}

// ── NovaForgeMaterialPreview ──────────────────────────────────────────────

pub struct NovaForgeMaterialPreview {
    descriptor:     MaterialPreviewDescriptor,
    last_applied:   MaterialPreviewDescriptor,
    world:          NovaForgePreviewWorld,
    preview_entity: EntityId,
    dirty:          bool,
}

impl NovaForgeMaterialPreview {
    pub fn new() -> Self {
        Self {
            descriptor:     MaterialPreviewDescriptor::default(),
            last_applied:   MaterialPreviewDescriptor::default(),
            world:          NovaForgePreviewWorld::new(),
            preview_entity: INVALID_ENTITY_ID,
            dirty:          false,
        }
    }

    // ── Material binding ──────────────────────────────────────────────────

    pub fn bind_material(&mut self, d: MaterialPreviewDescriptor) {
        self.descriptor = d;
        self.dirty = true;
        self.rebuild_preview();
    }

    pub fn clear_material(&mut self) {
        self.descriptor = MaterialPreviewDescriptor::default();
        if self.preview_entity != INVALID_ENTITY_ID {
            self.world.destroy_entity(self.preview_entity);
            self.preview_entity = INVALID_ENTITY_ID;
        }
        self.dirty = true;
    }

    pub fn has_material(&self) -> bool { !self.descriptor.material_path.is_empty() }
    pub fn descriptor(&self) -> &MaterialPreviewDescriptor { &self.descriptor }

    // ── Preview mesh ──────────────────────────────────────────────────────

    pub fn set_preview_mesh(&mut self, m: PreviewMeshType) -> bool {
        self.descriptor.preview_mesh = m;
        if self.preview_entity != INVALID_ENTITY_ID {
            self.world.set_mesh_tag(self.preview_entity, preview_mesh_type_tag(m));
        }
        self.dirty = true;
        true
    }

    pub fn preview_mesh(&self) -> PreviewMeshType { self.descriptor.preview_mesh }

    // ── Shader tag ────────────────────────────────────────────────────────

    pub fn set_shader_tag(&mut self, tag: &str) -> bool {
        self.descriptor.shader_tag = tag.to_string();
        self.dirty = true;
        true
    }

    pub fn shader_tag(&self) -> &str { &self.descriptor.shader_tag }

    // ── Parameters ────────────────────────────────────────────────────────

    pub fn set_parameter(&mut self, name: &str, value: &str, param_type: MaterialParameterType) -> bool {
        if let Some(p) = self.descriptor.parameters.iter_mut().find(|p| p.name == name) {
            p.value = value.to_string();
            p.param_type = param_type;
        } else {
            self.descriptor.parameters.push(MaterialParameter {
                name:          name.to_string(),
                param_type,
                value:         value.to_string(),
                default_value: value.to_string(),
            });
        }
        self.dirty = true;
        true
    }

    pub fn remove_parameter(&mut self, name: &str) -> bool {
        if let Some(pos) = self.descriptor.parameters.iter().position(|p| p.name == name) {
            self.descriptor.parameters.remove(pos);
            self.dirty = true;
            true
        } else {
            false
        }
    }

    pub fn reset_parameter_to_default(&mut self, name: &str) -> bool {
        if let Some(p) = self.descriptor.parameters.iter_mut().find(|p| p.name == name) {
            p.value = p.default_value.clone();
            self.dirty = true;
            true
        } else {
            false
        }
    }

    pub fn reset_all_parameters_to_default(&mut self) -> bool {
        for p in &mut self.descriptor.parameters {
            p.value = p.default_value.clone();
        }
        self.dirty = true;
        true
    }

    pub fn get_parameter(&self, name: &str, fallback: &str) -> String {
        self.descriptor.parameters.iter()
            .find(|p| p.name == name)
            .map(|p| p.value.clone())
            .unwrap_or_else(|| fallback.to_string())
    }

    pub fn parameters(&self) -> &[MaterialParameter] { &self.descriptor.parameters }
    pub fn parameter_count(&self) -> u32 { self.descriptor.parameters.len() as u32 }

    // ── Dirty tracking ────────────────────────────────────────────────────

    pub fn is_dirty(&self) -> bool { self.dirty }
    pub fn clear_dirty(&mut self) { self.dirty = false; }

    pub fn apply(&mut self) -> bool {
        self.last_applied = self.descriptor.clone();
        self.dirty = false;
        true
    }

    pub fn revert(&mut self) -> bool {
        self.descriptor = self.last_applied.clone();
        self.dirty = false;
        true
    }

    // ── Properties ───────────────────────────────────────────────────────

    pub fn properties(&self) -> Vec<(String, String)> {
        let d = &self.descriptor;
        let mut props = vec![
            ("materialPath".into(), d.material_path.clone()),
            ("shaderTag".into(),    d.shader_tag.clone()),
            ("previewMesh".into(),  preview_mesh_type_name(d.preview_mesh).to_string()),
        ];
        for p in &d.parameters {
            props.push((format!("param.{}", p.name), p.value.clone()));
        }
        props
    }

    pub fn preview_world(&self) -> &NovaForgePreviewWorld { &self.world }

    // ── Private ───────────────────────────────────────────────────────────

    fn rebuild_preview(&mut self) {
        if self.preview_entity != INVALID_ENTITY_ID {
            self.world.destroy_entity(self.preview_entity);
        }
        self.preview_entity = self.world.create_entity("__mat_preview", INVALID_ENTITY_ID);
        let mesh_tag = preview_mesh_type_tag(self.descriptor.preview_mesh);
        self.world.set_mesh_tag(self.preview_entity, mesh_tag);
        self.world.set_material_tag(self.preview_entity, &self.descriptor.shader_tag);
    }
}

impl Default for NovaForgeMaterialPreview {
    fn default() -> Self { Self::new() }
}

// ── Tests ─────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    fn desc(path: &str) -> MaterialPreviewDescriptor {
        MaterialPreviewDescriptor { material_path: path.to_string(), ..Default::default() }
    }

    #[test]
    fn bind_material_sets_descriptor() {
        let mut mp = NovaForgeMaterialPreview::new();
        mp.bind_material(desc("mat/rock.mat"));
        assert_eq!(mp.descriptor().material_path, "mat/rock.mat");
    }

    #[test]
    fn has_material_true_when_bound() {
        let mut mp = NovaForgeMaterialPreview::new();
        assert!(!mp.has_material());
        mp.bind_material(desc("x.mat"));
        assert!(mp.has_material());
    }

    #[test]
    fn clear_material() {
        let mut mp = NovaForgeMaterialPreview::new();
        mp.bind_material(desc("y.mat"));
        mp.clear_material();
        assert!(!mp.has_material());
    }

    #[test]
    fn set_preview_mesh_updates_tag() {
        let mut mp = NovaForgeMaterialPreview::new();
        mp.bind_material(desc("z.mat"));
        assert!(mp.set_preview_mesh(PreviewMeshType::Cube));
        assert_eq!(mp.preview_mesh(), PreviewMeshType::Cube);
    }

    #[test]
    fn set_and_get_parameter() {
        let mut mp = NovaForgeMaterialPreview::new();
        mp.set_parameter("roughness", "0.8", MaterialParameterType::Float);
        assert_eq!(mp.get_parameter("roughness", "0"), "0.8");
        assert_eq!(mp.parameter_count(), 1);
    }

    #[test]
    fn remove_parameter() {
        let mut mp = NovaForgeMaterialPreview::new();
        mp.set_parameter("metallic", "1.0", MaterialParameterType::Float);
        assert!(mp.remove_parameter("metallic"));
        assert_eq!(mp.parameter_count(), 0);
        assert!(!mp.remove_parameter("metallic"));
    }

    #[test]
    fn reset_all_parameters_to_default() {
        let mut mp = NovaForgeMaterialPreview::new();
        mp.set_parameter("p", "default_val", MaterialParameterType::Float);
        // override value
        if let Some(param) = mp.descriptor.parameters.iter_mut().find(|p| p.name == "p") {
            param.value = "changed".to_string();
        }
        mp.reset_all_parameters_to_default();
        assert_eq!(mp.get_parameter("p", ""), "default_val");
    }

    #[test]
    fn apply_and_revert() {
        let mut mp = NovaForgeMaterialPreview::new();
        mp.bind_material(desc("m.mat"));
        mp.apply();
        mp.set_shader_tag("shader/pbr");
        mp.revert();
        assert_eq!(mp.shader_tag(), "");
    }

    #[test]
    fn properties_has_base_entries() {
        let mp = NovaForgeMaterialPreview::new();
        let props = mp.properties();
        assert!(props.len() >= 3);
        assert!(props.iter().any(|(k, _)| k == "materialPath"));
        assert!(props.iter().any(|(k, _)| k == "shaderTag"));
        assert!(props.iter().any(|(k, _)| k == "previewMesh"));
    }
}
