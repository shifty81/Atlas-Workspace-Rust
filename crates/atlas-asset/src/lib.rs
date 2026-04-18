#![allow(dead_code)]

use std::collections::HashMap;
use uuid::Uuid;

pub type AssetSeed = u64;

#[derive(Debug, Clone)]
pub struct AssetContext {
    pub seed: AssetSeed,
    pub lod: u32,
}

pub trait AssetNode: Send + Sync {
    fn evaluate(&self, ctx: &AssetContext);
    fn name(&self) -> &str;
}

#[derive(Default)]
pub struct AssetGraph {
    nodes: Vec<(String, Box<dyn AssetNode>)>,
}

impl AssetGraph {
    pub fn new() -> Self { Self::default() }

    pub fn add_node(&mut self, name: &str, node: Box<dyn AssetNode>) {
        self.nodes.push((name.to_string(), node));
    }

    pub fn remove_node(&mut self, name: &str) -> bool {
        let before = self.nodes.len();
        self.nodes.retain(|(n, _)| n != name);
        self.nodes.len() < before
    }

    pub fn evaluate(&self, ctx: &AssetContext) {
        for (_, node) in &self.nodes {
            node.evaluate(ctx);
        }
    }

    pub fn node_count(&self) -> usize { self.nodes.len() }

    pub fn get_node(&self, name: &str) -> Option<&dyn AssetNode> {
        self.nodes.iter().find(|(n, _)| n == name).map(|(_, node)| node.as_ref())
    }
}

#[derive(Debug, Clone, serde::Serialize, serde::Deserialize)]
pub struct AssetMeta {
    pub id: String,
    pub name: String,
    pub asset_type: String,
    pub path: String,
    pub version: u32,
    pub dependencies: Vec<String>,
}

impl AssetMeta {
    pub fn new(name: &str, asset_type: &str, path: &str) -> Self {
        Self {
            id: Uuid::new_v4().to_string(),
            name: name.to_string(),
            asset_type: asset_type.to_string(),
            path: path.to_string(),
            version: 1,
            dependencies: Vec::new(),
        }
    }
}

#[derive(Default)]
pub struct AssetRegistry {
    assets: HashMap<String, AssetMeta>,
}

impl AssetRegistry {
    pub fn new() -> Self { Self::default() }

    pub fn register(&mut self, meta: AssetMeta) {
        self.assets.insert(meta.id.clone(), meta);
    }

    pub fn unregister(&mut self, id: &str) -> bool {
        self.assets.remove(id).is_some()
    }

    pub fn get(&self, id: &str) -> Option<&AssetMeta> { self.assets.get(id) }

    pub fn get_by_name(&self, name: &str) -> Option<&AssetMeta> {
        self.assets.values().find(|m| m.name == name)
    }

    pub fn count(&self) -> usize { self.assets.len() }

    pub fn list_by_type(&self, asset_type: &str) -> Vec<&AssetMeta> {
        self.assets.values().filter(|m| m.asset_type == asset_type).collect()
    }

    pub fn dependencies(&self, id: &str) -> Vec<&AssetMeta> {
        let Some(meta) = self.assets.get(id) else { return Vec::new(); };
        meta.dependencies.iter()
            .filter_map(|dep_id| self.assets.get(dep_id))
            .collect()
    }

    pub fn iter(&self) -> impl Iterator<Item = &AssetMeta> {
        self.assets.values()
    }

    pub fn clear(&mut self) { self.assets.clear(); }

    pub fn serialize(&self) -> String {
        let metas: Vec<&AssetMeta> = self.assets.values().collect();
        serde_json::to_string(&metas).unwrap_or_default()
    }

