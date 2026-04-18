#[derive(Debug, Clone)]
pub struct DeterminismVersion {
    pub major: u32,
    pub minor: u32,
    pub patch: u32,
    pub hash: u64,
    pub description: String,
}

impl Default for DeterminismVersion {
    fn default() -> Self {
        Self { major: 0, minor: 0, patch: 0, hash: 0, description: String::new() }
    }
}

#[derive(Debug, Clone)]
pub struct ForkInfo {
    pub name: String,
    pub base_version: DeterminismVersion,
    pub current_version: DeterminismVersion,
    pub is_compatible: bool,
}

#[derive(Debug, Default)]
pub struct DeterminismVersionRegistry {
    current_version: DeterminismVersion,
    forks: std::collections::HashMap<String, ForkInfo>,
}

impl DeterminismVersionRegistry {
    pub fn new() -> Self { Self::default() }

    pub fn set_current_version(&mut self, v: DeterminismVersion) { self.current_version = v; }
    pub fn get_current_version(&self) -> &DeterminismVersion { &self.current_version }

    pub fn register_fork(&mut self, fork: ForkInfo) { self.forks.insert(fork.name.clone(), fork); }
    pub fn unregister_fork(&mut self, name: &str) { self.forks.remove(name); }
    pub fn get_fork(&self, name: &str) -> Option<&ForkInfo> { self.forks.get(name) }
    pub fn list_forks(&self) -> Vec<&ForkInfo> { self.forks.values().collect() }
    pub fn fork_count(&self) -> usize { self.forks.len() }

    pub fn check_compatibility(&self, name: &str) -> bool {
        self.forks.get(name).map(|f| f.is_compatible).unwrap_or(false)
    }

    pub fn check_all_compatibility(&self) -> Vec<String> {
        self.forks.values().filter(|f| !f.is_compatible).map(|f| f.name.clone()).collect()
    }

    pub fn generate_report(&self) -> String {
        let mut s = format!("DeterminismVersionRegistry Report\n");
        s.push_str(&format!("Current Version: {}.{}.{}\n",
            self.current_version.major, self.current_version.minor, self.current_version.patch));
        s.push_str(&format!("Forks: {}\n", self.forks.len()));
        for fork in self.forks.values() {
            s.push_str(&format!("  {} compatible={}\n", fork.name, fork.is_compatible));
        }
        s
    }

    pub fn clear(&mut self) {
        self.forks.clear();
        self.current_version = DeterminismVersion::default();
    }
}
