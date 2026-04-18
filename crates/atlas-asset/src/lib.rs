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
