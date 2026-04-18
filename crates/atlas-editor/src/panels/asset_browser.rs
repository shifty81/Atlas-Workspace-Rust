//! Asset Browser panel — lists registered assets (M7).

use atlas_asset::AssetRegistry;

pub struct AssetBrowserPanel {
    pub open:   bool,
    filter:     String,
    type_filter: String,
}

impl AssetBrowserPanel {
    pub fn new() -> Self { Self { open: true, filter: String::new(), type_filter: String::new() } }

    pub fn show(&mut self, ctx: &egui::Context, registry: &AssetRegistry) {
        egui::TopBottomPanel::bottom("asset_browser")
            .resizable(true)
            .default_height(180.0)
            .show(ctx, |ui| {
                ui.horizontal(|ui| {
                    ui.heading("Asset Browser");
                    ui.separator();
                    ui.label("Name:");
                    ui.add(egui::TextEdit::singleline(&mut self.filter).hint_text("filter…").desired_width(120.0));
                    ui.label("Type:");
                    ui.add(egui::TextEdit::singleline(&mut self.type_filter).hint_text("mesh, texture…").desired_width(100.0));
                    if ui.small_button("✕").on_hover_text("Clear filters").clicked() {
                        self.filter.clear();
                        self.type_filter.clear();
                    }
                    ui.separator();
                    ui.weak(format!("{} asset(s)", registry.count()));
                });
                ui.separator();

                let name_filter = self.filter.to_lowercase();
                let type_filter = self.type_filter.to_lowercase();

                egui::ScrollArea::vertical().id_source("asset_scroll").show(ui, |ui| {
                    egui::Grid::new("asset_grid")
                        .num_columns(4)
                        .striped(true)
                        .min_col_width(80.0)
                        .show(ui, |ui| {
                            ui.strong("Name");
                            ui.strong("Type");
                            ui.strong("Path");
                            ui.strong("Version");
                            ui.end_row();

                            let mut any = false;
                            for asset in registry.iter() {
                                let name_match = name_filter.is_empty()
                                    || asset.name.to_lowercase().contains(&name_filter);
                                let type_match = type_filter.is_empty()
                                    || asset.asset_type.to_lowercase().contains(&type_filter);
                                if !name_match || !type_match { continue; }

                                any = true;
                                ui.label(&asset.name);
                                ui.label(
                                    egui::RichText::new(&asset.asset_type)
                                        .color(egui::Color32::from_rgb(130, 180, 255)),
                                );
                                ui.label(
                                    egui::RichText::new(&asset.path)
                                        .color(egui::Color32::GRAY)
                                        .small(),
                                );
                                ui.label(format!("v{}", asset.version));
                                ui.end_row();
                            }

                            if !any {
                                ui.label(egui::RichText::new(
                                    if registry.count() == 0 {
                                        "(no assets registered)"
                                    } else {
                                        "(no assets match filter)"
                                    }
                                ).weak());
                                ui.end_row();
                            }
                        });
                });
            });
    }
}

impl Default for AssetBrowserPanel { fn default() -> Self { Self::new() } }
