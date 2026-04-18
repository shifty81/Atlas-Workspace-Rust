#[derive(Debug, Clone)]
pub struct ReplayVersionInfo {
    pub version: u32,
    pub description: String,
    pub deprecated: bool,
}

#[derive(Debug, Clone, PartialEq)]
pub enum ReplayCompatibility {
    Compatible,
    Upgradeable,
    TooOld,
    TooNew,
    Unknown,
}

#[derive(Debug, Default)]
pub struct ReplayVersionRegistry {
    current_version: u32,
    minimum_version: u32,
    versions: Vec<ReplayVersionInfo>,
}

impl ReplayVersionRegistry {
    pub fn new() -> Self { Self::default() }

    pub fn set_current_version(&mut self, v: u32) { self.current_version = v; }
    pub fn current_version(&self) -> u32 { self.current_version }

    pub fn set_minimum_version(&mut self, v: u32) { self.minimum_version = v; }
    pub fn minimum_version(&self) -> u32 { self.minimum_version }

    pub fn register_version(&mut self, info: ReplayVersionInfo) {
        self.versions.push(info);
    }

    pub fn version_count(&self) -> usize { self.versions.len() }

    pub fn check_compatibility(&self, version: u32) -> ReplayCompatibility {
        let known = self.versions.iter().any(|v| v.version == version);
        if !known { return ReplayCompatibility::Unknown; }
        if version < self.minimum_version { return ReplayCompatibility::TooOld; }
        if version > self.current_version { return ReplayCompatibility::TooNew; }
        if version < self.current_version { return ReplayCompatibility::Upgradeable; }
        ReplayCompatibility::Compatible
    }

    pub fn get_version_info(&self, version: u32) -> Option<&ReplayVersionInfo> {
        self.versions.iter().find(|v| v.version == version)
    }

    pub fn can_migrate(&self, from: u32) -> bool {
        from >= self.minimum_version && from <= self.current_version
    }

    pub fn all_versions(&self) -> Vec<&ReplayVersionInfo> { self.versions.iter().collect() }

    pub fn deprecated_versions(&self) -> Vec<u32> {
        self.versions.iter().filter(|v| v.deprecated).map(|v| v.version).collect()
    }

    pub fn register_defaults(&mut self) {
        self.register_version(ReplayVersionInfo { version: 1, description: "Initial replay format".into(), deprecated: true });
        self.register_version(ReplayVersionInfo { version: 2, description: "Added state hash".into(), deprecated: false });
        self.register_version(ReplayVersionInfo { version: 3, description: "Current format with save points".into(), deprecated: false });
        self.set_current_version(3);
        self.set_minimum_version(1);
    }

    pub fn clear(&mut self) {
        self.versions.clear();
        self.current_version = 0;
        self.minimum_version = 0;
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn reg_with_defaults() -> ReplayVersionRegistry {
        let mut r = ReplayVersionRegistry::new();
        r.register_defaults();
        r
    }

    #[test]
    fn register_defaults_sets_versions() {
        let r = reg_with_defaults();
        assert_eq!(r.current_version(), 3);
        assert_eq!(r.minimum_version(), 1);
        assert_eq!(r.version_count(), 3);
    }

    #[test]
    fn compatible_version() {
        let r = reg_with_defaults();
        assert_eq!(r.check_compatibility(3), ReplayCompatibility::Compatible);
    }

    #[test]
    fn upgradeable_version() {
        let r = reg_with_defaults();
        assert_eq!(r.check_compatibility(2), ReplayCompatibility::Upgradeable);
    }

    #[test]
    fn too_old_version() {
        let mut r = reg_with_defaults();
        r.set_minimum_version(2);
        assert_eq!(r.check_compatibility(1), ReplayCompatibility::TooOld);
    }

    #[test]
    fn too_new_version() {
        let mut r = reg_with_defaults();
        r.register_version(ReplayVersionInfo { version: 99, description: "future".into(), deprecated: false });
        // 99 is known but above current_version=3 → TooNew
        assert_eq!(r.check_compatibility(99), ReplayCompatibility::TooNew);
    }

    #[test]
    fn unknown_version() {
        let r = reg_with_defaults();
        assert_eq!(r.check_compatibility(42), ReplayCompatibility::Unknown);
    }

    #[test]
    fn can_migrate_range() {
        let r = reg_with_defaults();
        assert!(r.can_migrate(1));
        assert!(r.can_migrate(3));
        assert!(!r.can_migrate(0));
        assert!(!r.can_migrate(4));
    }

    #[test]
    fn get_version_info() {
        let r = reg_with_defaults();
        let info = r.get_version_info(1).unwrap();
        assert!(info.deprecated);
        let info2 = r.get_version_info(3).unwrap();
        assert!(!info2.deprecated);
    }

    #[test]
    fn deprecated_versions_listed() {
        let r = reg_with_defaults();
        let depr = r.deprecated_versions();
        assert!(depr.contains(&1));
        assert!(!depr.contains(&3));
    }

    #[test]
    fn clear_resets_registry() {
        let mut r = reg_with_defaults();
        r.clear();
        assert_eq!(r.version_count(), 0);
        assert_eq!(r.current_version(), 0);
        assert_eq!(r.minimum_version(), 0);
    }

    #[test]
    fn all_versions_returns_full_list() {
        let r = reg_with_defaults();
        assert_eq!(r.all_versions().len(), 3);
    }
}
