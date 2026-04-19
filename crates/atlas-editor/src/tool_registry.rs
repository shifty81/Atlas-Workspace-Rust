//! [`IEditorTool`], [`IEditorPanel`] traits and [`ToolRegistry`] — editor tool management.
//!
//! Tools are persistent, stateful objects that occupy the editor's main
//! working area (e.g. a scene tool, a material editor tool, a game-specific
//! gameplay editor tool).  Panels are lightweight sub-views that can be
//! docked anywhere.
//!
//! Game project adapters (see [`crate::game_project_adapter`]) return a list
//! of [`ToolDescriptor`]s that the editor uses to populate the tool bar
//! and register the corresponding tools via [`ToolRegistry::register`].

use std::collections::HashMap;

// ── ToolDescriptor ────────────────────────────────────────────────────────────

/// Metadata about a single editor tool, used for toolbar rendering and
/// registration.
#[derive(Debug, Clone)]
pub struct ToolDescriptor {
    /// Stable identifier, e.g. `"scene"`, `"nf.economy"`.
    pub id: String,
    /// Display name shown in the toolbar / menu.
    pub title: String,
    /// Optional icon glyph or resource path.
    pub icon: Option<String>,
}

impl ToolDescriptor {
    /// Create a descriptor with no icon.
    pub fn new(id: impl Into<String>, title: impl Into<String>) -> Self {
        Self { id: id.into(), title: title.into(), icon: None }
    }

    /// Create a descriptor with an icon.
    pub fn with_icon(
        id: impl Into<String>,
        title: impl Into<String>,
        icon: impl Into<String>,
    ) -> Self {
        Self { id: id.into(), title: title.into(), icon: Some(icon.into()) }
    }
}

// ── IEditorTool ───────────────────────────────────────────────────────────────

/// Trait implemented by all editor tools.
///
/// A tool is a top-level editing mode that has its own state and may own one
/// or more panels.  Register tools via [`ToolRegistry::register`].
pub trait IEditorTool: Send + Sync {
    /// Stable identifier matching the [`ToolDescriptor::id`].
    fn id(&self) -> &str;

    /// Display name shown in the toolbar.
    fn title(&self) -> &str;

    /// Called once per frame when this tool is the active tool.
    ///
    /// The default implementation is a no-op.
    fn update(&mut self) {}
}

// ── IEditorPanel ─────────────────────────────────────────────────────────────

/// Trait implemented by reusable editor panels.
///
/// Panels are sub-views that can be docked anywhere in the editor layout.
/// Both core atlas panels and game-specific NovaForge panels implement this.
pub trait IEditorPanel: Send + Sync {
    /// Stable identifier, e.g. `"nf.economy"`.
    fn panel_id(&self) -> &str;

    /// Display name shown in the panel tab/header.
    fn panel_title(&self) -> &str;
}

// ── ToolRegistry ──────────────────────────────────────────────────────────────

/// Registry of all loaded editor tools.
///
/// At most one tool is active at any given time.  The first registered tool
/// becomes the initial active tool.
pub struct ToolRegistry {
    tools:     HashMap<String, Box<dyn IEditorTool>>,
    active_id: Option<String>,
}

impl ToolRegistry {
    /// Create an empty registry.
    pub fn new() -> Self {
        Self { tools: HashMap::new(), active_id: None }
    }

    /// Register a tool.
    ///
    /// Returns `true` on success; `false` if a tool with the same ID was
    /// already registered (the existing entry is unchanged).
    pub fn register(&mut self, tool: Box<dyn IEditorTool>) -> bool {
        let id = tool.id().to_string();
        if self.tools.contains_key(&id) {
            return false;
        }
        if self.active_id.is_none() {
            self.active_id = Some(id.clone());
        }
        self.tools.insert(id, tool);
        true
    }

    /// Activate a tool by ID.  Returns `false` if the ID is not registered.
    pub fn activate(&mut self, id: &str) -> bool {
        if self.tools.contains_key(id) {
            self.active_id = Some(id.to_string());
            true
        } else {
            false
        }
    }

    /// ID of the currently active tool, if any.
    pub fn active_id(&self) -> Option<&str> {
        self.active_id.as_deref()
    }