    pub fn deserialize(&mut self, json: &str) -> Result<(), serde_json::Error> {
        let metas: Vec<AssetMeta> = serde_json::from_str(json)?;
        for meta in metas {
            self.assets.insert(meta.id.clone(), meta);
        }
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // ── AssetContext ──────────────────────────────────────────────────────────

    #[test]
    fn asset_context_fields() {
        let ctx = AssetContext { seed: 42, lod: 3 };
        assert_eq!(ctx.seed, 42);
        assert_eq!(ctx.lod, 3);
    }

    // ── AssetGraph ────────────────────────────────────────────────────────────

    struct DummyNode { label: String }

    impl AssetNode for DummyNode {
        fn evaluate(&self, _ctx: &AssetContext) {}
        fn name(&self) -> &str { &self.label }
    }

    fn make_node(label: &str) -> Box<dyn AssetNode> {
        Box::new(DummyNode { label: label.to_string() })
    }

    #[test]
    fn graph_starts_empty() {
        let g = AssetGraph::new();
        assert_eq!(g.node_count(), 0);
    }

    #[test]
    fn graph_add_node_increments_count() {
        let mut g = AssetGraph::new();
        g.add_node("tex", make_node("tex"));
        assert_eq!(g.node_count(), 1);
        g.add_node("mesh", make_node("mesh"));
        assert_eq!(g.node_count(), 2);
    }

    #[test]
    fn graph_get_node_by_name() {
        let mut g = AssetGraph::new();
        g.add_node("alpha", make_node("alpha"));
        assert!(g.get_node("alpha").is_some());
        assert!(g.get_node("missing").is_none());
    }

    #[test]
    fn graph_remove_existing_node() {
        let mut g = AssetGraph::new();
        g.add_node("rm", make_node("rm"));
        assert!(g.remove_node("rm"));
        assert_eq!(g.node_count(), 0);
    }

    #[test]
    fn graph_remove_nonexistent_returns_false() {
        let mut g = AssetGraph::new();
        assert!(!g.remove_node("ghost"));
    }

    #[test]
    fn graph_evaluate_does_not_panic() {
        let mut g = AssetGraph::new();
        g.add_node("n", make_node("n"));
        let ctx = AssetContext { seed: 1, lod: 0 };
        g.evaluate(&ctx);
    }

    // ── AssetMeta ─────────────────────────────────────────────────────────────

    #[test]
    fn asset_meta_new_fields() {
        let m = AssetMeta::new("Texture", "texture", "assets/tex.png");
        assert_eq!(m.name, "Texture");
        assert_eq!(m.asset_type, "texture");
        assert_eq!(m.path, "assets/tex.png");
        assert_eq!(m.version, 1);
        assert!(m.dependencies.is_empty());
        assert!(!m.id.is_empty());
    }

    #[test]
    fn asset_meta_unique_ids() {
        let a = AssetMeta::new("A", "t", "p");
        let b = AssetMeta::new("A", "t", "p");
        assert_ne!(a.id, b.id);
    }

    // ── AssetRegistry ─────────────────────────────────────────────────────────

    #[test]
    fn registry_register_and_get() {
        let mut reg = AssetRegistry::new();
        let meta = AssetMeta::new("Mesh", "mesh", "assets/box.obj");
        let id = meta.id.clone();
        reg.register(meta);
        assert_eq!(reg.count(), 1);
        assert_eq!(reg.get(&id).unwrap().name, "Mesh");
    }

    #[test]
    fn registry_get_by_name() {
        let mut reg = AssetRegistry::new();
        reg.register(AssetMeta::new("Sky", "texture", "sky.png"));
        assert!(reg.get_by_name("Sky").is_some());
        assert!(reg.get_by_name("Ground").is_none());
    }

    #[test]
    fn registry_unregister() {
        let mut reg = AssetRegistry::new();
        let meta = AssetMeta::new("X", "t", "p");
        let id = meta.id.clone();
        reg.register(meta);
        assert!(reg.unregister(&id));
        assert_eq!(reg.count(), 0);
        assert!(!reg.unregister(&id));
    }

    #[test]
    fn registry_list_by_type() {
        let mut reg = AssetRegistry::new();
        reg.register(AssetMeta::new("A", "mesh", "a.obj"));
        reg.register(AssetMeta::new("B", "mesh", "b.obj"));
        reg.register(AssetMeta::new("C", "texture", "c.png"));
        assert_eq!(reg.list_by_type("mesh").len(), 2);
        assert_eq!(reg.list_by_type("texture").len(), 1);
        assert_eq!(reg.list_by_type("audio").len(), 0);
    }

    #[test]
    fn registry_clear() {
        let mut reg = AssetRegistry::new();
        reg.register(AssetMeta::new("A", "t", "p"));
        reg.clear();
        assert_eq!(reg.count(), 0);
    }

    #[test]
    fn registry_iter() {
        let mut reg = AssetRegistry::new();
        reg.register(AssetMeta::new("A", "t", "p"));
        reg.register(AssetMeta::new("B", "t", "p"));
        assert_eq!(reg.iter().count(), 2);
    }

    #[test]
    fn registry_serialize_deserialize_round_trip() {
        let mut reg = AssetRegistry::new();
        let mut meta = AssetMeta::new("Cube", "mesh", "cube.obj");
        meta.dependencies.push("dep-uuid-123".to_string());
        reg.register(meta);

        let json = reg.serialize();
        let mut reg2 = AssetRegistry::new();
        reg2.deserialize(&json).unwrap();
        assert_eq!(reg2.count(), 1);
        let found = reg2.get_by_name("Cube").unwrap();
        assert_eq!(found.dependencies, vec!["dep-uuid-123"]);
    }

    #[test]
    fn registry_dependencies_lookup() {
        let mut reg = AssetRegistry::new();
        let dep = AssetMeta::new("DepTex", "texture", "dep.png");
        let dep_id = dep.id.clone();
        reg.register(dep);

        let mut main = AssetMeta::new("MainMesh", "mesh", "main.obj");
        main.dependencies.push(dep_id.clone());
        let main_id = main.id.clone();
        reg.register(main);

        let deps = reg.dependencies(&main_id);
        assert_eq!(deps.len(), 1);
        assert_eq!(deps[0].name, "DepTex");
    }

    #[test]
    fn registry_dependencies_missing_id() {
        let reg = AssetRegistry::new();
        assert!(reg.dependencies("nonexistent").is_empty());
    }
}

// ── AssetLoader ──────────────────────────────────────────────────────────────

/// Resolves asset file paths relative to a configurable root directory.
///
/// The root is determined in priority order:
/// 1. Explicit path provided to [`AssetLoader::new`].
/// 2. `NOVAFORGE_ASSETS_DIR` environment variable.
/// 3. `./novaforge-assets` relative to the current working directory.
#[derive(Debug, Clone)]
pub struct AssetLoader {
    root: std::path::PathBuf,
}

impl AssetLoader {
    /// Create a loader with an explicit root directory.
    pub fn new(root: impl Into<std::path::PathBuf>) -> Self {
        Self { root: root.into() }
    }

