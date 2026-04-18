//! Properties panel — shows components of the selected entity (M6).

use atlas_ecs::World;
use atlas_math::Transform;

use crate::selection::SelectionState;

pub struct PropertiesPanel {
    pub open: bool,
}

impl PropertiesPanel {
    pub fn new() -> Self { Self { open: true } }

    pub fn show(&mut self, ctx: &egui::Context, world: &mut World, sel: &SelectionState) {
        egui::SidePanel::right("properties")
            .resizable(true)
            .default_width(280.0)
            .show(ctx, |ui| {
                ui.heading("Properties");
                ui.separator();

                match sel.primary() {
                    None => {
                        ui.label(egui::RichText::new("Nothing selected").weak());
                    }
                    Some(entity) => {
                        ui.label(format!("Entity #{entity}"));
                        ui.separator();

                        // ── Transform ────────────────────────────────────────
                        egui::CollapsingHeader::new("Transform")
                            .default_open(true)
                            .show(ui, |ui| {
                                if let Some(t) = world.components.get_mut::<Transform>(entity) {
                                    egui::Grid::new("transform_grid")
                                        .num_columns(4)
                                        .spacing([4.0, 4.0])
                                        .show(ui, |ui| {
                                            // Position
                                            ui.strong("Position");
                                            ui.add(egui::DragValue::new(&mut t.position.x).speed(0.1).prefix("X "));
                                            ui.add(egui::DragValue::new(&mut t.position.y).speed(0.1).prefix("Y "));
                                            ui.add(egui::DragValue::new(&mut t.position.z).speed(0.1).prefix("Z "));
                                            ui.end_row();

                                            // Scale
                                            ui.strong("Scale");
                                            ui.add(egui::DragValue::new(&mut t.scale.x).speed(0.01).prefix("X "));
                                            ui.add(egui::DragValue::new(&mut t.scale.y).speed(0.01).prefix("Y "));
                                            ui.add(egui::DragValue::new(&mut t.scale.z).speed(0.01).prefix("Z "));
                                            ui.end_row();
                                        });
                                } else {
                                    ui.label(egui::RichText::new("(no Transform)").weak());
                                }
                            });
                    }
                }
            });
    }
}

impl Default for PropertiesPanel { fn default() -> Self { Self::new() } }
