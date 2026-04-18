//! Scene Viewport panel (M7).

use atlas_ecs::{Name, World};

use crate::{scene_renderer::SceneRenderer, selection::SelectionState};

pub struct ViewportPanel {
    pub open:    bool,
    drag_active: bool,
    last_mouse:  egui::Pos2,
    alt_held:    bool,
    middle_held: bool,
}

impl ViewportPanel {
    pub fn new() -> Self {
        Self {
            open:        true,
            drag_active: false,
            last_mouse:  egui::Pos2::ZERO,
            alt_held:    false,
            middle_held: false,
        }
    }

    /// Draw the panel.  The scene texture (if available) is displayed; otherwise
    /// a placeholder with grid and entity billboards is shown.
    pub fn show(
        &mut self,
        ctx:      &egui::Context,
        renderer: &mut SceneRenderer,
        world:    &World,
        _sel:     &mut SelectionState,
    ) {
        egui::CentralPanel::default().show(ctx, |ui| {
            ui.heading("Viewport");

            let available = ui.available_size();
            let w = available.x.max(1.0) as u32;
            let h = available.y.max(1.0) as u32;

            if renderer.size() != (w, h) {
                renderer.resize(w, h);
            }

            // Track modifier keys
            self.alt_held    = ui.input(|i| i.modifiers.alt);
            self.middle_held = ui.input(|i| i.pointer.button_down(egui::PointerButton::Middle));

            match renderer.render(world) {
                Some(tex_id) => {
                    // Display GPU scene texture once available
                    ui.image(egui::load::SizedTexture::new(tex_id, available));
                }
                None => {
                    // Software-drawn viewport: background + grid + entity points
                    let (rect, response) = ui.allocate_exact_size(
                        available,
                        egui::Sense::click_and_drag(),
                    );

                    let painter = ui.painter_at(rect);

                    // Background
                    painter.rect_filled(rect, 0.0, egui::Color32::from_rgb(25, 25, 35));

                    // Ground grid
                    draw_grid(&painter, rect, &renderer.orbit);

                    // Entity billboards (one dot per entity with Transform)
                    draw_entity_billboards(&painter, rect, world, &renderer.orbit);

                    // ── Camera controls ──────────────────────────────────────

                    if response.drag_started() {
                        self.drag_active = true;
                        if let Some(pos) = response.interact_pointer_pos() {
                            self.last_mouse = pos;
                        }
                    }
                    if response.drag_stopped() {
                        self.drag_active = false;
                    }
                    if self.drag_active && response.dragged() {
                        let delta = response.drag_delta();
                        if self.middle_held {
                            renderer.orbit.pan(delta.x, delta.y);
                        } else {
                            renderer.orbit.orbit(delta.x, delta.y);
                        }
                        if let Some(pos) = response.interact_pointer_pos() {
                            self.last_mouse = pos;
                        }
                    }

                    // Zoom via scroll
                    let scroll = ui.input(|i| i.raw_scroll_delta.y);
                    if scroll.abs() > 0.0 {
                        renderer.orbit.zoom(scroll * 0.1);
                    }

                    // Overlay: camera info + entity count
                    let cam = &renderer.orbit;
                    painter.text(
                        egui::Pos2::new(rect.left() + 8.0, rect.bottom() - 24.0),
                        egui::Align2::LEFT_BOTTOM,
                        format!(
                            "Yaw: {:.1}°  Pitch: {:.1}°  Dist: {:.1}  Entities: {}",
                            cam.yaw_deg, cam.pitch_deg, cam.distance,
                            world.entities.count(),
                        ),
                        egui::FontId::monospace(11.0),
                        egui::Color32::from_rgb(180, 180, 180),
                    );

                    // Overlay: controls hint
                    painter.text(
                        egui::Pos2::new(rect.right() - 8.0, rect.bottom() - 24.0),
                        egui::Align2::RIGHT_BOTTOM,
                        "LMB Drag: orbit  MMB Drag: pan  Scroll: zoom",
                        egui::FontId::monospace(10.0),
                        egui::Color32::from_rgb(120, 120, 120),
                    );
                }
            }
        });
    }
}

