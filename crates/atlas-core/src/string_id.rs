//! Interned string identifiers.
//!
//! A [`StringId`] wraps a `&'static str` for zero-cost equality comparison
//! and hashing, analogous to the C++ `NF::StringID`.

use std::fmt;
use std::hash::{Hash, Hasher};

/// An interned, cheaply copyable string identifier.
#[derive(Clone, Copy, Eq)]
pub struct StringId {
    inner: &'static str,
}

impl StringId {
    /// Create a `StringId` from a static string.
    pub const fn new(s: &'static str) -> Self {
        Self { inner: s }
    }

    /// Return the underlying string slice.
    pub fn as_str(&self) -> &str {
        self.inner
    }
}

impl PartialEq for StringId {
    fn eq(&self, other: &Self) -> bool {
        self.inner == other.inner
    }
}

impl Hash for StringId {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.inner.hash(state);
    }
}

impl fmt::Display for StringId {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(self.inner)
    }
}

impl fmt::Debug for StringId {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "StringId({:?})", self.inner)
    }
}

impl From<&'static str> for StringId {
    fn from(s: &'static str) -> Self {
        Self::new(s)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::collections::HashMap;

    #[test]
    fn equality() {
        let a = StringId::new("entity");
        let b = StringId::new("entity");
        let c = StringId::new("component");
        assert_eq!(a, b);
        assert_ne!(a, c);
    }

    #[test]
    fn display_and_debug() {
        let id = StringId::new("transform");
        assert_eq!(id.to_string(), "transform");
        let dbg = format!("{:?}", id);
        assert!(dbg.contains("transform"));
    }

    #[test]
    fn as_str() {
        let id = StringId::new("velocity");
        assert_eq!(id.as_str(), "velocity");
    }

    #[test]
    fn usable_as_hashmap_key() {
        let mut map: HashMap<StringId, i32> = HashMap::new();
        map.insert(StringId::new("health"), 100);
        map.insert(StringId::new("mana"), 50);
        assert_eq!(map[&StringId::new("health")], 100);
        assert_eq!(map[&StringId::new("mana")], 50);
    }

    #[test]
    fn from_static_str() {
        let id: StringId = "damage".into();
        assert_eq!(id.as_str(), "damage");
    }

    #[test]
    fn copy_semantics() {
        let a = StringId::new("speed");
        let b = a; // copy
        assert_eq!(a, b);
    }
}
