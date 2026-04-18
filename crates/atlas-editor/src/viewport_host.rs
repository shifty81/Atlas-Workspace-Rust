//! [`ViewportHost`] — contract for hosting named rendering viewports (M14).
//!
//! The `ViewportHost` trait defines the interface a rendering surface must
//! implement to plug into the workspace shell.  `ViewportRegistry` manages
//! multiple named viewport slots, allowing the workspace to route rendering
//! to whichever viewport is currently active.

use std::collections::HashMap;

// ── ViewportHost trait ────────────────────────────────────────────────────────

/// The kind of content a viewport renders.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum ViewportKind {
    /// 3-D scene view (default editor viewport).
    Scene,
    /// 2-D sprite / UI preview.
    Sprite,
    /// Material / shader preview sphere.
    Material,
    /// Animation preview.
    Animation,
    /// Custom / user-defined.
    Custom,
}

/// Current rendering mode of a viewport.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ViewportRenderMode {
    /// Fully shaded PBR rendering.
    Lit,
    /// Diffuse only (no lighting computation).
    Unlit,
    /// Wireframe overlay.
    Wireframe,
    /// Solid shading without textures.
    Solid,
}

impl Default for ViewportRenderMode {
    fn default() -> Self { Self::Lit }
}

/// Describes the current size of a viewport in physical pixels.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct ViewportSize {
    pub width:  u32,
    pub height: u32,
}

impl ViewportSize {
    pub fn new(width: u32, height: u32) -> Self { Self { width, height } }
    pub fn aspect_ratio(&self) -> f32 {
        if self.height == 0 { 1.0 } else { self.width as f32 / self.height as f32 }
    }
}

/// Contract every hosted viewport must satisfy.
pub trait ViewportHost: Send + Sync {
    /// Unique stable identifier (e.g. `"scene"`, `"material_preview"`).
    fn id(&self) -> &str;

    /// Human-readable display name shown in tabs.
    fn title(&self) -> &str;

    /// What kind of content this viewport renders.
    fn kind(&self) -> ViewportKind;

    /// Called when the host requests a resize.
    fn resize(&mut self, size: ViewportSize);

    /// Current viewport size.
    fn size(&self) -> ViewportSize;

    /// Current render mode.
    fn render_mode(&self) -> ViewportRenderMode;

    /// Set the render mode.
    fn set_render_mode(&mut self, mode: ViewportRenderMode);

    /// Whether the viewport is currently visible / active.
    fn is_visible(&self) -> bool;

    /// Set visibility.
    fn set_visible(&mut self, visible: bool);
}

// ── ViewportRegistry ──────────────────────────────────────────────────────────

/// Manages a set of named [`ViewportHost`] slots.
///
/// The registry keeps track of which viewport is currently focused and routes
/// resize / mode-change requests to the correct host.
pub struct ViewportRegistry {
    viewports: HashMap<String, Box<dyn ViewportHost>>,
    /// The id of the currently focused viewport, if any.
    active_id: Option<String>,
    /// Order of registration (for deterministic iteration).
    order:     Vec<String>,
}

impl Default for ViewportRegistry {
    fn default() -> Self { Self::new() }
}

impl ViewportRegistry {
    pub fn new() -> Self {
        Self {
            viewports: HashMap::new(),
            active_id: None,
            order:     Vec::new(),
        }
    }

    /// Register a viewport.  Replaces any existing viewport with the same id.
    pub fn register(&mut self, vp: Box<dyn ViewportHost>) {
        let id = vp.id().to_string();
        if !self.viewports.contains_key(&id) {
            self.order.push(id.clone());
        }
        if self.active_id.is_none() {
            self.active_id = Some(id.clone());
        }
        self.viewports.insert(id, vp);
    }

    /// Remove a viewport by id.  Returns `true` if it was present.
    pub fn unregister(&mut self, id: &str) -> bool {
        if self.viewports.remove(id).is_none() { return false; }
        self.order.retain(|i| i != id);
        if self.active_id.as_deref() == Some(id) {
            self.active_id = self.order.first().cloned();
        }
        true
    }

