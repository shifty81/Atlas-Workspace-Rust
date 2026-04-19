// SPDX-License-Identifier: GPL-3.0-only
// NovaForge asset preview — port of NovaForge::NovaForgeAssetPreview.
// Copyright (C) NovaForge contributors. GNU General Public License v3.0.

use super::preview_world::{
    EntityId, INVALID_ENTITY_ID, NovaForgePreviewWorld, PreviewTransform, PreviewVec3,
};

// ── ColliderShape ─────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum ColliderShape {
    #[default]
    Box_,
    Sphere,
    Capsule,
    ConvexHull,
    TriangleMesh,
}

pub fn collider_shape_name(s: ColliderShape) -> &'static str {
    match s {
        ColliderShape::Box_         => "Box",
        ColliderShape::Sphere       => "Sphere",
        ColliderShape::Capsule      => "Capsule",
        ColliderShape::ConvexHull   => "ConvexHull",
        ColliderShape::TriangleMesh => "TriangleMesh",
    }
}

// ── ColliderDescriptor ────────────────────────────────────────────────────

#[derive(Debug, Clone)]
pub struct ColliderDescriptor {
    pub shape:      ColliderShape,
    pub extents:    PreviewVec3,
    pub radius:     f32,
    pub is_trigger: bool,
    pub tag:        String,
}

impl Default for ColliderDescriptor {
    fn default() -> Self {
        Self {
            shape:      ColliderShape::Box_,
            extents:    PreviewVec3 { x: 1.0, y: 1.0, z: 1.0 },
            radius:     0.5,
            is_trigger: false,
            tag:        String::new(),
        }
    }
}

// ── SocketDescriptor ──────────────────────────────────────────────────────

#[derive(Debug, Clone, Default)]
pub struct SocketDescriptor {
    pub name:            String,
    pub local_transform: PreviewTransform,
    pub socket_type:     String,
}

// ── AnchorDescriptor ──────────────────────────────────────────────────────

#[derive(Debug, Clone, Default)]
pub struct AnchorDescriptor {
    pub name:            String,
    pub local_transform: PreviewTransform,
    pub anchor_type:     String,
}

// ── AssetPcgMetadata ──────────────────────────────────────────────────────

#[derive(Debug, Clone)]
pub struct AssetPcgMetadata {
    pub placement_tag:    String,
    pub generation_tags:  Vec<String>,
    pub min_scale:        f32,
    pub max_scale:        f32,
    pub density:          f32,
    pub allow_rotation:   bool,
    pub align_to_normal:  bool,
    pub exclusion_group:  String,
}

impl Default for AssetPcgMetadata {
    fn default() -> Self {
        Self {
            placement_tag:   String::new(),
            generation_tags: Vec::new(),
            min_scale:       0.8,
            max_scale:       1.2,
            density:         1.0,
            allow_rotation:  true,
            align_to_normal: false,
            exclusion_group: String::new(),
        }
    }
}

// ── AssetPreviewDescriptor ────────────────────────────────────────────────

#[derive(Debug, Clone, Default)]
pub struct AssetPreviewDescriptor {
    pub asset_path:     String,
    pub mesh_tag:       String,
    pub material_tag:   String,
    pub attachment_tag: String,
    pub transform:      PreviewTransform,
    pub collider:       ColliderDescriptor,
    pub sockets:        Vec<SocketDescriptor>,
    pub anchors:        Vec<AnchorDescriptor>,
    pub pcg_metadata:   AssetPcgMetadata,
}

// ── NovaForgeAssetPreview ─────────────────────────────────────────────────

pub struct NovaForgeAssetPreview {
    descriptor:        AssetPreviewDescriptor,
    last_applied:      AssetPreviewDescriptor,
    world:             NovaForgePreviewWorld,
    preview_entity:    EntityId,
    dirty:             bool,
    has_pcg_service:   bool,
    pcg_regen_triggers: u32,
}

impl NovaForgeAssetPreview {
    pub fn new() -> Self {
        Self {
            descriptor:         AssetPreviewDescriptor::default(),
            last_applied:       AssetPreviewDescriptor::default(),
            world:              NovaForgePreviewWorld::new(),
            preview_entity:     INVALID_ENTITY_ID,
            dirty:              false,
            has_pcg_service:    false,
            pcg_regen_triggers: 0,
        }
    }

    // ── Asset binding ─────────────────────────────────────────────────────

    pub fn bind_asset(&mut self, descriptor: AssetPreviewDescriptor) {
        self.descriptor = descriptor;
        self.dirty = true;
        self.rebuild_preview();
    }

    pub fn clear_asset(&mut self) {
        self.descriptor = AssetPreviewDescriptor::default();
        if self.preview_entity != INVALID_ENTITY_ID {
            self.world.destroy_entity(self.preview_entity);
            self.preview_entity = INVALID_ENTITY_ID;
        }
        self.dirty = true;
    }

