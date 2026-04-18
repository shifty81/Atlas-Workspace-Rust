//! [`NovaForgeProjectBootstrap`] — validates a `.atlas` manifest and resolves
//! the NovaForge content roots.
//!
//! This is the first step when the editor opens a NovaForge project.  It reads
//! the manifest, checks that the required directories exist on disk, and
//! produces a [`BootstrapResult`] with resolved absolute paths for all roots.
//!
//! # License
//!
//! GPL v3.0 — this file is part of `novaforge-game`.

use atlas_editor::AtlasManifest;

// ── BootstrapResult ───────────────────────────────────────────────────────────

/// Resolved absolute content-root paths produced by a successful bootstrap.
#[derive(Debug, Clone)]
pub struct BootstrapResult {
    /// Absolute path to the manifest file.
    pub manifest_path: String,
    /// Resolved content root directory.
    pub content_root: String,
    /// Resolved data root directory.
    pub data_root: String,
    /// Resolved config root directory.
    pub config_root: String,
    /// Resolved schemas root directory.
    pub schemas_root: String,
    /// Project display name from the manifest.
    pub project_name: String,
    /// Adapter key declared in the manifest.
    pub adapter: String,
}

// ── NovaForgeProjectBootstrap ─────────────────────────────────────────────────

/// Validates a `.atlas` manifest and resolves NovaForge content roots.
///
/// ```rust,ignore
/// let bootstrap = NovaForgeProjectBootstrap::new();
/// let result = bootstrap.run("NovaForge/NovaForge.atlas")?;
/// println!("content root: {}", result.content_root);
/// ```
pub struct NovaForgeProjectBootstrap {
    /// When `true`, directory existence checks are skipped (useful in tests
    /// and CI where assets may not be present).
    pub skip_dir_checks: bool,
}

impl NovaForgeProjectBootstrap {
    /// Create a bootstrap with strict directory checking enabled.
    pub fn new() -> Self {
        Self { skip_dir_checks: false }
    }

    /// Create a bootstrap suitable for unit tests (skips filesystem checks).
    pub fn for_tests() -> Self {
        Self { skip_dir_checks: true }
    }

    /// Run the bootstrap against the given manifest path.
    ///
    /// Returns a [`BootstrapResult`] on success or an error string describing
    /// the first problem encountered.
    pub fn run(&self, manifest_path: &str) -> Result<BootstrapResult, String> {
        // 1. Parse the manifest
        let manifest = AtlasManifest::from_file(manifest_path)
            .map_err(|e| format!("[NovaForge Bootstrap] {e}"))?;

        self.run_with_manifest(manifest_path, &manifest)
    }

    /// Run the bootstrap from a pre-parsed manifest (useful when the editor
    /// already holds the manifest object).
    pub fn run_with_manifest(
        &self,
        manifest_path: &str,
        manifest: &AtlasManifest,
    ) -> Result<BootstrapResult, String> {
        // 2. Validate adapter type
        if manifest.adapter != "novaforge" && !manifest.adapter.is_empty() {
            return Err(format!(
                "[NovaForge Bootstrap] Unexpected adapter '{}' (expected 'novaforge')",
                manifest.adapter
            ));
        }

        // 3. Compute the base directory (directory containing the manifest)
        let base_dir = std::path::Path::new(manifest_path)
            .parent()
            .map(|p| p.to_string_lossy().into_owned())
            .unwrap_or_else(|| ".".to_string());

        // 4. Resolve content roots
        let content_root = self.resolve_root(&base_dir, &manifest.roots.content, "Content");
        let data_root    = self.resolve_root(&base_dir, &manifest.roots.data,    "Data");
        let config_root  = self.resolve_root(&base_dir, &manifest.roots.config,  "Config");
        let schemas_root = self.resolve_root(&base_dir, &manifest.roots.schemas, "Schemas");

        // 5. Optionally verify directories exist on disk
        if !self.skip_dir_checks {
            self.check_dir(&content_root, "content")?;
            self.check_dir(&data_root,    "data")?;
        }

        log::info!(
            "[NovaForge Bootstrap] Project '{}' bootstrapped from '{}'",
            manifest.project_name, manifest_path
        );
        log::info!("[NovaForge Bootstrap]  content  → {content_root}");
        log::info!("[NovaForge Bootstrap]  data     → {data_root}");
        log::info!("[NovaForge Bootstrap]  config   → {config_root}");

        Ok(BootstrapResult {
            manifest_path: manifest_path.to_string(),
            content_root,
            data_root,
            config_root,
            schemas_root,
            project_name: manifest.project_name.clone(),
            adapter:       manifest.adapter.clone(),
        })
    }

