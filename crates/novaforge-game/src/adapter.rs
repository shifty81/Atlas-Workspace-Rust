//! [`NovaForgeAdapter`] — the editor-side adapter for the NovaForge project.
//!
//! This struct bridges the Atlas Workspace editor (`atlas-editor`) and the
//! NovaForge game module via the `IGameProjectAdapter` trait contract.
//!
//! **Important**: `novaforge-game` must never be imported by any `atlas-*` crate.
//! The adapter pattern here ensures all communication goes one way: the editor
//! calls into the adapter, which delegates to `NovaForgeGameModule`.
//!
//! # License
//!
//! GPL v3.0 — this file is part of `novaforge-game`.

use atlas_asset::AssetRegistry;

/// Descriptor for a single editor tool provided by NovaForge.
#[derive(Debug, Clone)]
pub struct NovaForgeToolDescriptor {
    pub id:    String,
    pub title: String,
    pub icon:  Option<String>,
}

/// The NovaForge adapter — implements the editor ↔ game boundary.
///
/// In a future milestone this will implement `atlas_editor::IGameProjectAdapter`
/// once that trait is stabilised in `atlas-editor`.  For now it provides the
/// scaffolding and can be constructed independently.
pub struct NovaForgeAdapter {
    /// Human-readable project name shown in the editor title bar.
    pub project_name: String,
    /// Path to the `.atlas` manifest file for this project.
    pub manifest_path: String,
    /// Asset registry populated when the project is opened.
    pub assets: AssetRegistry,
    /// Tool descriptors provided to the editor's tool registry.
    pub tools: Vec<NovaForgeToolDescriptor>,
    initialized: bool,
}

impl NovaForgeAdapter {
    /// Create a new adapter pointing at `manifest_path` (a `.atlas` JSON file).
    pub fn new(manifest_path: impl Into<String>) -> Self {
        Self {
            project_name:  "NovaForge".into(),
            manifest_path: manifest_path.into(),
            assets:        AssetRegistry::new(),
            tools:         Self::default_tools(),
            initialized:   false,
        }
    }

    /// Initialise the adapter: parse the `.atlas` manifest, scan the asset
    /// catalog, and register default tools with the editor.
    ///
    /// Returns `Ok(())` on success, or an error string if initialisation fails.
    pub fn initialize(&mut self) -> Result<(), String> {
        if self.initialized {
            return Ok(());
        }

        log::info!("[NovaForgeAdapter] Initialising project: {}", self.manifest_path);

        // Validate that the manifest path looks sensible
        if self.manifest_path.is_empty() {
            return Err("manifest_path is empty".into());
        }

        // TODO (Phase 2.2): parse the .atlas JSON manifest
        // TODO (Phase 2.2): scan novaforge-assets/ via AssetCatalog
        // TODO (Phase 2.2): populate self.assets from the scan

        self.initialized = true;
        log::info!("[NovaForgeAdapter] Initialised — {} tools registered", self.tools.len());
        Ok(())
    }

    /// Shut down the adapter and flush any pending state.
    pub fn shutdown(&mut self) {
        log::info!("[NovaForgeAdapter] Shutting down");
        self.initialized = false;
        // TODO (Phase 2.2): flush open documents, save layout, etc.
    }

    /// Returns the list of tool descriptors to register with the editor.
    pub fn tool_descriptors(&self) -> &[NovaForgeToolDescriptor] {
        &self.tools
    }

    /// Returns `true` if the adapter has been successfully initialised.
    pub fn is_initialized(&self) -> bool { self.initialized }

    /// Returns the number of registered assets.
    pub fn asset_count(&self) -> usize { self.assets.count() }

    // ── Private helpers ──────────────────────────────────────────────────────

    fn default_tools() -> Vec<NovaForgeToolDescriptor> {
        vec![
            NovaForgeToolDescriptor { id: "nf.economy".into(),     title: "Economy Editor".into(),     icon: None },
            NovaForgeToolDescriptor { id: "nf.inventory".into(),   title: "Inventory Rules".into(),    icon: None },
            NovaForgeToolDescriptor { id: "nf.shop".into(),        title: "Shop Editor".into(),        icon: None },
            NovaForgeToolDescriptor { id: "nf.missions".into(),    title: "Mission Editor".into(),     icon: None },
            NovaForgeToolDescriptor { id: "nf.progression".into(), title: "Progression Editor".into(), icon: None },
            NovaForgeToolDescriptor { id: "nf.characters".into(),  title: "Character Rules".into(),    icon: None },
        ]
    }
}

// ── Tests ────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn adapter_new_is_not_initialized() {
        let adapter = NovaForgeAdapter::new("NovaForge.atlas");
        assert!(!adapter.is_initialized());
    }

    #[test]
    fn adapter_initialize_succeeds() {
        let mut adapter = NovaForgeAdapter::new("NovaForge.atlas");
        assert!(adapter.initialize().is_ok());
        assert!(adapter.is_initialized());
    }

    #[test]
    fn adapter_initialize_empty_path_fails() {
        let mut adapter = NovaForgeAdapter::new("");
        assert!(adapter.initialize().is_err());
    }

    #[test]
    fn adapter_double_initialize_is_idempotent() {
        let mut adapter = NovaForgeAdapter::new("NovaForge.atlas");
        assert!(adapter.initialize().is_ok());
        assert!(adapter.initialize().is_ok()); // second call is a no-op
    }

    #[test]
    fn adapter_has_default_tools() {
        let adapter = NovaForgeAdapter::new("NovaForge.atlas");
        assert!(!adapter.tool_descriptors().is_empty());
        assert!(adapter.tool_descriptors().iter().any(|t| t.id == "nf.economy"));
    }

    #[test]
    fn adapter_shutdown_resets_state() {
        let mut adapter = NovaForgeAdapter::new("NovaForge.atlas");
        adapter.initialize().unwrap();
        adapter.shutdown();
        assert!(!adapter.is_initialized());
    }

    #[test]
    fn adapter_project_name() {
        let adapter = NovaForgeAdapter::new("NovaForge.atlas");
        assert_eq!(adapter.project_name, "NovaForge");
    }
}
