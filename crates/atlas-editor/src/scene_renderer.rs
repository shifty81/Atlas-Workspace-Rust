//! [`SceneRenderer`] — renders the ECS world into an offscreen texture (M6).
//!
//! The scene renderer iterates ECS entities that have a `Transform` component,
//! collects their mesh references, and submits instanced draw calls into a
//! `VkFramebuffer` sized to the viewport panel.  The resulting colour
//! attachment is then registered as an egui texture and displayed in the
//! Viewport panel.
//!
//! In M5 this is a stub that just tracks the viewport size.  Real GPU
//! draw calls land in M6.

use atlas_ecs::World;
use atlas_renderer::{Camera, Viewport};

/// Camera orbit state for the scene viewport.
pub struct OrbitCamera {
    pub camera:    Camera,
    pub yaw_deg:   f32,
    pub pitch_deg: f32,
    pub distance:  f32,
    pub target:    [f32; 3],
}

impl Default for OrbitCamera {
    fn default() -> Self {
        Self {
            camera:    Camera::perspective(60.0, 0.1, 10_000.0),
            yaw_deg:   45.0,
            pitch_deg: 30.0,
            distance:  10.0,
            target:    [0.0, 0.0, 0.0],
        }
    }
}

impl OrbitCamera {
    /// Apply a drag delta (alt+drag) to orbit the camera.
    pub fn orbit(&mut self, delta_x: f32, delta_y: f32) {
        self.yaw_deg   += delta_x * 0.5;
        self.pitch_deg  = (self.pitch_deg - delta_y * 0.5).clamp(-89.0, 89.0);
        self.update_camera_position();
    }

    /// Zoom by scrolling.
    pub fn zoom(&mut self, delta: f32) {
        self.distance = (self.distance - delta * 0.5).max(0.1);
        self.update_camera_position();
    }

    /// Pan (middle-drag).
    pub fn pan(&mut self, delta_x: f32, delta_y: f32) {
        let scale = self.distance * 0.001;
        self.target[0] -= delta_x * scale;
        self.target[1] += delta_y * scale;
        self.update_camera_position();
    }

    fn update_camera_position(&mut self) {
        use atlas_math::{Vec3, Quat};
        let yaw   = self.yaw_deg.to_radians();
        let pitch = self.pitch_deg.to_radians();
        let x = self.distance * pitch.cos() * yaw.sin();
        let y = self.distance * pitch.sin();
        let z = self.distance * pitch.cos() * yaw.cos();
        self.camera.position = Vec3::new(
            self.target[0] + x,
            self.target[1] + y,
            self.target[2] + z,
        );
        // Look toward target
        let dir = Vec3::new(-x, -y, -z).normalize_or_zero();
        let right = dir.cross(Vec3::Y).normalize_or_zero();
        let up    = right.cross(dir);
        // Build rotation from axes
        let mat = atlas_math::Mat4::from_cols(
            right.extend(0.0),
            up.extend(0.0),
            (-dir).extend(0.0),
            atlas_math::Vec4::W,
        );
        self.camera.rotation = Quat::from_mat4(&mat);
    }
}

/// Identifier for an egui-managed scene texture (M6 integration).
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct SceneTextureId(pub u64);

/// Manages the offscreen render target for the scene viewport.
pub struct SceneRenderer {
    pub orbit:        OrbitCamera,
    pub viewport:     Viewport,
    /// Egui texture ID of the last rendered frame (None until M6 GPU path).
    pub texture_id:   Option<egui::TextureId>,
    /// Width × height of the last render.
    last_size:        (u32, u32),
}

impl SceneRenderer {
    pub fn new() -> Self {
        Self {
            orbit:      OrbitCamera::default(),
            viewport:   Viewport::default(),
            texture_id: None,
            last_size:  (0, 0),
        }
    }

    /// Called when the viewport panel resizes.
    pub fn resize(&mut self, w: u32, h: u32) {
        if w == 0 || h == 0 { return; }
        self.last_size = (w, h);
        self.viewport  = Viewport::new(w as f32, h as f32);
    }

    /// Render the scene.  In M5 this is a no-op; in M6 it submits GPU work.
    pub fn render(&mut self, _world: &World) -> Option<egui::TextureId> {
        // M6 TODO: allocate / resize offscreen VkFramebuffer, submit draw calls,
        // register result with egui via egui::Context::load_texture.
        self.texture_id
    }

    pub fn size(&self) -> (u32, u32) { self.last_size }
}

impl Default for SceneRenderer {
    fn default() -> Self { Self::new() }
}