    /// Get an immutable reference to a viewport.
    pub fn get(&self, id: &str) -> Option<&dyn ViewportHost> {
        self.viewports.get(id).map(|v| v.as_ref())
    }

    /// Apply a mutation to a viewport by id via a callback.
    /// Returns `Some(R)` if the viewport exists, `None` otherwise.
    pub fn apply_mut<F, R>(&mut self, id: &str, f: F) -> Option<R>
    where
        F: FnOnce(&mut dyn ViewportHost) -> R,
    {
        self.viewports.get_mut(id).map(|v| f(&mut **v))
    }

    /// Apply a mutation to the currently active viewport.
    pub fn apply_active_mut<F, R>(&mut self, f: F) -> Option<R>
    where
        F: FnOnce(&mut dyn ViewportHost) -> R,
    {
        let id = self.active_id.clone()?;
        self.apply_mut(&id, f)
    }

    /// Set the active (focused) viewport.  Does nothing if the id is unknown.
    pub fn set_active(&mut self, id: &str) {
        if self.viewports.contains_key(id) {
            self.active_id = Some(id.to_string());
        }
    }

    /// Currently active viewport id.
    pub fn active_id(&self) -> Option<&str> {
        self.active_id.as_deref()
    }

    /// Active viewport (immutable).
    pub fn active(&self) -> Option<&dyn ViewportHost> {
        self.active_id.as_deref().and_then(|id| self.get(id))
    }

    /// Number of registered viewports.
    pub fn count(&self) -> usize { self.viewports.len() }

    /// Iterate viewport ids in registration order.
    pub fn ids(&self) -> impl Iterator<Item = &str> {
        self.order.iter().map(|s| s.as_str())
    }

    /// Resize a specific viewport.
    pub fn resize(&mut self, id: &str, size: ViewportSize) {
        if let Some(vp) = self.viewports.get_mut(id) {
            vp.resize(size);
        }
    }
}

// ── Default stub implementation ───────────────────────────────────────────────

/// A minimal in-process viewport that stores state without actually rendering.
/// Used for tests and headless contexts.
pub struct StubViewport {
    id:          String,
    title:       String,
    kind:        ViewportKind,
    size:        ViewportSize,
    render_mode: ViewportRenderMode,
    visible:     bool,
}

impl StubViewport {
    pub fn new(id: &str, title: &str, kind: ViewportKind) -> Self {
        Self {
            id:          id.to_string(),
            title:       title.to_string(),
            kind,
            size:        ViewportSize::new(800, 600),
            render_mode: ViewportRenderMode::Lit,
            visible:     true,
        }
    }
}

impl ViewportHost for StubViewport {
    fn id(&self)            -> &str           { &self.id }
    fn title(&self)         -> &str           { &self.title }
    fn kind(&self)          -> ViewportKind   { self.kind }
    fn resize(&mut self, s: ViewportSize)     { self.size = s; }
    fn size(&self)          -> ViewportSize   { self.size }
    fn render_mode(&self)   -> ViewportRenderMode { self.render_mode }
    fn set_render_mode(&mut self, m: ViewportRenderMode) { self.render_mode = m; }
    fn is_visible(&self)    -> bool           { self.visible }
    fn set_visible(&mut self, v: bool)        { self.visible = v; }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn scene_vp() -> Box<StubViewport> {
        Box::new(StubViewport::new("scene", "Scene", ViewportKind::Scene))
    }

    fn material_vp() -> Box<StubViewport> {
        Box::new(StubViewport::new("material", "Material Preview", ViewportKind::Material))
    }

    // ── ViewportSize ─────────────────────────────────────────────────────────

    #[test]
    fn aspect_ratio() {
        let s = ViewportSize::new(1920, 1080);
        let ratio = s.aspect_ratio();
        assert!((ratio - (1920.0 / 1080.0)).abs() < 1e-4);
    }

    #[test]
    fn aspect_ratio_zero_height() {
        let s = ViewportSize::new(100, 0);
        assert_eq!(s.aspect_ratio(), 1.0);
    }

    // ── StubViewport ──────────────────────────────────────────────────────────

