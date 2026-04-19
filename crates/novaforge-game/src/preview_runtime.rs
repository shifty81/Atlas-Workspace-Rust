// SPDX-License-Identifier: GPL-3.0-only
// NovaForge preview runtime — port of NovaForge::NovaForgePreviewRuntime.
// Copyright (C) NovaForge contributors. GNU General Public License v3.0.

use super::preview_world::{
    EntityId, INVALID_ENTITY_ID, NovaForgePreviewWorld, PreviewTransform, PreviewVec3,
};

// ── FlyCameraState ────────────────────────────────────────────────────────

const CAM_DEFAULT_Y: f32 = 2.0;
const CAM_DEFAULT_Z: f32 = 10.0;
const CAM_DEFAULT_YAW: f32 = -90.0;
const CAM_DEFAULT_SPEED: f32 = 5.0;
const CAM_DEFAULT_SENSITIVITY: f32 = 0.1;

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct FlyCameraState {
    pub x:           f32,
    pub y:           f32,
    pub z:           f32,
    pub yaw:         f32,
    pub pitch:       f32,
    pub speed:       f32,
    pub sensitivity: f32,
}

impl Default for FlyCameraState {
    fn default() -> Self {
        Self {
            x: 0.0,
            y: CAM_DEFAULT_Y,
            z: CAM_DEFAULT_Z,
            yaw: CAM_DEFAULT_YAW,
            pitch: 0.0,
            speed: CAM_DEFAULT_SPEED,
            sensitivity: CAM_DEFAULT_SENSITIVITY,
        }
    }
}

// ── CameraInput ───────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Default)]
pub struct CameraInput {
    pub move_forward:     bool,
    pub move_back:        bool,
    pub move_left:        bool,
    pub move_right:       bool,
    pub move_up:          bool,
    pub move_down:        bool,
    pub mouse_delta_x:    f32,
    pub mouse_delta_y:    f32,
    pub mouse_button_held: bool,
}

// ── GizmoMode ─────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum GizmoMode {
    #[default]
    Translate,
    Rotate,
    Scale,
}

// ── GizmoState ────────────────────────────────────────────────────────────

#[derive(Debug, Clone)]
pub struct GizmoState {
    pub visible:   bool,
    pub mode:      GizmoMode,
    pub entity_id: EntityId,
    pub position:  PreviewVec3,
}

// ── NovaForgePreviewRuntime ───────────────────────────────────────────────

pub struct NovaForgePreviewRuntime {
    world:          NovaForgePreviewWorld,
    camera:         FlyCameraState,
    gizmo_mode:     GizmoMode,
    running:        bool,
    elapsed:        f32,
    sky_color:      u32,
    preview_dirty:  bool,
}

impl NovaForgePreviewRuntime {
    pub fn new() -> Self {
        Self {
            world:         NovaForgePreviewWorld::new(),
            camera:        FlyCameraState::default(),
            gizmo_mode:    GizmoMode::default(),
            running:       false,
            elapsed:       0.0,
            sky_color:     0x1A1A2EFF,
            preview_dirty: false,
        }
    }

    // ── World accessors ───────────────────────────────────────────────────

    pub fn world(&self) -> &NovaForgePreviewWorld { &self.world }
    pub fn world_mut(&mut self) -> &mut NovaForgePreviewWorld { &mut self.world }

    // ── Simulation ────────────────────────────────────────────────────────

    pub fn tick(&mut self, dt: f32) {
        if self.running {
            self.elapsed += dt;
        }
    }

    // ── Camera ────────────────────────────────────────────────────────────

    pub fn process_camera_input(&mut self, input: &CameraInput, dt: f32) {
        if input.mouse_button_held {
            self.camera.yaw   += input.mouse_delta_x * self.camera.sensitivity;
            self.camera.pitch -= input.mouse_delta_y * self.camera.sensitivity;
            self.camera.pitch  = self.camera.pitch.clamp(-89.0, 89.0);
        }

        let yaw_rad   = self.camera.yaw.to_radians();
        let pitch_rad = self.camera.pitch.to_radians();

        // Front vector (same trig as C++)
        let fx = pitch_rad.cos() * yaw_rad.cos();
        let fy = pitch_rad.sin();
        let fz = pitch_rad.cos() * yaw_rad.sin();

        // Right vector (cross(front, world_up), normalised)
        let world_up = (0.0_f32, 1.0_f32, 0.0_f32);
        let rx = fy * world_up.2 - fz * world_up.1;
        let ry = fz * world_up.0 - fx * world_up.2;
        let rz = fx * world_up.1 - fy * world_up.0;
        let r_len = (rx * rx + ry * ry + rz * rz).sqrt().max(1e-8);
        let (rx, ry, rz) = (rx / r_len, ry / r_len, rz / r_len);

        let v = self.camera.speed * dt;

        if input.move_forward  { self.camera.x += fx * v; self.camera.y += fy * v; self.camera.z += fz * v; }
        if input.move_back     { self.camera.x -= fx * v; self.camera.y -= fy * v; self.camera.z -= fz * v; }
        if input.move_right    { self.camera.x += rx * v; self.camera.y += ry * v; self.camera.z += rz * v; }
        if input.move_left     { self.camera.x -= rx * v; self.camera.y -= ry * v; self.camera.z -= rz * v; }
        if input.move_up       { self.camera.y += v; }
        if input.move_down     { self.camera.y -= v; }
    }