    pub fn has_asset(&self) -> bool { !self.descriptor.asset_path.is_empty() }
    pub fn descriptor(&self) -> &AssetPreviewDescriptor { &self.descriptor }

    // ── Transform ─────────────────────────────────────────────────────────

    pub fn set_transform(&mut self, t: PreviewTransform) -> bool {
        self.descriptor.transform = t;
        if self.preview_entity != INVALID_ENTITY_ID {
            self.world.set_transform(self.preview_entity, t);
        }
        self.dirty = true;
        true
    }

    // ── Mesh / Material / Attachment ──────────────────────────────────────

    pub fn set_mesh_tag(&mut self, tag: &str) -> bool {
        self.descriptor.mesh_tag = tag.to_string();
        if self.preview_entity != INVALID_ENTITY_ID {
            self.world.set_mesh_tag(self.preview_entity, tag);
        }
        self.dirty = true;
        true
    }

    pub fn set_material_tag(&mut self, tag: &str) -> bool {
        self.descriptor.material_tag = tag.to_string();
        if self.preview_entity != INVALID_ENTITY_ID {
            self.world.set_material_tag(self.preview_entity, tag);
        }
        self.dirty = true;
        true
    }

    pub fn set_attachment_tag(&mut self, tag: &str) -> bool {
        self.descriptor.attachment_tag = tag.to_string();
        self.dirty = true;
        true
    }

    // ── Collider ──────────────────────────────────────────────────────────

    pub fn set_collider(&mut self, c: ColliderDescriptor) -> bool {
        self.descriptor.collider = c;
        self.dirty = true;
        true
    }

    pub fn set_collider_shape(&mut self, s: ColliderShape) -> bool {
        self.descriptor.collider.shape = s;
        self.dirty = true;
        true
    }

    pub fn set_collider_extents(&mut self, e: PreviewVec3) -> bool {
        self.descriptor.collider.extents = e;
        self.dirty = true;
        true
    }

    pub fn set_collider_is_trigger(&mut self, b: bool) -> bool {
        self.descriptor.collider.is_trigger = b;
        self.dirty = true;
        true
    }

    pub fn set_collider_tag(&mut self, tag: &str) -> bool {
        self.descriptor.collider.tag = tag.to_string();
        self.dirty = true;
        true
    }

    pub fn collider(&self) -> &ColliderDescriptor { &self.descriptor.collider }

    // ── Sockets ───────────────────────────────────────────────────────────

    pub fn add_socket(&mut self, s: SocketDescriptor) -> bool {
        if self.descriptor.sockets.iter().any(|x| x.name == s.name) { return false; }
        self.descriptor.sockets.push(s);
        self.dirty = true;
        true
    }

    pub fn remove_socket(&mut self, name: &str) -> bool {
        if let Some(pos) = self.descriptor.sockets.iter().position(|x| x.name == name) {
            self.descriptor.sockets.remove(pos);
            self.dirty = true;
            true
        } else {
            false
        }
    }

    pub fn set_socket_transform(&mut self, name: &str, t: PreviewTransform) -> bool {
        if let Some(s) = self.descriptor.sockets.iter_mut().find(|x| x.name == name) {
            s.local_transform = t;
            self.dirty = true;
            true
        } else {
            false
        }
    }

    pub fn sockets(&self) -> &[SocketDescriptor] { &self.descriptor.sockets }
    pub fn socket_count(&self) -> u32 { self.descriptor.sockets.len() as u32 }

    // ── Anchors ───────────────────────────────────────────────────────────

    pub fn add_anchor(&mut self, a: AnchorDescriptor) -> bool {
        if self.descriptor.anchors.iter().any(|x| x.name == a.name) { return false; }
        self.descriptor.anchors.push(a);
        self.dirty = true;
        true
    }

    pub fn remove_anchor(&mut self, name: &str) -> bool {
        if let Some(pos) = self.descriptor.anchors.iter().position(|x| x.name == name) {
            self.descriptor.anchors.remove(pos);
            self.dirty = true;
            true
        } else {
            false
        }
    }

    pub fn set_anchor_transform(&mut self, name: &str, t: PreviewTransform) -> bool {
        if let Some(a) = self.descriptor.anchors.iter_mut().find(|x| x.name == name) {
            a.local_transform = t;
            self.dirty = true;
            true
        } else {
            false
        }
    }

    pub fn anchors(&self) -> &[AnchorDescriptor] { &self.descriptor.anchors }
    pub fn anchor_count(&self) -> u32 { self.descriptor.anchors.len() as u32 }

    // ── PCG metadata ──────────────────────────────────────────────────────

