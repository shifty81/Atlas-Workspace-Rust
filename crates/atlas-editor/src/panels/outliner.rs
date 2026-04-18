//! Outliner panel — lists ECS entities (M5).

use atlas_ecs::World;

use crate::selection::SelectionState;

pub struct OutlinerPanel {
    pub open: bool,
}

impl OutlinerPanel {
    pub fn new() -> Self { Self { open: true } }

    pub fn show(&mut self, ctx: &egui::Context, world: &World, sel: &mut SelectionState) {
        egui::SidePanel::left("outliner")
            .resizable(true)
            .default_width(220.0)
            .show(ctx, |ui| {
                ui.heading("Outliner");
                ui.separator();

                egui::ScrollArea::vertical().show(ui, |ui| {
                    let entities = world.entities.alive().to_vec();
                    if entities.is_empty() {
                        ui.label(egui::RichText::new("(no entities)").weak());
                    }
                    for id in entities {
                        let label = format!("Entity #{id}");
                        let selected = sel.is_selected(id);
                        let response = ui.selectable_label(selected, &label);
                        if response.clicked() {
                            sel.select_one(id);
                        }
                        response.context_menu(|ui| {
                            if ui.button("Delete").clicked() {
                                ui.close_menu();
                                // EditorApp handles the actual command via delete_selected()
                            }
                        });
                    }
                });
            });
    }
}

impl Default for OutlinerPanel { fn default() -> Self { Self::new() } }
