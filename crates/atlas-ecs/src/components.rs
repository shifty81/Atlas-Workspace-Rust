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

// ── PhysicsBody ──────────────────────────────────────────────────────────────

/// Links an ECS entity to a body in a [`PhysicsWorld`].
///
/// Add this component alongside a [`atlas_math::Transform`] to have the
/// physics simulation drive the entity's position each tick.
///
/// The owning code (typically a `PhysicsSystem`) is responsible for keeping
/// both in sync:
/// 1. **Before `step`** — copy `Transform.position` into the `PhysicsWorld` body.
/// 2. **After `step`** — copy the body's simulated position back into `Transform`.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct PhysicsBody {
    /// Opaque ID of the body inside the `PhysicsWorld`.
    pub body_id:   u32,
    /// Whether this body is a static (immovable) collider.
    pub is_static: bool,
    /// Mass in kg.  Static bodies may have mass 0.
    pub mass:      f32,
}

impl PhysicsBody {
    pub fn new(body_id: u32, mass: f32, is_static: bool) -> Self {
        Self { body_id, is_static, mass }
    }

    pub fn dynamic(body_id: u32, mass: f32) -> Self {
        Self::new(body_id, mass, false)
    }

    pub fn r#static(body_id: u32) -> Self {
        Self::new(body_id, 0.0, true)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn name_display() {
        let n = Name::new("Cube");
        assert_eq!(n.to_string(), "Cube");
        assert_eq!(n.as_str(), "Cube");
    }

    #[test]
    fn name_from_str() {
        let n = Name::from("Sphere");
        assert_eq!(n.0, "Sphere");
    }

    #[test]
    fn physics_body_constructors() {
        let d = PhysicsBody::dynamic(1, 2.5);
        assert!(!d.is_static);
        assert!((d.mass - 2.5).abs() < f32::EPSILON);

        let s = PhysicsBody::r#static(2);
        assert!(s.is_static);
        assert_eq!(s.body_id, 2);
    }
}
