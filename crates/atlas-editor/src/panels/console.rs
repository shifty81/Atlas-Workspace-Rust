//! Console / Log panel — displays captured log output (M5).

use std::sync::{Arc, Mutex};

use atlas_ui::log_capture::LogEntry;

pub struct ConsolePanel {
    pub open:        bool,
    entries:         Arc<Mutex<Vec<LogEntry>>>,
    auto_scroll:     bool,
    level_filter:    log::Level,
}

impl ConsolePanel {
    pub fn new(entries: Arc<Mutex<Vec<LogEntry>>>) -> Self {
        Self { open: true, entries, auto_scroll: true, level_filter: log::Level::Debug }
    }

    pub fn show(&mut self, ctx: &egui::Context) {
        egui::TopBottomPanel::bottom("console")
            .resizable(true)
            .default_height(160.0)
            .show(ctx, |ui| {
                // Header toolbar
                ui.horizontal(|ui| {
                    ui.heading("Console");
                    ui.separator();
                    ui.checkbox(&mut self.auto_scroll, "Auto-scroll");
                    ui.separator();
                    egui::ComboBox::from_id_source("log_level")
                        .selected_text(format!("{}", self.level_filter))
                        .show_ui(ui, |ui| {
                            for lvl in [log::Level::Error, log::Level::Warn, log::Level::Info, log::Level::Debug, log::Level::Trace] {
                                ui.selectable_value(&mut self.level_filter, lvl, format!("{lvl}"));
                            }
                        });
                    if ui.button("Clear").clicked() {
                        if let Ok(mut v) = self.entries.lock() { v.clear(); }
                    }
                });
                ui.separator();

                let entries = self.entries.lock().unwrap_or_else(|p| p.into_inner());
                let filtered: Vec<&LogEntry> = entries.iter()
                    .filter(|e| e.level <= self.level_filter)
                    .collect();

                let text_style = egui::TextStyle::Monospace;
                let row_height = ui.text_style_height(&text_style);

                egui::ScrollArea::vertical()
                    .auto_shrink([false; 2])
                    .stick_to_bottom(self.auto_scroll)
                    .show_rows(ui, row_height, filtered.len(), |ui, range| {
                        for entry in &filtered[range] {
                            let color = level_color(entry.level);
                            let text = format!(
                                "[{}] {} — {}",
                                entry.level, entry.target, entry.message
                            );
                            ui.label(egui::RichText::new(text).monospace().color(color));
                        }
                    });
            });
    }
}

fn level_color(level: log::Level) -> egui::Color32 {
    match level {
        log::Level::Error => egui::Color32::from_rgb(255, 80,  80),
        log::Level::Warn  => egui::Color32::from_rgb(255, 200, 80),
        log::Level::Info  => egui::Color32::WHITE,
        log::Level::Debug => egui::Color32::from_rgb(160, 160, 255),
        log::Level::Trace => egui::Color32::GRAY,
    }
}
