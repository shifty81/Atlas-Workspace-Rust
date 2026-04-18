//! Properties panel — shows components of the selected entity (M7).

use atlas_ecs::{Name, World};
use atlas_math::Transform;

use crate::selection::SelectionState;

/// Events returned by the Properties panel for the EditorApp to handle.
pub enum PropertiesEvent {
    /// User changed the entity's name.
    Rename { name: String },
}

pub struct PropertiesPanel {
    pub open:        bool,
    /// Buffer for the in-progress name edit.
    name_edit:       String,
}

impl PropertiesPanel {
    pub fn new() -> Self { Self { open: true, name_edit: String::new() } }

    pub fn show(
        &mut self,
        ctx: &egui::Context,
        world: &mut World,
        sel: &SelectionState,
    ) -> Vec<PropertiesEvent> {
        let mut events = Vec::new();

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
                        // ── Name ──────────────────────────────────────────────
                        let current_name = world.components
                            .get::<Name>(entity)
                            .map(|n| n.0.clone())
                            .unwrap_or_else(|| format!("Entity #{entity}"));

                        // Sync edit buffer when selection changes
                        if self.name_edit.is_empty() || !self.name_edit.starts_with(&current_name[..current_name.len().min(3)]) {
                            self.name_edit = current_name.clone();
                        }

                        ui.horizontal(|ui| {
                            ui.strong("Name");
                            let response = ui.text_edit_singleline(&mut self.name_edit);
                            if response.lost_focus()
                                && ui.input(|i| i.key_pressed(egui::Key::Enter))
                                && self.name_edit != current_name
                            {
                                events.push(PropertiesEvent::Rename { name: self.name_edit.clone() });
                            }
                        });

                        ui.label(
                            egui::RichText::new(format!("ID: #{entity}"))
                                .small()
                                .color(egui::Color32::GRAY),
                        );
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
                                    if ui.small_button("Add Transform").clicked() {
                                        // Returned as an event would require more plumbing;
                                        // perform directly since we have &mut World.
                                        world.components.add(entity, Transform::default());
                                    }
                                }
                            });
                    }
                }
            });

        events
    }
}

impl Default for PropertiesPanel { fn default() -> Self { Self::new() } }
