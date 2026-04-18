//! Asset Browser panel — lists registered assets (M5).

use atlas_asset::AssetRegistry;

pub struct AssetBrowserPanel {
    pub open:   bool,
    filter:     String,
}

impl AssetBrowserPanel {
    pub fn new() -> Self { Self { open: true, filter: String::new() } }

    pub fn show(&mut self, ctx: &egui::Context, registry: &AssetRegistry) {
        egui::TopBottomPanel::bottom("asset_browser")
            .resizable(true)
            .default_height(180.0)
            .show(ctx, |ui| {
                ui.horizontal(|ui| {
                    ui.heading("Asset Browser");
                    ui.separator();
                    ui.label("Filter:");
                    ui.text_edit_singleline(&mut self.filter);
                });
                ui.separator();

                egui::ScrollArea::vertical().id_source("asset_scroll").show(ui, |ui| {
                    egui::Grid::new("asset_grid")
                        .num_columns(3)
                        .striped(true)
                        .show(ui, |ui| {
                            ui.strong("Name");
                            ui.strong("Type");
                            ui.strong("Path");
                            ui.end_row();

                            // Iterate registry — AssetRegistry exposes assets via list_by_type or count
                            let filter = self.filter.to_lowercase();
                            let count = registry.count();
                            if count == 0 {
                                ui.label(egui::RichText::new("(no assets registered)").weak());
                                ui.end_row();
                            } else {
                                // We can't iterate all assets from AssetRegistry directly (no iter()),
                                // so we just show the count as a placeholder until an iterator is added.
                                ui.label(format!("{count} asset(s) registered"));
                                if !filter.is_empty() {
                                    ui.label(format!("(filter: {filter})"));
                                }
                                ui.end_row();
                            }
                        });
                });
            });
    }
}

impl Default for AssetBrowserPanel { fn default() -> Self { Self::new() } }
