//! World-space ray type.

use crate::Vec3;
use serde::{Deserialize, Serialize};

/// A ray defined by an origin point and a unit direction.
#[derive(Clone, Copy, Debug, PartialEq, Serialize, Deserialize)]
pub struct Ray {
    pub origin:    Vec3,
    pub direction: Vec3,
}

impl Ray {
    /// Construct a new ray, normalising the direction.
    pub fn new(origin: Vec3, direction: Vec3) -> Self {
        Self { origin, direction: direction.normalize() }
    }

    /// Point along the ray at parameter `t`.
    pub fn at(&self, t: f32) -> Vec3 {
        self.origin + self.direction * t
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::Vec3;

    #[test]
    fn at_t_zero_is_origin() {
        let r = Ray::new(Vec3::new(1.0, 2.0, 3.0), Vec3::new(0.0, 1.0, 0.0));
        assert_eq!(r.at(0.0), Vec3::new(1.0, 2.0, 3.0));
    }

    #[test]
    fn at_t_one_advances_by_direction() {
        let r = Ray::new(Vec3::ZERO, Vec3::new(0.0, 0.0, 1.0));
        let p = r.at(5.0);
        assert!((p.z - 5.0).abs() < 1e-6);
    }

    #[test]
    fn direction_is_normalised() {
        let r = Ray::new(Vec3::ZERO, Vec3::new(3.0, 0.0, 0.0));
        assert!((r.direction.length() - 1.0).abs() < 1e-6);
    }
}
