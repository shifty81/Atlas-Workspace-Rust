#[derive(Debug, Clone, PartialEq, Eq)]
pub struct SchemaVersion {
    pub major: u32,
    pub minor: u32,
}

impl PartialOrd for SchemaVersion {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.major.cmp(&other.major).then(self.minor.cmp(&other.minor)))
    }
}

#[derive(Debug, Clone, PartialEq)]
pub enum SerializerResult {
    Success,
    VersionTooOld,
    VersionTooNew,
    MigrationFailed,
    HashMismatch,
    InvalidData,
}

#[derive(Debug, Clone)]
pub struct SerializedState {
    pub version: SchemaVersion,
    pub hash: u64,
    pub data: Vec<u8>,
}

fn fnv1a(data: &[u8]) -> u64 {
    let mut h: u64 = 14695981039346656037u64;
    for &b in data {
        h ^= b as u64;
        h = h.wrapping_mul(0x100000001b3u64);
    }
    h
}

#[derive(Debug)]
pub struct WorldStateSerializer {
    current_version: SchemaVersion,
    minimum_version: SchemaVersion,
}

impl Default for WorldStateSerializer {
    fn default() -> Self {
        Self {
            current_version: SchemaVersion { major: 1, minor: 0 },
            minimum_version: SchemaVersion { major: 1, minor: 0 },
        }
    }
}

impl WorldStateSerializer {
    pub fn new() -> Self { Self::default() }

    pub fn set_current_version(&mut self, v: SchemaVersion) { self.current_version = v; }
    pub fn current_version(&self) -> SchemaVersion { self.current_version.clone() }

    pub fn set_minimum_version(&mut self, v: SchemaVersion) { self.minimum_version = v; }
    pub fn minimum_version(&self) -> SchemaVersion { self.minimum_version.clone() }

    pub fn migration_count(&self) -> usize { 0 }

    pub fn serialize(&self, data: &[u8]) -> SerializedState {
        let hash = fnv1a(data);
        SerializedState { version: self.current_version.clone(), hash, data: data.to_vec() }
    }

    pub fn deserialize(&self, state: &mut SerializedState) -> SerializerResult {
        self.validate(state)
    }

    pub fn can_migrate(&self, from: SchemaVersion) -> bool {
        from >= self.minimum_version && from <= self.current_version
    }

    pub fn validate(&self, state: &SerializedState) -> SerializerResult {
        if state.version < self.minimum_version { return SerializerResult::VersionTooOld; }
        if state.version > self.current_version { return SerializerResult::VersionTooNew; }
        let expected = fnv1a(&state.data);
        if expected != state.hash { return SerializerResult::HashMismatch; }
        SerializerResult::Success
    }
}
