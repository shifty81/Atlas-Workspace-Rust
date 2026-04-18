//! Properties panel — shows components of the selected entity (M5).

use atlas_ecs::{World};

use crate::selection::SelectionState;

pub struct PropertiesPanel {
    pub open: bool,
}

impl PropertiesPanel {
    pub fn new() -> Self { Self { open: true } }

    pub fn show(&mut self, ctx: &egui::Context, world: &World, sel: &SelectionState) {
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

                        // Show component list (M5: name the known component stores)
                        let cstore = &world.components;
                        egui::CollapsingHeader::new("Transform")
                            .default_open(true)
                            .show(ui, |ui| {
                                if let Some(_t) = cstore.get::<atlas_math::Mat4>(entity) {
                                    ui.label("(has Transform)");
                                } else {
                                    ui.label(egui::RichText::new("(no Transform)").weak());
                                }
                            });

                        // Additional components can be added here in M6
                    }
                }
            });
    }
}

impl Default for PropertiesPanel { fn default() -> Self { Self::new() } }