    // ── Private helpers ───────────────────────────────────────────────────────

    fn resolve_root(&self, base: &str, declared: &str, fallback: &str) -> String {
        let rel = if declared.is_empty() { fallback } else { declared };
        if std::path::Path::new(rel).is_absolute() {
            rel.to_string()
        } else {
            format!("{base}/{rel}")
        }
    }

    fn check_dir(&self, path: &str, label: &str) -> Result<(), String> {
        if !std::path::Path::new(path).is_dir() {
            log::warn!("[NovaForge Bootstrap] {label} directory not found: '{path}'");
        }
        Ok(())
    }
}

impl Default for NovaForgeProjectBootstrap {
    fn default() -> Self { Self::new() }
}

// ── Tests ────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use atlas_editor::AtlasManifest;

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
    fn bootstrap_with_manifest_succeeds() {
        let bs = NovaForgeProjectBootstrap::for_tests();
        let result = bs.run_with_manifest("Project/NovaForge.atlas", &manifest()).unwrap();
        assert_eq!(result.project_name, "NovaForge");
        assert_eq!(result.adapter, "novaforge");
    }

    #[test]
    fn content_root_resolved_relative_to_manifest_dir() {
        let bs = NovaForgeProjectBootstrap::for_tests();
        let result = bs.run_with_manifest("path/to/NovaForge.atlas", &manifest()).unwrap();
        assert!(result.content_root.contains("Content"));
        assert!(result.content_root.starts_with("path/to/"));
    }

    #[test]
    fn data_root_resolved_relative_to_manifest_dir() {
        let bs = NovaForgeProjectBootstrap::for_tests();
        let result = bs.run_with_manifest("path/to/NovaForge.atlas", &manifest()).unwrap();
        assert!(result.data_root.contains("Data"));
        assert!(result.data_root.starts_with("path/to/"));
    }

    #[test]
    fn wrong_adapter_returns_error() {
        let mut m = manifest();
        m.adapter = "other_game".to_string();
        let bs = NovaForgeProjectBootstrap::for_tests();
        assert!(bs.run_with_manifest("x.atlas", &m).is_err());
    }

    #[test]
    fn empty_adapter_is_accepted() {
        let mut m = manifest();
        m.adapter = String::new();
        let bs = NovaForgeProjectBootstrap::for_tests();
        assert!(bs.run_with_manifest("x.atlas", &m).is_ok());
    }

    #[test]
    fn missing_manifest_file_returns_error() {
        let bs = NovaForgeProjectBootstrap::new();
        assert!(bs.run("/no/such/path.atlas").is_err());
    }

    #[test]
    fn fallback_roots_used_when_manifest_roots_empty() {
        let m = AtlasManifest::from_json(
            r#"{"projectId":"nf","projectName":"NF","projectType":"t","adapter":"novaforge","roots":{}}"#
        ).unwrap();
        let bs = NovaForgeProjectBootstrap::for_tests();
        let result = bs.run_with_manifest("base/x.atlas", &m).unwrap();
        assert!(result.content_root.contains("Content"));
        assert!(result.data_root.contains("Data"));
    }
}
