//! [`AtlasManifest`] — deserializes a `.atlas` JSON project manifest.
//!
//! A `.atlas` file is the root descriptor for a project hosted inside the
//! Atlas Workspace.  It contains the project identity, adapter type, content
//! roots, runtime settings, and optional capability declarations.
//!
//! ## Example
//!
//! ```json
//! {
//!   "schema": "atlas.project.v1",
//!   "projectId": "novaforge",
//!   "projectName": "NovaForge",
//!   "projectType": "novaforge",
//!   "projectVersion": 1,
//!   "name": "NovaForge",
//!   "version": "0.1.0",
//!   "description": "Open-world voxel MMO on the Atlas engine.",
//!   "adapter": "novaforge",
//!   "roots": { "content": "Content", "data": "Data" },
//!   "runtime": { "tickRate": 30, "maxPlayers": 64 }
//! }
//! ```

use serde::{Deserialize, Serialize};
use std::collections::HashMap;

// ── AtlasManifest ────────────────────────────────────────────────────────────

/// Root structure of a `.atlas` project manifest file (JSON, schema v1).
#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct AtlasManifest {
    /// Schema identifier — expected to be `"atlas.project.v1"`.
    #[serde(default)]
    pub schema: String,

    /// Stable, machine-readable project identifier (e.g. `"novaforge"`).
    pub project_id: String,

    /// Human-readable project name (e.g. `"NovaForge"`).
    pub project_name: String,

    /// Project type tag used to select the adapter (e.g. `"novaforge"`).
    #[serde(default)]
    pub project_type: String,

    /// Monotonically increasing format version for the manifest itself.
    #[serde(default)]
    pub project_version: u32,

    /// Display name (may duplicate `project_name` for UI convenience).
    #[serde(default)]
    pub name: String,

    /// SemVer string for the project content (e.g. `"0.1.0"`).
    #[serde(default)]
    pub version: String,

    /// Free-text description shown in the project picker.
    #[serde(default)]
    pub description: String,

    /// Adapter key that the editor uses to locate the registered adapter
    /// (e.g. `"novaforge"`).
    #[serde(default)]
    pub adapter: String,

    /// Engine capability flags declared by this project.
    #[serde(default)]
    pub capabilities: Vec<String>,

    /// Filesystem roots relative to the manifest directory.
    #[serde(default)]
    pub roots: AtlasManifestRoots,

    /// Registry file paths relative to the manifest directory.
    #[serde(default)]
    pub registries: HashMap<String, String>,

    /// Editor startup preferences.
    #[serde(default)]
    pub startup: AtlasManifestStartup,

    /// Runtime/game-loop parameters.
    #[serde(default)]
    pub runtime: AtlasManifestRuntime,
}

impl AtlasManifest {
    /// Parse a manifest from a JSON string.
    pub fn from_json(json: &str) -> Result<Self, String> {
        serde_json::from_str(json).map_err(|e| format!("manifest parse error: {e}"))
    }

    /// Load and parse a manifest from a file on disk.
    pub fn from_file(path: &str) -> Result<Self, String> {
        let content = std::fs::read_to_string(path)
            .map_err(|e| format!("cannot read '{path}': {e}"))?;
        Self::from_json(&content)
    }

    /// Returns `true` if the manifest declares the given capability.
    pub fn has_capability(&self, cap: &str) -> bool {
        self.capabilities.iter().any(|c| c == cap)
    }
}

// ── Sub-structures ────────────────────────────────────────────────────────────

/// Filesystem root directories, all relative to the manifest file.
#[derive(Debug, Clone, Serialize, Deserialize, Default)]
pub struct AtlasManifestRoots {
    #[serde(default)] pub content:   String,
    #[serde(default)] pub data:      String,
    #[serde(default)] pub config:    String,
    #[serde(default)] pub schemas:   String,
    #[serde(default)] pub generated: String,
    #[serde(default)] pub cache:     String,
}

/// Editor startup preferences read from the manifest.
#[derive(Debug, Clone, Serialize, Deserialize, Default)]
pub struct AtlasManifestStartup {
    /// Default layout preset ID (e.g. `"novaforge_default"`).
    #[serde(default)] pub layout: String,
    /// Tool to activate on startup (e.g. `"scene"`).
    #[serde(default)] pub tool:   String,
    /// World file to open on startup, relative to the manifest directory.
    #[serde(default)] pub world:  String,
}

/// Runtime / game-loop settings embedded in the manifest.
#[derive(Debug, Clone, Serialize, Deserialize, Default)]
#[serde(rename_all = "camelCase")]
pub struct AtlasManifestRuntime {
    /// Entry world file path (relative to manifest directory).
    #[serde(default)] pub entry_world: String,
    /// Simulation tick rate in Hz (0 = use engine default).
    #[serde(default)] pub tick_rate:   u32,
    /// Maximum simultaneous players (0 = singleplayer / unlimited).
    #[serde(default)] pub max_players: u32,
}

// ── Tests ────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    const NOVAFORGE_JSON: &str = r#"{
        "schema": "atlas.project.v1",
        "projectId": "novaforge",
        "projectName": "NovaForge",
        "projectType": "novaforge",
        "projectVersion": 1,
        "name": "NovaForge",
        "version": "0.1.0",
        "description": "NovaForge test manifest",
        "adapter": "novaforge",
        "capabilities": ["Rendering3D", "Physics3D", "Networking"],
        "roots": { "content": "Content", "data": "Data", "config": "Config" },
        "runtime": { "tickRate": 30, "maxPlayers": 64 }
    }"#;

    #[test]
    fn parse_novaforge_manifest() {
        let m = AtlasManifest::from_json(NOVAFORGE_JSON).unwrap();
        assert_eq!(m.project_id, "novaforge");
        assert_eq!(m.project_name, "NovaForge");
        assert_eq!(m.adapter, "novaforge");
    }

    #[test]
    fn runtime_fields() {
        let m = AtlasManifest::from_json(NOVAFORGE_JSON).unwrap();
        assert_eq!(m.runtime.tick_rate, 30);
        assert_eq!(m.runtime.max_players, 64);
    }

    #[test]
    fn roots_parsed() {
        let m = AtlasManifest::from_json(NOVAFORGE_JSON).unwrap();
        assert_eq!(m.roots.content, "Content");
        assert_eq!(m.roots.data, "Data");
    }

    #[test]
    fn capabilities_parsed() {
        let m = AtlasManifest::from_json(NOVAFORGE_JSON).unwrap();
        assert!(m.has_capability("Rendering3D"));
        assert!(m.has_capability("Networking"));
        assert!(!m.has_capability("VoxelEditor"));
    }

    #[test]
    fn malformed_json_returns_error() {
        assert!(AtlasManifest::from_json("{ not json }").is_err());
    }

    #[test]
    fn missing_optional_fields_use_defaults() {
        let m = AtlasManifest::from_json(
            r#"{"projectId":"x","projectName":"X","projectType":"x"}"#
        ).unwrap();
        assert_eq!(m.project_version, 0);
        assert!(m.capabilities.is_empty());
        assert_eq!(m.runtime.tick_rate, 0);
    }

    #[test]
    fn from_file_missing_path_errors() {
        assert!(AtlasManifest::from_file("/tmp/no_such_file_xyz.atlas").is_err());
    }
}
