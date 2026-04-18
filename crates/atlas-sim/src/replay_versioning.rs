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
