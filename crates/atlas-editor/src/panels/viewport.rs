//! Scene Viewport panel (M6).

use atlas_ecs::World;

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
    /// a placeholder is shown.
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
                    // Display scene texture
                    ui.image(egui::load::SizedTexture::new(tex_id, available));
                }
                None => {
                    // Placeholder — click-and-drag controls the orbit camera
                    let (rect, response) = ui.allocate_exact_size(
                        available,
                        egui::Sense::click_and_drag(),
                    );
                    ui.painter().rect_filled(rect, 0.0, egui::Color32::from_rgb(30, 30, 40));
                    ui.painter().text(
                        rect.center(),
                        egui::Align2::CENTER_CENTER,
                        "Scene Viewport\n(GPU rendering initialises in M7)",
                        egui::FontId::proportional(14.0),
                        egui::Color32::GRAY,
                    );

                    // Orbit (left-drag), pan (middle-drag), or alt+drag = orbit
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
                            // left-drag or alt+left-drag both orbit
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

                    // Show camera info
                    let cam = &renderer.orbit;
                    ui.painter().text(
                        egui::Pos2::new(rect.left() + 8.0, rect.bottom() - 24.0),
                        egui::Align2::LEFT_BOTTOM,
                        format!(
                            "Yaw: {:.1}°  Pitch: {:.1}°  Dist: {:.1}",
                            cam.yaw_deg, cam.pitch_deg, cam.distance
                        ),
                        egui::FontId::monospace(11.0),
                        egui::Color32::from_rgb(180, 180, 180),
                    );
                }
            }
        });
    }
}

impl Default for ViewportPanel { fn default() -> Self { Self::new() } }