    pub fn set_pcg_metadata(&mut self, m: AssetPcgMetadata) -> bool {
        self.descriptor.pcg_metadata = m;
        self.dirty = true;
        true
    }

    pub fn set_placement_tag(&mut self, tag: &str) -> bool {
        self.descriptor.pcg_metadata.placement_tag = tag.to_string();
        self.dirty = true;
        true
    }

    pub fn add_generation_tag(&mut self, tag: &str) -> bool {
        if self.descriptor.pcg_metadata.generation_tags.iter().any(|t| t == tag) { return false; }
        self.descriptor.pcg_metadata.generation_tags.push(tag.to_string());
        self.dirty = true;
        true
    }

    pub fn remove_generation_tag(&mut self, tag: &str) -> bool {
        if let Some(pos) = self.descriptor.pcg_metadata.generation_tags.iter().position(|t| t == tag) {
            self.descriptor.pcg_metadata.generation_tags.remove(pos);
            self.dirty = true;
            true
        } else {
            false
        }
    }

    pub fn set_pcg_scale_range(&mut self, min: f32, max: f32) -> bool {
        if min > max { return false; }
        self.descriptor.pcg_metadata.min_scale = min;
        self.descriptor.pcg_metadata.max_scale = max;
        self.dirty = true;
        true
    }

    pub fn set_pcg_density(&mut self, d: f32) -> bool {
        if d < 0.0 { return false; }
        self.descriptor.pcg_metadata.density = d;
        self.dirty = true;
        true
    }

    pub fn set_pcg_exclusion_group(&mut self, g: &str) -> bool {
        self.descriptor.pcg_metadata.exclusion_group = g.to_string();
        self.dirty = true;
        true
    }

    pub fn pcg_metadata(&self) -> &AssetPcgMetadata { &self.descriptor.pcg_metadata }

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

    // ── PCG service ───────────────────────────────────────────────────────

    pub fn attach_pcg_preview_service(&mut self) { self.has_pcg_service = true; }
    pub fn detach_pcg_preview_service(&mut self) { self.has_pcg_service = false; }
    pub fn has_pcg_preview_service(&self) -> bool { self.has_pcg_service }
    pub fn pcg_regen_trigger_count(&self) -> u32 { self.pcg_regen_triggers }

    // ── Notify variants (trigger PCG regen) ──────────────────────────────

    pub fn set_placement_tag_and_notify(&mut self, tag: &str) -> bool {
        let ok = self.set_placement_tag(tag);
        if ok { self.trigger_pcg_regen(); }
        ok
    }

    pub fn add_generation_tag_and_notify(&mut self, tag: &str) -> bool {
        let ok = self.add_generation_tag(tag);
        if ok { self.trigger_pcg_regen(); }
        ok
    }

    pub fn remove_generation_tag_and_notify(&mut self, tag: &str) -> bool {
        let ok = self.remove_generation_tag(tag);
        if ok { self.trigger_pcg_regen(); }
        ok
    }

    pub fn set_pcg_scale_range_and_notify(&mut self, min: f32, max: f32) -> bool {
        let ok = self.set_pcg_scale_range(min, max);
        if ok { self.trigger_pcg_regen(); }
        ok
    }

    pub fn set_pcg_density_and_notify(&mut self, d: f32) -> bool {
        let ok = self.set_pcg_density(d);
        if ok { self.trigger_pcg_regen(); }
        ok
    }

    pub fn set_pcg_exclusion_group_and_notify(&mut self, g: &str) -> bool {
        let ok = self.set_pcg_exclusion_group(g);
        if ok { self.trigger_pcg_regen(); }
        ok
    }

    // ── Properties ───────────────────────────────────────────────────────

    pub fn properties(&self) -> Vec<(String, String)> {
        let d = &self.descriptor;
        let c = &d.collider;
        let p = &d.pcg_metadata;
        vec![
            ("assetPath".into(),       d.asset_path.clone()),
            ("meshTag".into(),         d.mesh_tag.clone()),
            ("materialTag".into(),     d.material_tag.clone()),
            ("attachmentTag".into(),   d.attachment_tag.clone()),
            ("position.x".into(),      format!("{:.3}", d.transform.position.x)),
            ("position.y".into(),      format!("{:.3}", d.transform.position.y)),
            ("position.z".into(),      format!("{:.3}", d.transform.position.z)),
            ("colliderShape".into(),   collider_shape_name(c.shape).to_string()),
            ("colliderExtents.x".into(), format!("{:.3}", c.extents.x)),
            ("colliderExtents.y".into(), format!("{:.3}", c.extents.y)),
            ("colliderExtents.z".into(), format!("{:.3}", c.extents.z)),
            ("colliderRadius".into(),  format!("{:.3}", c.radius)),
            ("isTrigger".into(),       c.is_trigger.to_string()),
            ("socketCount".into(),     d.sockets.len().to_string()),
            ("anchorCount".into(),     d.anchors.len().to_string()),
            ("placementTag".into(),    p.placement_tag.clone()),
            ("minScale".into(),        format!("{:.3}", p.min_scale)),
            ("maxScale".into(),        format!("{:.3}", p.max_scale)),
            ("density".into(),         format!("{:.3}", p.density)),
            ("exclusionGroup".into(),  p.exclusion_group.clone()),
        ]
    }

