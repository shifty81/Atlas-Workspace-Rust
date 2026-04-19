//! [`ProjectRegistry`] — loads, tracks, and activates `.atlas` project manifests.
//!
//! The registry is the editor's authoritative store of all known projects.
//! A project is loaded from a `.atlas` manifest file (JSON) and identified by
//! the `projectId` field inside that manifest.

use std::collections::HashMap;

use crate::manifest::AtlasManifest;

// ── LoadedProject ────────────────────────────────────────────────────────────

/// An entry in the [`ProjectRegistry`] representing a successfully parsed project.
#[derive(Debug, Clone)]
pub struct LoadedProject {
    /// Path to the `.atlas` manifest file (as given to [`ProjectRegistry::load`]).
    pub manifest_path: String,
    /// The parsed manifest contents.
    pub manifest: AtlasManifest,
}

impl LoadedProject {
    /// Returns the project ID (convenience shorthand).
    pub fn id(&self) -> &str {
        &self.manifest.project_id
    }

    /// Returns the human-readable project name.
    pub fn name(&self) -> &str {
        &self.manifest.project_name
    }
}

// ── ProjectRegistry ───────────────────────────────────────────────────────────

/// Manages all loaded projects and tracks the currently active one.
///
/// ## Typical usage
///
/// ```rust,ignore
/// let mut reg = ProjectRegistry::new();
/// let project = reg.load("NovaForge/NovaForge.atlas")?;
/// println!("Opened: {}", project.name());
/// ```
pub struct ProjectRegistry {
    projects:   HashMap<String, LoadedProject>,
    current_id: Option<String>,
}

impl ProjectRegistry {
    /// Create an empty registry.
    pub fn new() -> Self {
        Self { projects: HashMap::new(), current_id: None }
    }

    /// Load a project from a `.atlas` manifest file on disk.
    ///
    /// On success the project becomes the current (active) project and a
    /// reference to it is returned.
    pub fn load(&mut self, manifest_path: &str) -> Result<&LoadedProject, String> {
        let manifest = AtlasManifest::from_file(manifest_path)?;
        let id = manifest.project_id.clone();
        self.projects.insert(id.clone(), LoadedProject {
            manifest_path: manifest_path.to_string(),
            manifest,
        });
        self.current_id = Some(id.clone());
        // SAFETY: we just inserted `id`
        Ok(self.projects.get(&id).unwrap())
    }

    /// Register a project from a pre-parsed manifest.
    ///
    /// Does **not** override the current project if one is already set.
    pub fn register(
        &mut self,
        manifest_path: String,
        manifest: AtlasManifest,
    ) -> &LoadedProject {
        let id = manifest.project_id.clone();
        self.projects.insert(id.clone(), LoadedProject { manifest_path, manifest });
        if self.current_id.is_none() {
            self.current_id = Some(id.clone());
        }
        // SAFETY: we just inserted `id`
        self.projects.get(&id).unwrap()
    }

    /// Return the currently active project, if any.
    pub fn current(&self) -> Option<&LoadedProject> {
        self.current_id.as_ref().and_then(|id| self.projects.get(id))
    }

    /// Activate a project by its `projectId`.  Returns `false` if not found.
    pub fn set_current(&mut self, project_id: &str) -> bool {
        if self.projects.contains_key(project_id) {
            self.current_id = Some(project_id.to_string());
            true
        } else {
            false
        }
    }

    /// Look up a project by its `projectId`.
    pub fn get(&self, project_id: &str) -> Option<&LoadedProject> {
        self.projects.get(project_id)
    }

    /// Number of projects currently registered.
    pub fn count(&self) -> usize {
        self.projects.len()
    }

    /// Remove a project by ID.  If it was the current project, the current is
    /// cleared.
    pub fn remove(&mut self, project_id: &str) -> bool {
        let removed = self.projects.remove(project_id).is_some();
        if removed {
            if self.current_id.as_deref() == Some(project_id) {
                self.current_id = None;
            }
        }
        removed
    }
}

impl Default for ProjectRegistry {
    fn default() -> Self { Self::new() }
}

// ── Tests ────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    fn make_manifest(id: &str) -> AtlasManifest {
        AtlasManifest::from_json(&format!(
            r#"{{"projectId":"{id}","projectName":"Test","projectType":"test"}}"#
        )).unwrap()
    }

    #[test]
    fn empty_registry_has_no_current() {
        let reg = ProjectRegistry::new();
        assert!(reg.current().is_none());
        assert_eq!(reg.count(), 0);
    }

    #[test]
    fn register_sets_current_when_empty() {
        let mut reg = ProjectRegistry::new();
        reg.register("a.atlas".into(), make_manifest("proj_a"));
        assert_eq!(reg.current().unwrap().id(), "proj_a");
        assert_eq!(reg.count(), 1);
    }

    #[test]
    fn register_second_project_does_not_override_current() {
        let mut reg = ProjectRegistry::new();
        reg.register("a.atlas".into(), make_manifest("proj_a"));
        reg.register("b.atlas".into(), make_manifest("proj_b"));
        assert_eq!(reg.current().unwrap().id(), "proj_a");
        assert_eq!(reg.count(), 2);
    }

    #[test]
    fn set_current_switches_active_project() {
        let mut reg = ProjectRegistry::new();
        reg.register("a.atlas".into(), make_manifest("proj_a"));
        reg.register("b.atlas".into(), make_manifest("proj_b"));
        assert!(reg.set_current("proj_b"));
        assert_eq!(reg.current().unwrap().id(), "proj_b");
    }

    #[test]
    fn set_current_unknown_returns_false() {
        let mut reg = ProjectRegistry::new();
        assert!(!reg.set_current("no_such_project"));
    }

    #[test]
    fn get_returns_project_by_id() {
        let mut reg = ProjectRegistry::new();
        reg.register("a.atlas".into(), make_manifest("proj_a"));
        assert!(reg.get("proj_a").is_some());
        assert!(reg.get("proj_b").is_none());
    }

    #[test]
    fn remove_clears_current_if_active() {
        let mut reg = ProjectRegistry::new();
        reg.register("a.atlas".into(), make_manifest("proj_a"));
        assert!(reg.remove("proj_a"));
        assert!(reg.current().is_none());
        assert_eq!(reg.count(), 0);
    }

    #[test]
    fn load_from_disk_missing_file_returns_error() {
        let mut reg = ProjectRegistry::new();
        assert!(reg.load("/tmp/no_such_manifest.atlas").is_err());
    }

    #[test]
    fn loaded_project_name_and_id() {
        let m = make_manifest("my_proj");
        let lp = LoadedProject { manifest_path: "x.atlas".into(), manifest: m };
        assert_eq!(lp.id(), "my_proj");
        assert_eq!(lp.name(), "Test");
    }
}
