//! Camera and viewport state.

use atlas_math::{Mat4, Vec3, Quat};

/// Projection mode.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ProjectionMode {
    Perspective,
    Orthographic,
}

/// Camera properties.
#[derive(Clone, Debug)]
pub struct Camera {
    pub position:   Vec3,
    pub rotation:   Quat,
    pub fov_y_deg:  f32,
    pub near:       f32,
    pub far:        f32,
    pub projection: ProjectionMode,
}

impl Camera {
    pub fn perspective(fov_y_deg: f32, near: f32, far: f32) -> Self {
        Self {
            position:   Vec3::ZERO,
            rotation:   Quat::IDENTITY,
            fov_y_deg,
            near,
            far,
            projection: ProjectionMode::Perspective,
        }
    }

    /// View matrix (world → camera).
    pub fn view_matrix(&self) -> Mat4 {
        let forward = self.rotation * Vec3::NEG_Z;
        let up      = self.rotation * Vec3::Y;
        Mat4::look_to_rh(self.position, forward, up)
    }

    /// Perspective projection matrix.
    pub fn projection_matrix(&self, aspect: f32) -> Mat4 {
        match self.projection {
            ProjectionMode::Perspective => {
                Mat4::perspective_rh(
                    self.fov_y_deg.to_radians(),
                    aspect,
                    self.near,
                    self.far,
                )
            }
            ProjectionMode::Orthographic => {
                let half_h = self.fov_y_deg * 0.5;
                let half_w = half_h * aspect;
                Mat4::orthographic_rh(-half_w, half_w, -half_h, half_h, self.near, self.far)
            }
        }
    }
}

impl Default for Camera {
    fn default() -> Self {
        Self::perspective(60.0, 0.1, 10_000.0)
    }
}

/// Viewport dimensions and scissor rect.
#[derive(Clone, Copy, Debug)]
pub struct Viewport {
    pub x:      f32,
    pub y:      f32,
    pub width:  f32,
    pub height: f32,
    pub min_depth: f32,
    pub max_depth: f32,
}

impl Viewport {
    pub fn new(width: f32, height: f32) -> Self {
        Self { x: 0.0, y: 0.0, width, height, min_depth: 0.0, max_depth: 1.0 }
    }

    pub fn aspect_ratio(&self) -> f32 {
        if self.height == 0.0 { 1.0 } else { self.width / self.height }
    }
}

impl Default for Viewport {
    fn default() -> Self {
        Self::new(1280.0, 720.0)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn view_matrix_identity_at_origin() {
        let cam = Camera::default();
        let view = cam.view_matrix();
        // Camera at origin looking down -Z: translation column should be zero.
        assert!(view.w_axis.x.abs() < 1e-4);
        assert!(view.w_axis.y.abs() < 1e-4);
        assert!(view.w_axis.z.abs() < 1e-4);
    }

    #[test]
    fn viewport_aspect() {
        let vp = Viewport::new(1920.0, 1080.0);
        assert!((vp.aspect_ratio() - 16.0 / 9.0).abs() < 1e-4);
    }
}
