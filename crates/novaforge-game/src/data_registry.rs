//! [`DataRegistry`] — loads JSON data files from the NovaForge `Data/`
//! directory and makes them queryable by key.
//!
//! Each JSON file that matches `Data/**/*.json` is loaded into a flat map
//! keyed by its path relative to the data root.  Values are kept as raw
//! [`serde_json::Value`] objects so callers can deserialize them into any
//! type they need.
//!
//! # License
//!
//! GPL v3.0 — this file is part of `novaforge-game`.

use serde_json::Value;
use std::collections::HashMap;
use std::path::Path;

// ── DataRegistry ──────────────────────────────────────────────────────────────

/// Loads and indexes JSON data files from the NovaForge data root.
pub struct DataRegistry {
    /// Data root directory (e.g. `"NovaForge/Data"`).
    pub data_root: String,
    /// Loaded data keyed by relative path from the data root.
    entries: HashMap<String, Value>,
}

impl DataRegistry {
    /// Create an empty registry pointing at `data_root`.
    pub fn new(data_root: impl Into<String>) -> Self {
        Self { data_root: data_root.into(), entries: HashMap::new() }
    }

    /// Load all `*.json` files found under the data root.
    ///
    /// Returns the number of files successfully loaded.  Files that fail to
    /// parse are logged as warnings and skipped.
    pub fn load_all(&mut self) -> usize {
        self.entries.clear();
        let root_dir = self.data_root.clone();
        let root = Path::new(&root_dir);
        if !root.is_dir() {
            log::warn!("[DataRegistry] Data root not found: '{root_dir}'");
            return 0;
        }
        Self::walk_json_into(root, root, &mut self.entries);
        log::info!("[DataRegistry] Loaded {} data files from '{root_dir}'", self.entries.len());
        self.entries.len()
    }

    /// Insert a data entry directly (useful in tests).
    pub fn insert(&mut self, key: impl Into<String>, value: Value) {
        self.entries.insert(key.into(), value);
    }

    /// Look up a data entry by its relative path key (e.g. `"Worlds/DefaultWorld.json"`).
    pub fn get(&self, key: &str) -> Option<&Value> {
        self.entries.get(key)
    }

    /// Return all keys in the registry.
    pub fn keys(&self) -> impl Iterator<Item = &str> {
        self.entries.keys().map(|s| s.as_str())
    }

    /// Number of loaded data entries.
    pub fn len(&self) -> usize { self.entries.len() }

    /// `true` if no data has been loaded.
    pub fn is_empty(&self) -> bool { self.entries.is_empty() }

    // ── Private ───────────────────────────────────────────────────────────────

    fn walk_json_into(root: &Path, dir: &Path, entries: &mut HashMap<String, Value>) {
        let iter = match std::fs::read_dir(dir) {
            Ok(i) => i,
            Err(e) => {
                log::warn!("[DataRegistry] Cannot read '{}': {e}", dir.display());
                return;
            }
        };
        for item in iter.flatten() {
            let path = item.path();
            if path.is_dir() {
                Self::walk_json_into(root, &path, entries);
            } else if path.extension().and_then(|e| e.to_str()) == Some("json") {
                let Ok(rel) = path.strip_prefix(root) else { continue };
                let key = rel.to_string_lossy().replace('\\', "/");
                match std::fs::read_to_string(&path) {
                    Ok(text) => match serde_json::from_str::<Value>(&text) {
                        Ok(v)  => { entries.insert(key, v); }
                        Err(e) => log::warn!(
                            "[DataRegistry] Parse error in '{}': {e}", path.display()
                        ),
                    },
                    Err(e) => log::warn!(
                        "[DataRegistry] Read error for '{}': {e}", path.display()
                    ),
                }
            }
        }
    }
}

// ── Tests ────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use std::path::PathBuf;

    fn make_temp_data_dir() -> PathBuf {
        let p = std::env::temp_dir().join(format!(
            "nf_data_reg_test_{}",
            std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .unwrap_or_default()
                .subsec_nanos()
        ));
        std::fs::create_dir_all(&p).expect("temp dir");
        p
    }

    #[test]
    fn missing_data_root_loads_zero() {
        let mut reg = DataRegistry::new("/tmp/no_such_nf_data_xyz");
        assert_eq!(reg.load_all(), 0);
    }

    #[test]
    fn loads_json_files_from_disk() {
        let dir = make_temp_data_dir();
        std::fs::write(dir.join("world.json"), r#"{"name":"TestWorld"}"#).unwrap();
        let mut reg = DataRegistry::new(dir.to_string_lossy().to_string());
        assert_eq!(reg.load_all(), 1);
        let v = reg.get("world.json").unwrap();
        assert_eq!(v["name"], "TestWorld");
    }

    #[test]
    fn ignores_non_json_files() {
        let dir = make_temp_data_dir();
        std::fs::write(dir.join("file.txt"), b"text").unwrap();
        let mut reg = DataRegistry::new(dir.to_string_lossy().to_string());
        assert_eq!(reg.load_all(), 0);
    }

    #[test]
    fn malformed_json_is_skipped() {
        let dir = make_temp_data_dir();
        std::fs::write(dir.join("bad.json"), b"{ not valid }").unwrap();
        let mut reg = DataRegistry::new(dir.to_string_lossy().to_string());
        // Should not panic; bad file is skipped
        reg.load_all();
        assert!(reg.get("bad.json").is_none());
    }

    #[test]
    fn recursive_walk_finds_nested_json() {
        let dir = make_temp_data_dir();
        let sub = dir.join("Worlds");
        std::fs::create_dir_all(&sub).unwrap();
        std::fs::write(sub.join("DefaultWorld.json"), r#"{"id":"world1"}"#).unwrap();
        let mut reg = DataRegistry::new(dir.to_string_lossy().to_string());
        assert_eq!(reg.load_all(), 1);
        assert!(reg.get("Worlds/DefaultWorld.json").is_some());
    }

    #[test]
    fn insert_and_get_works() {
        let mut reg = DataRegistry::new(".");
        reg.insert("test/key.json", serde_json::json!({ "x": 42 }));
        assert_eq!(reg.get("test/key.json").unwrap()["x"], 42);
        assert_eq!(reg.len(), 1);
    }

    #[test]
    fn keys_iterator_covers_all_entries() {
        let mut reg = DataRegistry::new(".");
        reg.insert("a.json", serde_json::json!({}));
        reg.insert("b.json", serde_json::json!({}));
        let keys: Vec<&str> = reg.keys().collect();
        assert_eq!(keys.len(), 2);
    }
}