// ── Grid drawing ─────────────────────────────────────────────────────────────

/// Project a 3-D world point into 2-D screen space using a simple isometric
/// projection.  Returns `None` if the point is behind the camera.
fn world_to_screen(
    wx: f32, wy: f32, wz: f32,
    rect: egui::Rect,
    orbit: &crate::scene_renderer::OrbitCamera,
) -> Option<egui::Pos2> {
    use atlas_math::Vec3;
    let cam_pos = orbit.camera.position;
    let target  = Vec3::from(orbit.target);

    // Camera axes
    let fwd   = (target - cam_pos).normalize_or_zero();
    let right = fwd.cross(Vec3::Y).normalize_or_zero();
    let up    = right.cross(fwd);

    let to_point = Vec3::new(wx, wy, wz) - cam_pos;
    let depth    = fwd.dot(to_point);
    if depth <= 0.01 { return None; }

    let fov_scale = rect.height() / (2.0 * (30.0_f32.to_radians()).tan() * depth);
    let sx = rect.center().x + right.dot(to_point) * fov_scale;
    let sy = rect.center().y - up.dot(to_point)    * fov_scale;

    if rect.contains(egui::Pos2::new(sx, sy)) {
        Some(egui::Pos2::new(sx, sy))
    } else {
        None
    }
}

/// Draw a flat XZ ground grid.
fn draw_grid(
    painter: &egui::Painter,
    rect:    egui::Rect,
    orbit:   &crate::scene_renderer::OrbitCamera,
) {
    let half   = 10;       // grid extends ±10 units
    let step   = 1.0_f32;
    let dim    = egui::Color32::from_rgba_unmultiplied(60, 60, 80, 200);
    let origin = egui::Color32::from_rgba_unmultiplied(100, 100, 120, 255);

    for i in -half..=half {
        let fi = i as f32 * step;
        let lim = half as f32 * step;

        let color = if i == 0 { origin } else { dim };

        // Line along X axis (constant Z = fi)
        if let (Some(a), Some(b)) = (
            world_to_screen(-lim, 0.0, fi, rect, orbit),
            world_to_screen( lim, 0.0, fi, rect, orbit),
        ) {
            painter.line_segment([a, b], egui::Stroke::new(if i == 0 { 1.5 } else { 0.5 }, color));
        }
        // Line along Z axis (constant X = fi)
        if let (Some(a), Some(b)) = (
            world_to_screen(fi, 0.0, -lim, rect, orbit),
            world_to_screen(fi, 0.0,  lim, rect, orbit),
        ) {
            painter.line_segment([a, b], egui::Stroke::new(if i == 0 { 1.5 } else { 0.5 }, color));
        }
    }
}

/// Draw a small circle at each entity's position.
fn draw_entity_billboards(
    painter: &egui::Painter,
    rect:    egui::Rect,
    world:   &World,
    orbit:   &crate::scene_renderer::OrbitCamera,
) {
    use atlas_math::Transform;

    let fill   = egui::Color32::from_rgb(255, 160, 50);
    let stroke = egui::Stroke::new(1.0, egui::Color32::WHITE);

    let all_transforms = world.components.get_all::<Transform>();
    for (&id, t) in &all_transforms {
        let Some(screen) = world_to_screen(
            t.position.x, t.position.y, t.position.z, rect, orbit,
        ) else { continue };

        painter.circle(screen, 5.0, fill, stroke);

        // Label
        let label = world.components
            .get::<Name>(id)
            .map(|n| n.0.clone())
            .unwrap_or_else(|| format!("#{id}"));
        painter.text(
            egui::Pos2::new(screen.x + 7.0, screen.y - 7.0),
            egui::Align2::LEFT_BOTTOM,
            &label,
            egui::FontId::proportional(11.0),
            egui::Color32::from_rgb(220, 220, 255),
        );
    }
}

impl Default for ViewportPanel { fn default() -> Self { Self::new() } }