    pub fn camera_state(&self) -> &FlyCameraState { &self.camera }
    pub fn set_camera_state(&mut self, s: FlyCameraState) { self.camera = s; }
    pub fn set_camera_speed(&mut self, s: f32) { self.camera.speed = s; }
    pub fn set_camera_sensitivity(&mut self, s: f32) { self.camera.sensitivity = s; }

    // ── Gizmo ─────────────────────────────────────────────────────────────

    pub fn gizmo_state(&self) -> GizmoState {
        if let Some(e) = self.world.selected_entity() {
            GizmoState {
                visible:   true,
                mode:      self.gizmo_mode,
                entity_id: e.id,
                position:  e.transform.position,
            }
        } else {
            GizmoState {
                visible:   false,
                mode:      self.gizmo_mode,
                entity_id: INVALID_ENTITY_ID,
                position:  PreviewVec3::default(),
            }
        }
    }

    pub fn set_gizmo_mode(&mut self, m: GizmoMode) { self.gizmo_mode = m; }
    pub fn gizmo_mode(&self) -> GizmoMode { self.gizmo_mode }

    // ── Lifecycle ─────────────────────────────────────────────────────────

    pub fn start(&mut self) { self.running = true; }
    pub fn stop(&mut self)  { self.running = false; }
    pub fn is_running(&self) -> bool { self.running }
    pub fn elapsed_seconds(&self) -> f32 { self.elapsed }

    // ── Sky ───────────────────────────────────────────────────────────────

    pub fn sky_color(&self) -> u32 { self.sky_color }
    pub fn set_sky_color(&mut self, c: u32) { self.sky_color = c; }

    // ── Document binding (stub) ───────────────────────────────────────────

    pub fn bind_world_document(&mut self, _world_id: &str) {
        self.preview_dirty = true;
    }

    pub fn bind_level_document(&mut self, _level_id: &str) {
        self.preview_dirty = true;
    }

    pub fn rebuild_from_document(&mut self) {
        self.preview_dirty = false;
    }

    // ── Entity changes ────────────────────────────────────────────────────

    pub fn apply_entity_change(
        &mut self,
        id: EntityId,
        position: PreviewVec3,
        rotation: PreviewVec3,
        scale: PreviewVec3,
    ) {
        let t = PreviewTransform { position, rotation, scale };
        self.world.set_transform(id, t);
    }

    pub fn apply_selection(&mut self, id: EntityId) {
        self.world.select_entity(id);
    }

    // ── Properties ───────────────────────────────────────────────────────

    pub fn selected_entity_properties(&self) -> Vec<(String, String)> {
        let Some(e) = self.world.selected_entity() else { return Vec::new(); };
        vec![
            ("name".into(),       e.name.clone()),
            ("position.x".into(), format!("{:.3}", e.transform.position.x)),
            ("position.y".into(), format!("{:.3}", e.transform.position.y)),
            ("position.z".into(), format!("{:.3}", e.transform.position.z)),
            ("rotation.x".into(), format!("{:.3}", e.transform.rotation.x)),
            ("rotation.y".into(), format!("{:.3}", e.transform.rotation.y)),
            ("rotation.z".into(), format!("{:.3}", e.transform.rotation.z)),
            ("scale.x".into(),    format!("{:.3}", e.transform.scale.x)),
            ("scale.y".into(),    format!("{:.3}", e.transform.scale.y)),
            ("scale.z".into(),    format!("{:.3}", e.transform.scale.z)),
            ("mesh".into(),       e.mesh_tag.clone()),
            ("material".into(),   e.material_tag.clone()),
            ("visible".into(),    e.visible.to_string()),
        ]
    }

    // ── Hierarchy ─────────────────────────────────────────────────────────

