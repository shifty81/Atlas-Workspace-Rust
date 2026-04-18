//! Axis-Aligned Bounding Box.

use crate::Vec3;
use serde::{Deserialize, Serialize};

/// An axis-aligned bounding box defined by its minimum and maximum corners.
#[derive(Clone, Copy, Debug, PartialEq, Serialize, Deserialize)]
pub struct Aabb {
    pub min: Vec3,
    pub max: Vec3,
}

impl Aabb {
    /// Construct from explicit corners.
    pub fn new(min: Vec3, max: Vec3) -> Self {
        Self { min, max }
    }

    /// A degenerate AABB that contains nothing.
    pub fn empty() -> Self {
        Self {
            min: Vec3::splat(f32::INFINITY),
            max: Vec3::splat(f32::NEG_INFINITY),
        }
    }

    /// Expand to include a point.
    pub fn expand_point(&mut self, p: Vec3) {
        self.min = self.min.min(p);
        self.max = self.max.max(p);
    }

    /// Expand to include another AABB.
    pub fn expand_aabb(&mut self, other: &Aabb) {
        self.min = self.min.min(other.min);
        self.max = self.max.max(other.max);
    }

    /// Extents along each axis.
    pub fn size(&self) -> Vec3 {
        self.max - self.min
    }

    /// Centre point.
    pub fn center(&self) -> Vec3 {
        (self.min + self.max) * 0.5
    }

    /// True if the AABB contains the point (inclusive).
    pub fn contains(&self, p: Vec3) -> bool {
        p.cmpge(self.min).all() && p.cmple(self.max).all()
    }

    /// True if this AABB overlaps `other`.
    pub fn overlaps(&self, other: &Aabb) -> bool {
        self.min.cmple(other.max).all() && self.max.cmpge(other.min).all()
    }
}

impl Default for Aabb {
    fn default() -> Self {
        Self::empty()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::Vec3;

    #[test]
    fn empty_aabb_contains_nothing() {
        let a = Aabb::empty();
        assert!(!a.contains(Vec3::ZERO));
    }

    #[test]
    fn default_is_empty() {
        let a = Aabb::default();
        assert!(!a.contains(Vec3::ZERO));
    }

    #[test]
    fn new_and_contains() {
        let a = Aabb::new(Vec3::new(-1.0, -1.0, -1.0), Vec3::new(1.0, 1.0, 1.0));
        assert!(a.contains(Vec3::ZERO));
        assert!(a.contains(Vec3::new(1.0, 1.0, 1.0)));  // inclusive boundary
        assert!(!a.contains(Vec3::new(2.0, 0.0, 0.0)));
    }

    #[test]
    fn size_and_center() {
        let a = Aabb::new(Vec3::new(0.0, 0.0, 0.0), Vec3::new(4.0, 6.0, 8.0));
        assert_eq!(a.size(), Vec3::new(4.0, 6.0, 8.0));
        assert_eq!(a.center(), Vec3::new(2.0, 3.0, 4.0));
    }

    #[test]
    fn expand_point() {
        let mut a = Aabb::empty();
        a.expand_point(Vec3::new(1.0, 2.0, 3.0));
        a.expand_point(Vec3::new(-1.0, 0.0, 5.0));
        assert!(a.contains(Vec3::new(0.0, 1.0, 4.0)));
        assert!(!a.contains(Vec3::new(2.0, 0.0, 0.0)));
    }

    #[test]
    fn expand_aabb() {
        let mut a = Aabb::new(Vec3::ZERO, Vec3::ONE);
        let b = Aabb::new(Vec3::new(2.0, 2.0, 2.0), Vec3::new(3.0, 3.0, 3.0));
        a.expand_aabb(&b);
        assert!(a.contains(Vec3::new(2.5, 2.5, 2.5)));
        assert!(a.contains(Vec3::ZERO));
    }

    #[test]
    fn overlaps() {
        let a = Aabb::new(Vec3::ZERO, Vec3::ONE);
        let b = Aabb::new(Vec3::new(0.5, 0.5, 0.5), Vec3::new(1.5, 1.5, 1.5));
        let c = Aabb::new(Vec3::new(2.0, 2.0, 2.0), Vec3::new(3.0, 3.0, 3.0));
        assert!(a.overlaps(&b));
        assert!(!a.overlaps(&c));
    }
}
