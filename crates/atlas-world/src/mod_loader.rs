use std::collections::HashMap;

#[derive(Debug, Clone)]
pub struct ModDescriptor {
    pub id: String,
    pub name: String,
    pub version: String,
    pub author: String,
    pub description: String,
    pub dependencies: Vec<String>,
    pub entry_path: String,
    pub enabled: bool,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum ModLoadResult {
    Success,
    NotFound,
    InvalidDescriptor,
    MissingDependency,
    AlreadyLoaded,
}

#[derive(Default)]
pub struct ModLoader {
    mods: HashMap<String, ModDescriptor>,
    load_order: Vec<String>,
}

impl ModLoader {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn register_mod(&mut self, descriptor: ModDescriptor) -> ModLoadResult {
        if self.mods.contains_key(&descriptor.id) {
            return ModLoadResult::AlreadyLoaded;
        }
        if descriptor.id.is_empty() || descriptor.name.is_empty() {
            return ModLoadResult::InvalidDescriptor;
        }
        let id = descriptor.id.clone();
        self.mods.insert(id.clone(), descriptor);
        self.load_order.push(id);
        ModLoadResult::Success
    }

    pub fn unregister_mod(&mut self, id: &str) {
        self.mods.remove(id);
        self.load_order.retain(|i| i != id);
    }

    pub fn load_mod(&mut self, id: &str) -> ModLoadResult {
        let descriptor = match self.mods.get(id) {
            Some(d) => d.clone(),
            None => return ModLoadResult::NotFound,
        };
        // Check dependencies
        for dep in &descriptor.dependencies {
            if !self.mods.contains_key(dep) {
                return ModLoadResult::MissingDependency;
            }
        }
        if let Some(m) = self.mods.get_mut(id) {
            m.enabled = true;
        }
        log::info!("Loaded mod: {} v{}", descriptor.name, descriptor.version);
        ModLoadResult::Success
    }

    pub fn unload_mod(&mut self, id: &str) {
        if let Some(m) = self.mods.get_mut(id) {
            m.enabled = false;
        }
    }

    pub fn is_loaded(&self, id: &str) -> bool {
        self.mods.get(id).map_or(false, |m| m.enabled)
    }

    pub fn get_mod(&self, id: &str) -> Option<&ModDescriptor> {
        self.mods.get(id)
    }

    pub fn mod_count(&self) -> usize {
        self.mods.len()
    }

    pub fn loaded_mod_count(&self) -> usize {
        self.mods.values().filter(|m| m.enabled).count()
    }

    pub fn load_order(&self) -> &[String] {
        &self.load_order
    }

    pub fn resolve_load_order(&mut self) -> bool {
        let mut visited = std::collections::HashSet::new();
        let mut order = Vec::new();
        let ids: Vec<String> = self.mods.keys().cloned().collect();

        fn visit(
            id: &str,
            mods: &HashMap<String, ModDescriptor>,
            visited: &mut std::collections::HashSet<String>,
            order: &mut Vec<String>,
        ) -> bool {
            if visited.contains(id) {
                return true;
            }
            visited.insert(id.to_owned());
            if let Some(m) = mods.get(id) {
                for dep in &m.dependencies {
                    if !visit(dep, mods, visited, order) {
                        return false;
                    }
                }
            }
            order.push(id.to_owned());
            true
        }

        for id in &ids {
            if !visit(id, &self.mods, &mut visited, &mut order) {
                return false;
            }
        }
        self.load_order = order;
        true
    }
}
