//! Scene Viewport panel (M5 / M6).

use atlas_ecs::World;

use crate::{scene_renderer::SceneRenderer, selection::SelectionState};

pub struct ViewportPanel {
    pub open:      bool,
    /// Drag state for orbit camera.
    drag_active:   bool,
    last_mouse:    (f32, f32),
    alt_held:      bool,
    middle_held:   bool,
}

impl ViewportPanel {
    pub fn new() -> Self {
        Self { open: true, drag_active: false, last_mouse: (0.0, 0.0), alt_held: false, middle_held: false }
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

            match renderer.render(world) {
                Some(tex_id) => {
                    // Display scene texture
                    ui.image(egui::load::SizedTexture::new(tex_id, available));
                }
                None => {
                    // M5 placeholder
                    let (rect, response) = ui.allocate_exact_size(
                        available,
                        egui::Sense::click_and_drag(),
                    );
                    ui.painter().rect_filled(rect, 0.0, egui::Color32::from_rgb(30, 30, 40));
                    ui.painter().text(
                        rect.center(),
                        egui::Align2::CENTER_CENTER,
                        "Scene Viewport\n(GPU rendering initialises in M6)",
                        egui::FontId::proportional(14.0),
                        egui::Color32::GRAY,
                    );

                    // Orbit controls (M6 wires these to the GPU camera)
                    if response.dragged() {
                        let delta = response.drag_delta();
                        renderer.orbit.orbit(delta.x, delta.y);
                    }
                    if let Some(scroll) = ui.input(|i| {
                        i.events.iter().find_map(|e| {
                            if let egui::Event::Scroll(s) = e { Some(s.y) } else { None }
                        })
                    }) {
                        renderer.orbit.zoom(scroll);
                    }
                }
            }
        });
    }
}

impl Default for ViewportPanel { fn default() -> Self { Self::new() } }