    pub fn preview_world(&self) -> &NovaForgePreviewWorld { &self.world }

    // ── Private ───────────────────────────────────────────────────────────

    fn rebuild_preview(&mut self) {
        if self.preview_entity != INVALID_ENTITY_ID {
            self.world.destroy_entity(self.preview_entity);
        }
        self.preview_entity = self.world.create_entity("__asset_preview", INVALID_ENTITY_ID);
        self.world.set_transform(self.preview_entity, self.descriptor.transform);
        self.world.set_mesh_tag(self.preview_entity, &self.descriptor.mesh_tag);
        self.world.set_material_tag(self.preview_entity, &self.descriptor.material_tag);
    }

    fn trigger_pcg_regen(&mut self) {
        self.pcg_regen_triggers += 1;
    }
}

impl Default for NovaForgeAssetPreview {
    fn default() -> Self { Self::new() }
}

// ── Tests ─────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    fn desc(path: &str) -> AssetPreviewDescriptor {
        AssetPreviewDescriptor { asset_path: path.to_string(), ..Default::default() }
    }

    #[test]
    fn bind_asset_sets_descriptor() {
        let mut ap = NovaForgeAssetPreview::new();
        ap.bind_asset(desc("assets/tree.gltf"));
        assert_eq!(ap.descriptor().asset_path, "assets/tree.gltf");
    }

    #[test]
    fn has_asset_returns_true_when_bound() {
        let mut ap = NovaForgeAssetPreview::new();
        assert!(!ap.has_asset());
        ap.bind_asset(desc("x.gltf"));
        assert!(ap.has_asset());
    }

    #[test]
    fn clear_asset_removes_path() {
        let mut ap = NovaForgeAssetPreview::new();
        ap.bind_asset(desc("y.gltf"));
        ap.clear_asset();
        assert!(!ap.has_asset());
    }

    #[test]
    fn set_mesh_and_material_tag() {
        let mut ap = NovaForgeAssetPreview::new();
        ap.bind_asset(desc("z.gltf"));
        assert!(ap.set_mesh_tag("mesh/rock"));
        assert!(ap.set_material_tag("mat/stone"));
        assert_eq!(ap.descriptor().mesh_tag, "mesh/rock");
        assert_eq!(ap.descriptor().material_tag, "mat/stone");
    }

    #[test]
    fn add_and_remove_socket() {
        let mut ap = NovaForgeAssetPreview::new();
        let s = SocketDescriptor { name: "hand_r".to_string(), ..Default::default() };
        assert!(ap.add_socket(s.clone()));
        assert_eq!(ap.socket_count(), 1);
        assert!(!ap.add_socket(s)); // duplicate
        assert!(ap.remove_socket("hand_r"));
        assert_eq!(ap.socket_count(), 0);
    }

    #[test]
    fn add_and_remove_anchor() {
        let mut ap = NovaForgeAssetPreview::new();
        let a = AnchorDescriptor { name: "root".to_string(), ..Default::default() };
        assert!(ap.add_anchor(a));
        assert_eq!(ap.anchor_count(), 1);
        assert!(ap.remove_anchor("root"));
        assert_eq!(ap.anchor_count(), 0);
    }

    #[test]
    fn pcg_regen_triggered_by_notify() {
        let mut ap = NovaForgeAssetPreview::new();
        ap.set_placement_tag_and_notify("tag/forest");
        ap.add_generation_tag_and_notify("gen/oak");
        ap.set_pcg_density_and_notify(2.0);
        assert_eq!(ap.pcg_regen_trigger_count(), 3);
    }

    #[test]
    fn apply_and_revert() {
        let mut ap = NovaForgeAssetPreview::new();
        ap.bind_asset(desc("a.gltf"));
        ap.apply();
        ap.set_mesh_tag("mesh/changed");
        ap.revert();
        assert_eq!(ap.descriptor().mesh_tag, "");
    }

    #[test]
    fn properties_has_20_entries() {
        let ap = NovaForgeAssetPreview::new();
        assert_eq!(ap.properties().len(), 20);
    }

    #[test]
    fn pcg_scale_range_invalid_rejected() {
        let mut ap = NovaForgeAssetPreview::new();
        assert!(!ap.set_pcg_scale_range(2.0, 1.0));
        assert!(ap.set_pcg_scale_range(0.5, 1.5));
    }
}
