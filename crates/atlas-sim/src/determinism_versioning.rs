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

#[cfg(test)]
mod tests {
    use super::*;

    fn v(major: u32, minor: u32, patch: u32) -> DeterminismVersion {
        DeterminismVersion { major, minor, patch, hash: 0, description: String::new() }
    }

    fn fork(name: &str, compat: bool) -> ForkInfo {
        ForkInfo {
            name: name.into(),
            base_version: v(1, 0, 0),
            current_version: v(1, 0, 0),
            is_compatible: compat,
        }
    }

    #[test]
    fn set_and_get_version() {
        let mut r = DeterminismVersionRegistry::new();
        r.set_current_version(v(2, 3, 1));
        let cur = r.get_current_version();
        assert_eq!(cur.major, 2);
        assert_eq!(cur.minor, 3);
        assert_eq!(cur.patch, 1);
    }

    #[test]
    fn register_and_lookup_fork() {
        let mut r = DeterminismVersionRegistry::new();
        r.register_fork(fork("server", true));
        assert_eq!(r.fork_count(), 1);
        let f = r.get_fork("server").unwrap();
        assert_eq!(f.name, "server");
        assert!(f.is_compatible);
    }

    #[test]
    fn unregister_fork() {
        let mut r = DeterminismVersionRegistry::new();
        r.register_fork(fork("client", true));
        r.unregister_fork("client");
        assert_eq!(r.fork_count(), 0);
        assert!(r.get_fork("client").is_none());
    }

    #[test]
    fn check_compatibility_per_fork() {
        let mut r = DeterminismVersionRegistry::new();
        r.register_fork(fork("ok", true));
        r.register_fork(fork("bad", false));
        assert!(r.check_compatibility("ok"));
        assert!(!r.check_compatibility("bad"));
        assert!(!r.check_compatibility("unknown")); // non-existent → false
    }

    #[test]
    fn check_all_compatibility_lists_incompatible() {
        let mut r = DeterminismVersionRegistry::new();
        r.register_fork(fork("ok", true));
        r.register_fork(fork("broken", false));
        let incompat = r.check_all_compatibility();
        assert!(incompat.contains(&"broken".to_string()));
        assert!(!incompat.contains(&"ok".to_string()));
    }

    #[test]
    fn generate_report_contains_fork_info() {
        let mut r = DeterminismVersionRegistry::new();
        r.set_current_version(v(1, 0, 0));
        r.register_fork(fork("demo", true));
        let report = r.generate_report();
        assert!(report.contains("demo"));
        assert!(report.contains("compatible"));
    }

    #[test]
    fn clear_resets_all() {
        let mut r = DeterminismVersionRegistry::new();
        r.set_current_version(v(2, 0, 0));
        r.register_fork(fork("a", true));
        r.clear();
        assert_eq!(r.fork_count(), 0);
        assert_eq!(r.get_current_version().major, 0);
    }

    #[test]
    fn list_forks_returns_all() {
        let mut r = DeterminismVersionRegistry::new();
        r.register_fork(fork("x", true));
        r.register_fork(fork("y", false));
        assert_eq!(r.list_forks().len(), 2);
    }
}
