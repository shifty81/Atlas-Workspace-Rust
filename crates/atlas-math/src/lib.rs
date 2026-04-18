//! # Atlas Math
//!
//! Linear algebra and geometric primitives for the Atlas Workspace,
//! built on top of [`glam`].
//!
//! Re-exports the most common types so downstream crates only need to
//! depend on `atlas-math`.

pub use glam::{
    BVec2, BVec3, BVec4,
    DVec2, DVec3, DVec4,
    IVec2, IVec3, IVec4,
    Mat2, Mat3, Mat4,
    Quat,
    UVec2, UVec3, UVec4,
    Vec2, Vec3, Vec3A, Vec4,
};

pub mod aabb;
pub mod color;
pub mod ray;
pub mod transform;

pub use aabb::Aabb;
pub use color::Color;
pub use ray::Ray;
pub use transform::Transform;

/// Convert degrees to radians.
#[inline]
pub fn deg_to_rad(deg: f32) -> f32 {
    deg * std::f32::consts::PI / 180.0
}

/// Convert radians to degrees.
#[inline]
pub fn rad_to_deg(rad: f32) -> f32 {
    rad * 180.0 / std::f32::consts::PI
}

/// Linear interpolation between two values.
#[inline]
pub fn lerp(a: f32, b: f32, t: f32) -> f32 {
    a + (b - a) * t
}

/// Clamp a value to [min, max].
#[inline]
pub fn clamp(v: f32, min: f32, max: f32) -> f32 {
    v.max(min).min(max)
}
