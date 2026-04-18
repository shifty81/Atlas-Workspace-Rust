#[derive(Debug, Clone, PartialEq, Eq)]
pub struct SchemaVersion {
    pub major: u32,
    pub minor: u32,
}

impl PartialOrd for SchemaVersion {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}

impl Ord for SchemaVersion {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        self.major.cmp(&other.major).then(self.minor.cmp(&other.minor))
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

#[cfg(test)]
mod tests {
    use super::*;

    fn v(major: u32, minor: u32) -> SchemaVersion { SchemaVersion { major, minor } }

    #[test]
    fn serialize_then_validate_succeeds() {
        let s = WorldStateSerializer::new();
        let data = b"hello world";
        let state = s.serialize(data);
        assert_eq!(state.version, v(1, 0));
        assert_eq!(s.validate(&state), SerializerResult::Success);
    }

    #[test]
    fn hash_mismatch_detected() {
        let s = WorldStateSerializer::new();
        let mut state = s.serialize(b"original");
        state.hash ^= 1; // corrupt the hash
        assert_eq!(s.validate(&state), SerializerResult::HashMismatch);
    }

    #[test]
    fn version_too_old() {
        let mut s = WorldStateSerializer::new();
        s.set_minimum_version(v(2, 0));
        let mut state = s.serialize(b"data");
        state.version = v(1, 0);
        assert_eq!(s.validate(&state), SerializerResult::VersionTooOld);
    }

    #[test]
    fn version_too_new() {
        let s = WorldStateSerializer::new();
        let mut state = s.serialize(b"data");
        state.version = v(99, 0);
        assert_eq!(s.validate(&state), SerializerResult::VersionTooNew);
    }

    #[test]
    fn deserialize_calls_validate() {
        let s = WorldStateSerializer::new();
        let mut state = s.serialize(b"test");
        assert_eq!(s.deserialize(&mut state), SerializerResult::Success);
    }

    #[test]
    fn can_migrate_within_range() {
        let mut s = WorldStateSerializer::new();
        s.set_current_version(v(2, 0));
        s.set_minimum_version(v(1, 0));
        assert!(s.can_migrate(v(1, 0)));
        assert!(s.can_migrate(v(2, 0)));
        assert!(!s.can_migrate(v(0, 9)));
        assert!(!s.can_migrate(v(3, 0)));
    }

    #[test]
    fn schema_version_ordering() {
        assert!(v(1, 0) < v(2, 0));
        assert!(v(1, 1) < v(1, 2));
        assert!(v(1, 0) == v(1, 0));
    }

    #[test]
    fn serialize_different_data_gives_different_hashes() {
        let s = WorldStateSerializer::new();
        let a = s.serialize(b"aaa");
        let b = s.serialize(b"bbb");
        assert_ne!(a.hash, b.hash);
    }
}