    /// Create a loader that auto-detects the root via environment variable,
    /// then falls back to `./novaforge-assets`.
    pub fn auto() -> Self {
        let root = if let Ok(dir) = std::env::var("NOVAFORGE_ASSETS_DIR") {
            if !dir.is_empty() {
                std::path::PathBuf::from(dir)
            } else {
                std::path::PathBuf::from("novaforge-assets")
            }
        } else {
            std::path::PathBuf::from("novaforge-assets")
        };
        Self { root }
    }

    /// Return the resolved root directory path.
    pub fn root(&self) -> &std::path::Path { &self.root }

    /// Resolve `relative_path` against the root, returning the full path.
    pub fn resolve(&self, relative_path: &str) -> std::path::PathBuf {
        self.root.join(relative_path)
    }

    /// Return `true` if the root directory exists on disk.
    pub fn root_exists(&self) -> bool { self.root.is_dir() }

    /// Scan the root directory (non-recursively) and register all JSON files
    /// as `AssetMeta` entries in `registry`.  Returns the number of assets found.
    ///
    /// Files that do not parse cleanly are skipped with a warning log.
    pub fn scan_into(&self, registry: &mut AssetRegistry) -> usize {
        let Ok(entries) = std::fs::read_dir(&self.root) else {
            return 0;
        };
        let mut count = 0;
        for entry in entries.flatten() {
            let path = entry.path();
            if path.extension().and_then(|e| e.to_str()) == Some("json") {
                match std::fs::read_to_string(&path) {
                    Ok(json) => {
                        if let Ok(mut metas) = serde_json::from_str::<Vec<AssetMeta>>(&json) {
                            count += metas.len();
                            for meta in metas.drain(..) {
                                registry.register(meta);
                            }
                        }
                    }
                    Err(e) => {
                        log::warn!("[AssetLoader] Could not read {}: {}", path.display(), e);
                    }
                }
            }
        }
        count
    }
}

#[cfg(test)]
mod loader_tests {
    use super::*;

