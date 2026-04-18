//! Built-in ECS components used across the editor and game runtime.

use serde::{Deserialize, Serialize};

/// A human-readable name for an entity.
///
/// Shown in the Outliner and Properties panels.  If absent, the editor
/// falls back to `"Entity #<id>"`.
#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub struct Name(pub String);

impl Name {
    pub fn new(s: impl Into<String>) -> Self { Self(s.into()) }
    pub fn as_str(&self) -> &str { &self.0 }
}

impl std::fmt::Display for Name {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(&self.0)
    }
}

impl From<&str> for Name {
    fn from(s: &str) -> Self { Self(s.to_string()) }
}

impl From<String> for Name {
    fn from(s: String) -> Self { Self(s) }
}
