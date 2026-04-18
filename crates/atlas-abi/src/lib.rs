#![allow(dead_code)]

use std::collections::HashMap;
use std::sync::Arc;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct AbiVersion {
    pub major: u32,
    pub minor: u32,
}

impl AbiVersion {
    pub fn new(major: u32, minor: u32) -> Self { Self { major, minor } }

    pub fn to_string(&self) -> String {
        format!("atlas_abi_v{}_{}", self.major, self.minor)
    }

    pub fn from_str(s: &str) -> Option<AbiVersion> {
        let s = s.strip_prefix("atlas_abi_v")?;
        let mut parts = s.splitn(2, '_');
        let major = parts.next()?.parse().ok()?;
        let minor = parts.next()?.parse().ok()?;
        Some(AbiVersion { major, minor })
    }

    pub fn is_compatible_with(&self, other: &AbiVersion) -> bool {
        self.major == other.major
    }
}

impl PartialOrd for AbiVersion {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.major.cmp(&other.major).then(self.minor.cmp(&other.minor)))
    }
}

impl Ord for AbiVersion {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        self.major.cmp(&other.major).then(self.minor.cmp(&other.minor))
    }
}

#[derive(Debug, Clone)]
pub struct AbiFunctionTableStatus {
    pub table_version: u32,
    pub bound_count: u32,
    pub is_complete: bool,
}

impl Default for AbiFunctionTableStatus {
    fn default() -> Self { Self { table_version: 0, bound_count: 0, is_complete: false } }
}

#[derive(Debug, Clone)]
pub struct AbiCapsule {
    version: AbiVersion,
    description: String,
    sealed: bool,
    table_status: AbiFunctionTableStatus,
}

impl AbiCapsule {
    pub fn new(version: AbiVersion, description: String) -> Self {
        Self { version, description, sealed: false, table_status: AbiFunctionTableStatus::default() }
    }

    pub fn version(&self) -> &AbiVersion { &self.version }
    pub fn description(&self) -> &str { &self.description }
    pub fn is_ready(&self) -> bool { self.table_status.is_complete }
    pub fn seal(&mut self) { self.sealed = true; }
    pub fn is_sealed(&self) -> bool { self.sealed }
    pub fn table_status(&self) -> &AbiFunctionTableStatus { &self.table_status }
    pub fn set_bound_count(&mut self, n: u32) { self.table_status.bound_count = n; }
    pub fn set_complete(&mut self, b: bool) { self.table_status.is_complete = b; }
}

#[derive(Debug, Clone)]
pub struct ProjectAbiTarget {
    pub project_name: String,
    pub target_abi: AbiVersion,
    pub determinism_profile: String,
}

#[derive(Debug, Default)]
pub struct AbiRegistry {
    capsules: HashMap<String, Arc<AbiCapsule>>,
    bindings: HashMap<String, Arc<AbiCapsule>>,
}

impl AbiRegistry {
    pub fn new() -> Self { Self::default() }

    pub fn register_capsule(&mut self, capsule: AbiCapsule) {
        let key = capsule.version().to_string();
        self.capsules.insert(key, Arc::new(capsule));
    }

    pub fn get_capsule(&self, version: &AbiVersion) -> Option<Arc<AbiCapsule>> {
        self.capsules.get(&version.to_string()).cloned()
    }

    pub fn find_compatible(&self, requested: &AbiVersion) -> Option<Arc<AbiCapsule>> {
        self.capsules.values()
            .filter(|c| c.version().is_compatible_with(requested) && c.version().minor <= requested.minor)
            .max_by_key(|c| c.version().minor)
            .cloned()
    }

    pub fn has_version(&self, version: &AbiVersion) -> bool {
        self.capsules.contains_key(&version.to_string())
    }

    pub fn registered_versions(&self) -> Vec<AbiVersion> {
        self.capsules.values().map(|c| c.version().clone()).collect()
    }

    pub fn capsule_count(&self) -> usize { self.capsules.len() }

    pub fn bind_project(&mut self, target: &ProjectAbiTarget) -> bool {
        if let Some(capsule) = self.find_compatible(&target.target_abi) {
            self.bindings.insert(target.project_name.clone(), capsule);
            true
        } else { false }
    }

    pub fn get_project_capsule(&self, name: &str) -> Option<Arc<AbiCapsule>> {
        self.bindings.get(name).cloned()
    }

    pub fn is_project_bound(&self, name: &str) -> bool { self.bindings.contains_key(name) }
    pub fn unbind_project(&mut self, name: &str) { self.bindings.remove(name); }
    pub fn bound_projects(&self) -> Vec<String> { self.bindings.keys().cloned().collect() }
}
