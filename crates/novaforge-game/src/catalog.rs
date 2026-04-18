//! [`AssetCatalog`] — scans the NovaForge asset tree and registers discovered
//! assets with the `atlas-asset` [`AssetRegistry`].
//!
//! The catalog does a shallow recursive walk of the configured assets
//! directory and produces one [`AssetEntry`] per file found.
//!
//! # License
//!
//! GPL v3.0 — this file is part of `novaforge-game`.

use atlas_asset::{AssetMeta, AssetRegistry};
use std::path::{Path, PathBuf};
use uuid::Uuid;

// ── AssetEntry ────────────────────────────────────────────────────────────────

/// A single discovered asset.
#[derive(Debug, Clone)]
pub struct AssetEntry {
    /// Stable UUID for this asset.
    pub id: Uuid,
    /// File path relative to the scan root.
    pub relative_path: PathBuf,
    /// Absolute file path.
    pub absolute_path: PathBuf,
    /// Asset category inferred from the file extension (e.g. `"mesh"`, `"texture"`).
    pub category: String,
}

impl AssetEntry {
    /// Infer the asset category from a file extension.
    pub fn category_for_extension(ext: &str) -> &'static str {
        match ext {
            "vox" | "obj" | "gltf" | "glb" => "mesh",
            "png" | "jpg" | "jpeg" | "webp" | "dds" => "texture",
            "ogg" | "wav" | "flac" | "mp3" => "audio",
            "atlas" | "json" => "data",
            "glsl" | "wgsl" | "vert" | "frag" | "comp" => "shader",
            "anim" => "animation",
            _ => "unknown",
        }
    }
}

// ── AssetCatalog ──────────────────────────────────────────────────────────────

/// Scans the NovaForge asset directories and registers assets.
pub struct AssetCatalog {
    /// Root directory to scan (from config, env var, or default).
    pub root_dir: String,
    /// All discovered assets (populated by [`scan`]).
    ///
    /// [`scan`]: AssetCatalog::scan
    pub entries: Vec<AssetEntry>,
}

impl AssetCatalog {
    /// Create a catalog rooted at `root_dir`.
    pub fn new(root_dir: impl Into<String>) -> Self {
        Self { root_dir: root_dir.into(), entries: Vec::new() }
    }

    /// Scan the root directory tree and populate [`self.entries`].
    ///
    /// Returns the number of assets discovered.  If the root directory does
    /// not exist the scan succeeds with zero entries.
    ///
    /// [`self.entries`]: AssetCatalog::entries
    pub fn scan(&mut self) -> usize {
        self.entries.clear();
        let root_dir = self.root_dir.clone();
        let root = Path::new(&root_dir);
        if !root.is_dir() {
            log::warn!("[AssetCatalog] Root directory not found: '{root_dir}'");
            return 0;
        }
        Self::walk_into(root, root, &mut self.entries);
        log::info!("[AssetCatalog] Scanned {} assets from '{root_dir}'", self.entries.len());
        self.entries.len()
    }

    /// Scan and then register all discovered assets into `registry`.
    ///
    /// Returns the number of newly registered assets.
    pub fn scan_into(&mut self, registry: &mut AssetRegistry) -> usize {
        let count = self.scan();
        for entry in &self.entries {
            let rel = entry.relative_path.to_string_lossy();
            let abs = entry.absolute_path.to_string_lossy();
            let meta = AssetMeta::new(&rel, &entry.category, &abs);
            registry.register(meta);
        }
        count
    }

    /// Total number of entries discovered in the last scan.
    pub fn len(&self) -> usize { self.entries.len() }

    /// `true` if no assets were found in the last scan.
    pub fn is_empty(&self) -> bool { self.entries.is_empty() }

    // ── Private ───────────────────────────────────────────────────────────────

    fn walk_into(root: &Path, dir: &Path, entries: &mut Vec<AssetEntry>) {
        let iter = match std::fs::read_dir(dir) {
            Ok(i) => i,
            Err(e) => {
                log::warn!("[AssetCatalog] Cannot read '{}': {e}", dir.display());
                return;
            }
        };
        for item in iter.flatten() {
            let path = item.path();
            if path.is_dir() {
                Self::walk_into(root, &path, entries);
            } else if path.is_file() {
                if let Ok(rel) = path.strip_prefix(root) {
                    let ext = path.extension()
                        .and_then(|e| e.to_str())
                        .unwrap_or("")
                        .to_lowercase();
                    let category = AssetEntry::category_for_extension(&ext).to_string();
                    entries.push(AssetEntry {
                        id:            Uuid::new_v4(),
                        relative_path: rel.to_path_buf(),
                        absolute_path: path,
                        category,
                    });
                }
            }
        }
    }
}

// ── Tests ────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    fn tempdir() -> PathBuf {
        let p = std::env::temp_dir().join(format!(
            "nf_catalog_test_{}",
            std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .unwrap_or_default()
                .subsec_nanos()
        ));
        std::fs::create_dir_all(&p).expect("temp dir");
        p
    }

    #[test]
    fn missing_dir_scans_zero_assets() {
        let mut catalog = AssetCatalog::new("/tmp/no_such_novaforge_assets_xyz");
        assert_eq!(catalog.scan(), 0);
        assert!(catalog.is_empty());
    }

    #[test]
    fn scan_empty_dir_returns_zero() {
        let dir = tempdir();
        let mut catalog = AssetCatalog::new(dir.to_string_lossy().to_string());
        assert_eq!(catalog.scan(), 0);
    }

    #[test]
    fn scan_with_files_discovers_assets() {
        let dir = tempdir();
        std::fs::write(dir.join("ship.vox"), b"voxel data").unwrap();
        std::fs::write(dir.join("texture.png"), b"png data").unwrap();
        let mut catalog = AssetCatalog::new(dir.to_string_lossy().to_string());
        let n = catalog.scan();
        assert_eq!(n, 2);
        assert_eq!(catalog.len(), 2);
    }

    #[test]
    fn category_inference() {
        assert_eq!(AssetEntry::category_for_extension("vox"),  "mesh");
        assert_eq!(AssetEntry::category_for_extension("png"),  "texture");
        assert_eq!(AssetEntry::category_for_extension("ogg"),  "audio");
        assert_eq!(AssetEntry::category_for_extension("json"), "data");
        assert_eq!(AssetEntry::category_for_extension("glsl"), "shader");
        assert_eq!(AssetEntry::category_for_extension("xyz"),  "unknown");
    }

    #[test]
    fn scan_into_registers_in_asset_registry() {
        let dir = tempdir();
        std::fs::write(dir.join("a.vox"), b"a").unwrap();
        std::fs::write(dir.join("b.vox"), b"b").unwrap();
        let mut catalog = AssetCatalog::new(dir.to_string_lossy().to_string());
        let mut registry = AssetRegistry::new();
        let n = catalog.scan_into(&mut registry);
        assert_eq!(n, 2);
        assert_eq!(registry.count(), 2);
    }

    #[test]
    fn recursive_scan_finds_nested_files() {
        let dir = tempdir();
        let sub = dir.join("ships");
        std::fs::create_dir_all(&sub).unwrap();
        std::fs::write(sub.join("frigate.vox"), b"v").unwrap();
        let mut catalog = AssetCatalog::new(dir.to_string_lossy().to_string());
        assert_eq!(catalog.scan(), 1);
    }
}

