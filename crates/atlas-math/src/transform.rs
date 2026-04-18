//! 3-D transform (position, rotation, scale).

use crate::{Mat4, Quat, Vec3};
use serde::{Deserialize, Serialize};

/// A spatial transform consisting of translation, rotation, and uniform scale.
#[derive(Clone, Copy, Debug, PartialEq, Serialize, Deserialize)]
pub struct Transform {
    pub position: Vec3,
    pub rotation: Quat,
    pub scale:    Vec3,
}

impl Transform {
    /// Identity transform.
    pub const IDENTITY: Self = Self {
        position: Vec3::ZERO,
        rotation: Quat::IDENTITY,
        scale:    Vec3::ONE,
    };

    pub fn new(position: Vec3, rotation: Quat, scale: Vec3) -> Self {
        Self { position, rotation, scale }
    }

    /// Construct from a position only (identity rotation, unit scale).
    pub fn from_position(position: Vec3) -> Self {
        Self { position, ..Self::IDENTITY }
    }

    /// Compute the local-to-world matrix.
    pub fn to_matrix(&self) -> Mat4 {
        Mat4::from_scale_rotation_translation(self.scale, self.rotation, self.position)
    }

    /// Compose two transforms (self applied after `parent`).
    pub fn compose(&self, parent: &Transform) -> Transform {
        Transform {
            position: parent.position + parent.rotation * (parent.scale * self.position),
            rotation: parent.rotation * self.rotation,
            scale:    parent.scale * self.scale,
        }
    }
}

impl Default for Transform {
    fn default() -> Self {
        Self::IDENTITY
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{Quat, Vec3};

    #[test]
    fn identity_position_is_zero() {
        let t = Transform::IDENTITY;
        assert_eq!(t.position, Vec3::ZERO);
        assert_eq!(t.scale, Vec3::ONE);
    }

    #[test]
    fn from_position() {
        let t = Transform::from_position(Vec3::new(1.0, 2.0, 3.0));
        assert_eq!(t.position, Vec3::new(1.0, 2.0, 3.0));
        assert_eq!(t.rotation, Quat::IDENTITY);
        assert_eq!(t.scale, Vec3::ONE);
    }

    #[test]
    fn to_matrix_identity() {
        let m = Transform::IDENTITY.to_matrix();
        // identity matrix: diagonal is 1, translation is 0
        assert!((m.col(3).x).abs() < 1e-5);
        assert!((m.col(3).y).abs() < 1e-5);
        assert!((m.col(3).z).abs() < 1e-5);
    }

    #[test]
    fn compose_with_identity_is_self() {
        let t = Transform::from_position(Vec3::new(5.0, 0.0, 0.0));
        let composed = t.compose(&Transform::IDENTITY);
        assert!((composed.position.x - 5.0).abs() < 1e-5);
    }

    #[test]
    fn compose_translations_add() {
        let parent = Transform::from_position(Vec3::new(10.0, 0.0, 0.0));
        let child  = Transform::from_position(Vec3::new(5.0, 0.0, 0.0));
        let world  = child.compose(&parent);
        assert!((world.position.x - 15.0).abs() < 1e-4);
    }

    #[test]
    fn default_is_identity() {
        assert_eq!(Transform::default(), Transform::IDENTITY);
    }
}