    #[test]
    fn stub_default_state() {
        let vp = StubViewport::new("scene", "Scene", ViewportKind::Scene);
        assert_eq!(vp.id(), "scene");
        assert_eq!(vp.kind(), ViewportKind::Scene);
        assert_eq!(vp.size(), ViewportSize::new(800, 600));
        assert_eq!(vp.render_mode(), ViewportRenderMode::Lit);
        assert!(vp.is_visible());
    }

    #[test]
    fn stub_resize() {
        let mut vp = StubViewport::new("s", "S", ViewportKind::Scene);
        vp.resize(ViewportSize::new(1280, 720));
        assert_eq!(vp.size(), ViewportSize::new(1280, 720));
    }

    #[test]
    fn stub_set_render_mode() {
        let mut vp = StubViewport::new("s", "S", ViewportKind::Scene);
        vp.set_render_mode(ViewportRenderMode::Wireframe);
        assert_eq!(vp.render_mode(), ViewportRenderMode::Wireframe);
    }

    #[test]
    fn stub_visibility() {
        let mut vp = StubViewport::new("s", "S", ViewportKind::Scene);
        vp.set_visible(false);
        assert!(!vp.is_visible());
    }

    // ── ViewportRegistry ──────────────────────────────────────────────────────

    #[test]
    fn registry_starts_empty() {
        let reg = ViewportRegistry::new();
        assert_eq!(reg.count(), 0);
        assert!(reg.active_id().is_none());
    }

    #[test]
    fn register_makes_first_active() {
        let mut reg = ViewportRegistry::new();
        reg.register(scene_vp());
        assert_eq!(reg.active_id(), Some("scene"));
    }

    #[test]
    fn register_second_does_not_change_active() {
        let mut reg = ViewportRegistry::new();
        reg.register(scene_vp());
        reg.register(material_vp());
        assert_eq!(reg.active_id(), Some("scene"));
        assert_eq!(reg.count(), 2);
    }

    #[test]
    fn get_and_apply_mut() {
        let mut reg = ViewportRegistry::new();
        reg.register(scene_vp());
        assert!(reg.get("scene").is_some());
        assert!(reg.get("nope").is_none());
        reg.apply_mut("scene", |vp| vp.resize(ViewportSize::new(1024, 768)));
        assert_eq!(reg.get("scene").unwrap().size(), ViewportSize::new(1024, 768));
    }

    #[test]
    fn set_active() {
        let mut reg = ViewportRegistry::new();
        reg.register(scene_vp());
        reg.register(material_vp());
        reg.set_active("material");
        assert_eq!(reg.active_id(), Some("material"));
    }

    #[test]
    fn set_active_unknown_noop() {
        let mut reg = ViewportRegistry::new();
        reg.register(scene_vp());
        reg.set_active("ghost");
        assert_eq!(reg.active_id(), Some("scene"));
    }

    #[test]
    fn unregister_existing() {
        let mut reg = ViewportRegistry::new();
        reg.register(scene_vp());
        reg.register(material_vp());
        assert!(reg.unregister("scene"));
        assert_eq!(reg.count(), 1);
        assert!(reg.get("scene").is_none());
    }

    #[test]
    fn unregister_active_switches_to_next() {
        let mut reg = ViewportRegistry::new();
        reg.register(scene_vp());
        reg.register(material_vp());
        reg.unregister("scene");
        assert_eq!(reg.active_id(), Some("material"));
    }

    #[test]
    fn unregister_nonexistent_returns_false() {
        let mut reg = ViewportRegistry::new();
        assert!(!reg.unregister("ghost"));
    }

    #[test]
    fn registry_resize() {
        let mut reg = ViewportRegistry::new();
        reg.register(scene_vp());
        reg.resize("scene", ViewportSize::new(640, 480));
        assert_eq!(reg.get("scene").unwrap().size(), ViewportSize::new(640, 480));
    }

    #[test]
    fn ids_in_registration_order() {
        let mut reg = ViewportRegistry::new();
        reg.register(scene_vp());
        reg.register(material_vp());
        let ids: Vec<&str> = reg.ids().collect();
        assert_eq!(ids, vec!["scene", "material"]);
    }
}
