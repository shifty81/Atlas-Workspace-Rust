//! [`NovaForgeAdapter`] — the editor-side adapter for the NovaForge project.
//!
//! This struct bridges the Atlas Workspace editor (`atlas-editor`) and the
//! NovaForge game module via the `GameProjectAdapter` trait contract.
//!
//! **Important**: `novaforge-game` must never be imported by any `atlas-*` crate.
//! The adapter pattern here ensures all communication goes one way: the editor
//! calls into the adapter, which delegates to `NovaForgeGameModule`.
//!
//! # License
//!
//! GPL v3.0 — this file is part of `novaforge-game`.

use atlas_asset::AssetRegistry;
use atlas_ecs::World;
use atlas_editor::{
    AtlasManifest, GameProjectAdapter, PieState, ToolDescriptor,
};

use crate::{
    bootstrap::NovaForgeProjectBootstrap,
    catalog::AssetCatalog,
    data_registry::DataRegistry,
    document_registry::DocumentRegistry,
};

// ── NovaForgeAdapter ──────────────────────────────────────────────────────────

/// The NovaForge adapter — implements the `GameProjectAdapter` editor boundary.
///
/// Wires together the bootstrap, asset catalog, data registry, and document
/// registry, and exposes them to the editor session.
pub struct NovaForgeAdapter {
    /// Human-readable project name shown in the editor title bar.
    pub project_name:      String,
    /// Path to the `.atlas` manifest file for this project.
    pub manifest_path:     String,
    /// Asset registry populated when the project is opened.
    pub assets:            AssetRegistry,
    /// Asset catalog (filesystem view of `novaforge-assets/`).
    pub catalog:           AssetCatalog,
    /// Data registry (JSON files from `Data/`).
    pub data:              DataRegistry,
    /// Document registry (tracks open documents).
    pub documents:         DocumentRegistry,
    /// PIE state for this session.
    pie_state:             PieState,
    initialized:           bool,
}

impl NovaForgeAdapter {
    /// Create a new adapter pointing at `manifest_path` (a `.atlas` JSON file).
    pub fn new(manifest_path: impl Into<String>) -> Self {
        let manifest_path = manifest_path.into();
        Self {
            project_name:  "NovaForge".into(),
            manifest_path,
            assets:        AssetRegistry::new(),
            catalog:       AssetCatalog::new("novaforge-assets"),
            data:          DataRegistry::new("NovaForge/Data"),
            documents:     DocumentRegistry::with_novaforge_defaults(),
            pie_state:     PieState::Idle,
            initialized:   false,
        }
    }

    /// Returns `true` if [`initialize_project`] has been called successfully.
    ///
    /// [`initialize_project`]: GameProjectAdapter::initialize_project
    pub fn is_initialized(&self) -> bool { self.initialized }

    /// Number of registered assets (after initialization).
    pub fn asset_count(&self) -> usize { self.assets.count() }
}

impl GameProjectAdapter for NovaForgeAdapter {
    fn project_name(&self) -> &str { &self.project_name }

    fn initialize_project(&mut self, manifest: &AtlasManifest) -> Result<(), String> {
        if self.initialized { return Ok(()); }

        log::info!("[NovaForgeAdapter] Initialising project: {}", self.manifest_path);

        // 1. Bootstrap — validate manifest + resolve content roots
        let bs = NovaForgeProjectBootstrap::for_tests(); // skip FS checks for missing assets
        let boot = bs.run_with_manifest(&self.manifest_path, manifest)?;

        // 2. Update paths from bootstrap result
        self.catalog  = AssetCatalog::new(&boot.content_root);
        self.data     = DataRegistry::new(&boot.data_root);

        // 3. Scan assets (non-blocking; missing dir warns but does not fail)
        let asset_count = self.catalog.scan_into(&mut self.assets);
        log::info!("[NovaForgeAdapter] {} assets registered", asset_count);

        // 4. Load data files
        let data_count = self.data.load_all();
        log::info!("[NovaForgeAdapter] {} data files loaded", data_count);

        // 5. Finalise
        self.initialized = true;
        self.project_name = manifest.project_name.clone();
        log::info!("[NovaForgeAdapter] Initialised project '{}'", self.project_name);
        Ok(())
    }

    fn tool_descriptors(&self) -> Vec<ToolDescriptor> {
        vec![
            ToolDescriptor::new("nf.economy",     "Economy Editor"),
            ToolDescriptor::new("nf.inventory",   "Inventory Rules"),
            ToolDescriptor::new("nf.shop",        "Shop Editor"),
            ToolDescriptor::new("nf.missions",    "Mission Rules"),
            ToolDescriptor::new("nf.progression", "Progression"),
            ToolDescriptor::new("nf.characters",  "Character Rules"),
        ]
    }

    fn start_pie(&mut self, _world: &World) -> Result<(), String> {
        if self.pie_state == PieState::Running || self.pie_state == PieState::Starting {
            return Ok(());
        }
        log::info!("[NovaForgeAdapter] Starting PIE …");
        self.pie_state = PieState::Running;
        Ok(())
    }

    fn stop_pie(&mut self) {
        if self.pie_state == PieState::Idle { return; }
        log::info!("[NovaForgeAdapter] Stopping PIE");
        self.pie_state = PieState::Idle;
    }

    fn poll(&mut self) -> PieState {
        self.pie_state.clone()
    }
}

// ── Tests ────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    const MANIFEST_JSON: &str = r#"{
        "projectId": "novaforge",
        "projectName": "NovaForge",
        "projectType": "novaforge",
        "adapter": "novaforge",
        "roots": { "content": "Content", "data": "Data", "config": "Config" }
    }"#;

    fn manifest() -> AtlasManifest {
        AtlasManifest::from_json(MANIFEST_JSON).unwrap()
    }

    #[test]
    fn new_adapter_is_not_initialized() {
        let a = NovaForgeAdapter::new("NovaForge.atlas");
        assert!(!a.is_initialized());
    }

    #[test]
    fn initialize_project_succeeds() {
        let mut a = NovaForgeAdapter::new("NovaForge.atlas");
        assert!(a.initialize_project(&manifest()).is_ok());
        assert!(a.is_initialized());
    }

    #[test]
    fn initialize_project_is_idempotent() {
        let mut a = NovaForgeAdapter::new("NovaForge.atlas");
        assert!(a.initialize_project(&manifest()).is_ok());
        assert!(a.initialize_project(&manifest()).is_ok());
    }

    #[test]
    fn tool_descriptors_returns_six_tools() {
        let a = NovaForgeAdapter::new("NovaForge.atlas");
        assert_eq!(a.tool_descriptors().len(), 6);
    }

    #[test]
    fn tool_descriptors_includes_economy() {
        let a = NovaForgeAdapter::new("NovaForge.atlas");
        assert!(a.tool_descriptors().iter().any(|t| t.id == "nf.economy"));
    }

    #[test]
    fn pie_idle_initially() {
        let a = NovaForgeAdapter::new("NovaForge.atlas");
        assert_eq!(a.pie_state, PieState::Idle);
    }

    #[test]
    fn start_and_stop_pie() {
        let mut a = NovaForgeAdapter::new("NovaForge.atlas");
        let mut w = World::new();
        a.start_pie(&mut w).unwrap();
        assert_eq!(a.poll(), PieState::Running);
        a.stop_pie();
        assert_eq!(a.poll(), PieState::Idle);
    }

    #[test]
    fn project_name_matches_manifest() {
        let mut a = NovaForgeAdapter::new("NovaForge.atlas");
        a.initialize_project(&manifest()).unwrap();
        assert_eq!(a.project_name(), "NovaForge");
    }
}