    pub fn hierarchy_order(&self) -> Vec<EntityId> {
        let entities = self.world.entities();
        let mut result = Vec::with_capacity(entities.len());
        let mut queue: std::collections::VecDeque<EntityId> = std::collections::VecDeque::new();

        // Enqueue roots first
        for e in entities {
            if e.parent_id == INVALID_ENTITY_ID {
                queue.push_back(e.id);
            }
        }

        // BFS
        while let Some(id) = queue.pop_front() {
            result.push(id);
            for e in entities {
                if e.parent_id == id {
                    queue.push_back(e.id);
                }
            }
        }

        result
    }
}

impl Default for NovaForgePreviewRuntime {
    fn default() -> Self { Self::new() }
}

// ── Tests ─────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use crate::preview_world::INVALID_ENTITY_ID;

    #[test]
    fn new_runtime_defaults() {
        let rt = NovaForgePreviewRuntime::new();
        assert!(!rt.is_running());
        assert_eq!(rt.elapsed_seconds(), 0.0);
        assert_eq!(rt.sky_color(), 0x1A1A2EFF);
        assert_eq!(rt.gizmo_mode(), GizmoMode::Translate);
    }

    #[test]
    fn tick_advances_elapsed_when_running() {
        let mut rt = NovaForgePreviewRuntime::new();
        rt.start();
        rt.tick(1.5);
        assert!((rt.elapsed_seconds() - 1.5).abs() < 1e-6);
    }

    #[test]
    fn tick_does_not_advance_when_stopped() {
        let mut rt = NovaForgePreviewRuntime::new();
        rt.tick(1.5);
        assert_eq!(rt.elapsed_seconds(), 0.0);
    }

    #[test]
    fn camera_input_forward_moves_camera() {
        let mut rt = NovaForgePreviewRuntime::new();
        let initial_z = rt.camera_state().z;
        let input = CameraInput { move_forward: true, mouse_button_held: false, ..Default::default() };
        rt.process_camera_input(&input, 1.0);
        // At yaw=-90, pitch=0: forward is (0, 0, -1), so z decreases
        assert!(rt.camera_state().z < initial_z);
    }

    #[test]
    fn camera_input_mouse_look_changes_yaw_pitch() {
        let mut rt = NovaForgePreviewRuntime::new();
        let input = CameraInput {
            mouse_button_held: true,
            mouse_delta_x: 10.0,
            mouse_delta_y: 5.0,
            ..Default::default()
        };
        rt.process_camera_input(&input, 0.016);
        let cam = rt.camera_state();
        // yaw should increase, pitch should decrease (delta_y is subtracted)
        assert!(cam.yaw > -90.0);
        assert!(cam.pitch < 0.0);
    }

    #[test]
    fn pitch_clamped_to_89() {
        let mut rt = NovaForgePreviewRuntime::new();
        let input = CameraInput {
            mouse_button_held: true,
            mouse_delta_y: -10000.0,
            ..Default::default()
        };
        rt.process_camera_input(&input, 1.0);
        assert!(rt.camera_state().pitch <= 89.0);
        assert!(rt.camera_state().pitch >= -89.0);
    }

    #[test]
    fn gizmo_state_with_selection() {
        let mut rt = NovaForgePreviewRuntime::new();
        let id = rt.world_mut().create_entity("sel", INVALID_ENTITY_ID);
        rt.apply_selection(id);
        let gs = rt.gizmo_state();
        assert!(gs.visible);
        assert_eq!(gs.entity_id, id);
    }

    #[test]
    fn gizmo_state_no_selection() {
        let rt = NovaForgePreviewRuntime::new();
        let gs = rt.gizmo_state();
        assert!(!gs.visible);
        assert_eq!(gs.entity_id, INVALID_ENTITY_ID);
    }

    #[test]
    fn hierarchy_order_bfs() {
        let mut rt = NovaForgePreviewRuntime::new();
        let root = rt.world_mut().create_entity("root", INVALID_ENTITY_ID);
        let child = rt.world_mut().create_entity("child", root);
        let order = rt.hierarchy_order();
        assert_eq!(order[0], root);
        assert_eq!(order[1], child);
    }

    #[test]
    fn selected_entity_properties() {
        let mut rt = NovaForgePreviewRuntime::new();
        let id = rt.world_mut().create_entity("hero", INVALID_ENTITY_ID);
        rt.apply_selection(id);
        let props = rt.selected_entity_properties();
        assert!(!props.is_empty());
        let name_pair = props.iter().find(|(k, _)| k == "name");
        assert_eq!(name_pair.unwrap().1, "hero");
    }
}