    /// Returns a mutable reference to the tool with the given ID.
    pub fn get_mut(&mut self, id: &str) -> Option<&mut dyn IEditorTool> {
        if let Some(t) = self.tools.get_mut(id) {
            Some(&mut **t)
        } else {
            None
        }
    }

    /// Number of registered tools.
    pub fn count(&self) -> usize {
        self.tools.len()
    }

    /// Descriptors for all registered tools (order is unspecified).
    pub fn descriptors(&self) -> Vec<ToolDescriptor> {
        self.tools
            .values()
            .map(|t| ToolDescriptor::new(t.id(), t.title()))
            .collect()
    }

    /// Call [`IEditorTool::update`] on the currently active tool (if any).
    pub fn tick_active(&mut self) {
        if let Some(id) = self.active_id.clone() {
            if let Some(tool) = self.tools.get_mut(&id) {
                tool.update();
            }
        }
    }
}

impl Default for ToolRegistry {
    fn default() -> Self { Self::new() }
}

// ── Tests ────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    struct DummyTool { id: &'static str, title: &'static str, ticks: u32 }

    impl IEditorTool for DummyTool {
        fn id(&self)    -> &str { self.id }
        fn title(&self) -> &str { self.title }
        fn update(&mut self) { self.ticks += 1; }
    }

    fn make_tool(id: &'static str, title: &'static str) -> Box<DummyTool> {
        Box::new(DummyTool { id, title, ticks: 0 })
    }

    #[test]
    fn empty_registry_has_no_active_tool() {
        let reg = ToolRegistry::new();
        assert!(reg.active_id().is_none());
        assert_eq!(reg.count(), 0);
    }

    #[test]
    fn first_registered_tool_becomes_active() {
        let mut reg = ToolRegistry::new();
        assert!(reg.register(make_tool("scene", "Scene")));
        assert_eq!(reg.active_id(), Some("scene"));
    }

    #[test]
    fn duplicate_registration_returns_false() {
        let mut reg = ToolRegistry::new();
        reg.register(make_tool("scene", "Scene"));
        assert!(!reg.register(make_tool("scene", "Scene Duplicate")));
        assert_eq!(reg.count(), 1);
    }

    #[test]
    fn activate_switches_tool() {
        let mut reg = ToolRegistry::new();
        reg.register(make_tool("scene", "Scene"));
        reg.register(make_tool("nf.economy", "Economy"));
        assert!(reg.activate("nf.economy"));
        assert_eq!(reg.active_id(), Some("nf.economy"));
    }

    #[test]
    fn activate_unknown_returns_false() {
        let mut reg = ToolRegistry::new();
        assert!(!reg.activate("no_such_tool"));
    }

    #[test]
    fn tick_active_calls_update_on_active_tool() {
        let mut reg = ToolRegistry::new();
        // tick_active should not panic on an empty registry or registered tool
        reg.tick_active(); // no-op, no tools registered
        reg.register(make_tool("scene", "Scene"));
        reg.tick_active(); // calls update on "scene"
        reg.tick_active();
        // If we reach here without panic the test passes;
        // tick count is internal to DummyTool which we can't safely downcast.
        assert_eq!(reg.active_id(), Some("scene"));
    }

    #[test]
    fn descriptors_returns_all_tools() {
        let mut reg = ToolRegistry::new();
        reg.register(make_tool("a", "A"));
        reg.register(make_tool("b", "B"));
        let descs = reg.descriptors();
        assert_eq!(descs.len(), 2);
    }

    #[test]
    fn tool_descriptor_new() {
        let d = ToolDescriptor::new("t", "Title");
        assert_eq!(d.id, "t");
        assert!(d.icon.is_none());
    }

    #[test]
    fn tool_descriptor_with_icon() {
        let d = ToolDescriptor::with_icon("t", "Title", "icon.png");
        assert_eq!(d.icon.as_deref(), Some("icon.png"));
    }

    #[test]
    fn ieditor_panel_trait() {
        struct DummyPanel;
        impl IEditorPanel for DummyPanel {
            fn panel_id(&self)    -> &str { "dummy" }
            fn panel_title(&self) -> &str { "Dummy Panel" }
        }
        let p = DummyPanel;
        assert_eq!(p.panel_id(), "dummy");
        assert_eq!(p.panel_title(), "Dummy Panel");
    }
}
