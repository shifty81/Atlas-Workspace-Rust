//! Outliner panel — lists ECS entities (M7).

use atlas_ecs::{EntityId, Name, World};

use crate::selection::SelectionState;

/// Events produced by the Outliner that the EditorApp processes.
pub enum OutlinerEvent {
    SpawnEntity,
    DeleteEntity(EntityId),
}

pub struct OutlinerPanel {
    pub open: bool,
}

impl OutlinerPanel {
    pub fn new() -> Self { Self { open: true } }

    /// Draw the panel; returns any events the caller should process.
    pub fn show(
        &mut self,
        ctx:   &egui::Context,
        world: &World,
        sel:   &mut SelectionState,
    ) -> Vec<OutlinerEvent> {
        let mut events = Vec::new();

        egui::SidePanel::left("outliner")
            .resizable(true)
            .default_width(220.0)
            .show(ctx, |ui| {
                ui.horizontal(|ui| {
                    ui.heading("Outliner");
                    if ui.small_button("＋").on_hover_text("Spawn entity").clicked() {
                        events.push(OutlinerEvent::SpawnEntity);
                    }
                });
                ui.separator();

                egui::ScrollArea::vertical().show(ui, |ui| {
                    let entities = world.entities.alive().to_vec();
                    if entities.is_empty() {
                        ui.label(egui::RichText::new("(no entities)").weak());
                    }
                    for id in entities {
                        let label = world.components
                            .get::<Name>(id)
                            .map(|n| format!("{} [#{}]", n.as_str(), id))
                            .unwrap_or_else(|| format!("Entity #{id}"));

                        let selected = sel.is_selected(id);
                        let response = ui.selectable_label(selected, &label);
                        if response.clicked() {
                            sel.select_one(id);
                        }
                        response.context_menu(|ui| {
                            if ui.button("Delete").clicked() {
                                ui.close_menu();
                                events.push(OutlinerEvent::DeleteEntity(id));
                            }
                        });
                    }
                });
            });

        events
    }
}

impl Default for OutlinerPanel { fn default() -> Self { Self::new() } }
