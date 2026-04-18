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