    #[test]
    fn loader_new_sets_root() {
        let loader = AssetLoader::new("/tmp/assets");
        assert_eq!(loader.root(), std::path::Path::new("/tmp/assets"));
    }

    #[test]
    fn loader_resolve_joins_path() {
        let loader = AssetLoader::new("/tmp/assets");
        let resolved = loader.resolve("common/items.json");
        assert_eq!(resolved, std::path::PathBuf::from("/tmp/assets/common/items.json"));
    }

    #[test]
    fn loader_auto_uses_env_var() {
        // Set the env var and verify it is picked up
        std::env::set_var("NOVAFORGE_ASSETS_DIR", "/env/assets");
        let loader = AssetLoader::auto();
        assert_eq!(loader.root(), std::path::Path::new("/env/assets"));
        std::env::remove_var("NOVAFORGE_ASSETS_DIR");
    }

    #[test]
    fn loader_auto_fallback_when_env_empty() {
        std::env::set_var("NOVAFORGE_ASSETS_DIR", "");
        let loader = AssetLoader::auto();
        assert_eq!(loader.root(), std::path::Path::new("novaforge-assets"));
        std::env::remove_var("NOVAFORGE_ASSETS_DIR");
    }

    #[test]
    fn loader_auto_fallback_when_env_unset() {
        std::env::remove_var("NOVAFORGE_ASSETS_DIR");
        let loader = AssetLoader::auto();
        assert_eq!(loader.root(), std::path::Path::new("novaforge-assets"));
    }

    #[test]
    fn loader_root_exists_false_for_nonexistent() {
        let loader = AssetLoader::new("/this/path/does/not/exist");
        assert!(!loader.root_exists());
    }

    #[test]
    fn loader_scan_into_empty_dir_returns_zero() {
        use std::io::Write;
        // Create a temporary directory with no JSON files
        let tmp = std::env::temp_dir().join("atlas_asset_loader_test");
        std::fs::create_dir_all(&tmp).ok();
        let loader = AssetLoader::new(&tmp);
        let mut reg = AssetRegistry::new();
        let count = loader.scan_into(&mut reg);
        assert_eq!(count, 0);
        std::fs::remove_dir_all(&tmp).ok();
    }

    #[test]
    fn loader_scan_into_parses_json_assets() {
        use std::io::Write;
        let tmp = std::env::temp_dir().join("atlas_asset_loader_test_scan");
        std::fs::create_dir_all(&tmp).ok();

        // Write a minimal asset manifest
        let meta = AssetMeta::new("ScanMesh", "mesh", "scan.obj");
        let json = serde_json::to_string(&vec![meta]).unwrap();
        let mut f = std::fs::File::create(tmp.join("catalog.json")).unwrap();
        f.write_all(json.as_bytes()).unwrap();

        let loader = AssetLoader::new(&tmp);
        let mut reg = AssetRegistry::new();
        let count = loader.scan_into(&mut reg);
        assert_eq!(count, 1);
        assert!(reg.get_by_name("ScanMesh").is_some());

        std::fs::remove_dir_all(&tmp).ok();
    }
}

