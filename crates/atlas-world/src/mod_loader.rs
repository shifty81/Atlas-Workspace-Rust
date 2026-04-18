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

#[cfg(test)]
mod tests {
    use super::*;

    fn basic_mod(id: &str) -> ModDescriptor {
        ModDescriptor {
            id: id.into(),
            name: format!("Mod {id}"),
            version: "1.0".into(),
            author: "Tester".into(),
            description: String::new(),
            dependencies: Vec::new(),
            entry_path: String::new(),
            enabled: false,
        }
    }

    fn mod_with_dep(id: &str, dep: &str) -> ModDescriptor {
        let mut m = basic_mod(id);
        m.dependencies = vec![dep.into()];
        m
    }

    #[test]
    fn register_and_count() {
        let mut ml = ModLoader::new();
        assert_eq!(ml.register_mod(basic_mod("alpha")), ModLoadResult::Success);
        assert_eq!(ml.mod_count(), 1);
    }

    #[test]
    fn duplicate_register_returns_already_loaded() {
        let mut ml = ModLoader::new();
        ml.register_mod(basic_mod("a"));
        assert_eq!(ml.register_mod(basic_mod("a")), ModLoadResult::AlreadyLoaded);
        assert_eq!(ml.mod_count(), 1);
    }

    #[test]
    fn invalid_descriptor_empty_id() {
        let mut ml = ModLoader::new();
        let mut m = basic_mod("");
        m.name = "valid name".into();
        assert_eq!(ml.register_mod(m), ModLoadResult::InvalidDescriptor);
        assert_eq!(ml.mod_count(), 0);
    }

    #[test]
    fn unregister_removes_mod() {
        let mut ml = ModLoader::new();
        ml.register_mod(basic_mod("beta"));
        ml.unregister_mod("beta");
        assert_eq!(ml.mod_count(), 0);
    }

    #[test]
    fn load_mod_marks_enabled() {
        let mut ml = ModLoader::new();
        ml.register_mod(basic_mod("gamma"));
        assert_eq!(ml.load_mod("gamma"), ModLoadResult::Success);
        assert!(ml.is_loaded("gamma"));
        assert_eq!(ml.loaded_mod_count(), 1);
    }

    #[test]
    fn load_nonexistent_returns_not_found() {
        let mut ml = ModLoader::new();
        assert_eq!(ml.load_mod("missing"), ModLoadResult::NotFound);
    }

    #[test]
    fn load_with_missing_dependency_returns_error() {
        let mut ml = ModLoader::new();
        ml.register_mod(mod_with_dep("child", "parent"));
        assert_eq!(ml.load_mod("child"), ModLoadResult::MissingDependency);
    }

    #[test]
    fn load_with_satisfied_dependency_succeeds() {
        let mut ml = ModLoader::new();
        ml.register_mod(basic_mod("parent"));
        ml.register_mod(mod_with_dep("child", "parent"));
        assert_eq!(ml.load_mod("child"), ModLoadResult::Success);
    }

    #[test]
    fn unload_mod_marks_disabled() {
        let mut ml = ModLoader::new();
        ml.register_mod(basic_mod("delta"));
        ml.load_mod("delta");
        assert!(ml.is_loaded("delta"));
        ml.unload_mod("delta");
        assert!(!ml.is_loaded("delta"));
    }

    #[test]
    fn load_order_tracks_registration_sequence() {
        let mut ml = ModLoader::new();
        ml.register_mod(basic_mod("first"));
        ml.register_mod(basic_mod("second"));
        let order = ml.load_order();
        assert_eq!(order[0], "first");
        assert_eq!(order[1], "second");
    }

    #[test]
    fn resolve_load_order_respects_dependencies() {
        let mut ml = ModLoader::new();
        ml.register_mod(basic_mod("base"));
        ml.register_mod(mod_with_dep("plugin", "base"));
        assert!(ml.resolve_load_order());
        let order = ml.load_order();
        // "base" must come before "plugin"
        let base_pos = order.iter().position(|s| s == "base").unwrap();
        let plugin_pos = order.iter().position(|s| s == "plugin").unwrap();
        assert!(base_pos < plugin_pos);
    }

    #[test]
    fn get_mod_returns_descriptor() {
        let mut ml = ModLoader::new();
        ml.register_mod(basic_mod("omega"));
        let m = ml.get_mod("omega").unwrap();
        assert_eq!(m.name, "Mod omega");
    }
}
