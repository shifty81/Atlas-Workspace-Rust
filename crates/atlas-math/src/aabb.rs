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
