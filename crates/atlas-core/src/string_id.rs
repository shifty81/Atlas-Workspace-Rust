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
