//! [`EditorApp`] — the editor application (M7).
//!
//! Orchestrates the winit event loop, egui UI, Vulkan renderer, ECS world,
//! and all five editor panels.  In headless mode (no GPU / no display) the
//! GPU calls are skipped so CI tests pass.

use std::sync::{Arc, Mutex};

use anyhow::Result;
use atlas_asset::AssetRegistry;
use atlas_ecs::World;
use atlas_renderer::{
    FrameSync, RenderConfig, RenderLoop, Swapchain, SwapchainConfig, VulkanContext,
};
use atlas_ui::{log_capture::LogEntry, UiContext, UiLogCapture, UiRenderer};
use winit::{
    event::{Event, WindowEvent},
    event_loop::{ControlFlow, EventLoop},
    window::{Window, WindowBuilder},
};

use crate::{
    build_system::GameBuildSystem,
    command::CommandStack,
    entity_commands::{DeleteEntityCommand, RenameEntityCommand, SpawnEntityCommand},
    game_project_adapter::{EditorSession, PieState},
    panels::{
        AssetBrowserPanel, ConsolePanel, OutlinerEvent, OutlinerPanel,
        PropertiesEvent, PropertiesPanel, ViewportPanel,
    },
    scene_renderer::SceneRenderer,
    scene_serial,
    selection::SelectionState,
};

// ── EditorApp ───────────────────────────────────────────────────────────────

/// Main editor application.  Drives the winit + egui + wgpu frame loop.
pub struct EditorApp {
    // ECS
    world:     World,
    assets:    AssetRegistry,
    // Rendering
    render:    RenderLoop,
    /// wgpu-backed egui renderer.  `None` when no GPU is available.
    ui_renderer: Option<UiRenderer>,
    // UI
    ui:        UiContext,
    // Editor state
    selection: SelectionState,
    commands:  CommandStack,
    scene:     SceneRenderer,
    // Game / PIE
    session:   EditorSession,
    build_sys: GameBuildSystem,
    // Panels
    outliner:    OutlinerPanel,
    properties:  PropertiesPanel,
    viewport:    ViewportPanel,
    asset_panel: AssetBrowserPanel,
    console:     ConsolePanel,
    // Misc
    _log_entries:    Arc<Mutex<Vec<LogEntry>>>,
    show_about:      bool,
    /// Path of the most recently saved / opened scene file.
    last_scene_path: Option<std::path::PathBuf>,
    /// Toast message shown briefly in the status bar.
    status_message:  Option<String>,
}

impl EditorApp {
    /// Create the application.  Requires the window to have been created
    /// already so egui and Vulkan can both reference it.
    fn init(window: Arc<Window>, log_entries: Arc<Mutex<Vec<LogEntry>>>) -> Result<Self> {
        let mut config = RenderConfig::default();
        config.title = "Atlas Workspace — Editor".into();

        // Build a headless GPU context.  The Vulkan swapchain is intentionally
        // kept headless here — the window surface is owned by the wgpu-based
        // UiRenderer below, avoiding a conflict between two Vulkan instances
        // on the same HWND.  The headless RenderLoop remains available for
        // future off-screen GPU work.
        let ctx = VulkanContext::new_with_window(config.clone(), None::<&Window>)?;

        // Swapchain / FrameSync (headless stubs — ctx is always headless here)
        let (w, h) = {
            let sz = window.inner_size();
            (sz.width.max(1), sz.height.max(1))
        };
        let sc_cfg = SwapchainConfig {
            width: w, height: h,
            ..Default::default()
        };
        let swapchain  = Swapchain::new_headless(sc_cfg)?;
        let frame_sync = FrameSync::new_headless(config.frames_in_flight);
        let render     = RenderLoop::new(ctx, swapchain, frame_sync);

        // egui
        let ui = UiContext::new(&*window);

        // Style — dark theme
        ui.egui_ctx().set_style(egui_dark_style());

        // wgpu-backed egui renderer.  Failure (e.g. no GPU) degrades to the
        // headless path — the window stays dark-grey but the process runs.
        let ui_renderer = match UiRenderer::new(window.clone()) {
            Ok(r)  => { log::info!("[Editor] UiRenderer ready (wgpu)"); Some(r) }
            Err(e) => { log::warn!("[Editor] UiRenderer unavailable: {e}"); None  }
        };

        Ok(Self {
            world: World::new(),
            assets: AssetRegistry::new(),
            render,
            ui_renderer,
            ui,
            selection:        SelectionState::new(),
            commands:         CommandStack::default(),
            scene:            SceneRenderer::new(),
            session:          EditorSession::new(),
            build_sys:        GameBuildSystem::new(),
            outliner:         OutlinerPanel::new(),
            properties:       PropertiesPanel::new(),
            viewport:         ViewportPanel::new(),
            asset_panel:      AssetBrowserPanel::new(),
            console:          ConsolePanel::new(log_entries.clone()),
            _log_entries:     log_entries,
            show_about:       false,
            last_scene_path:  None,
            status_message:   None,
        })
    }

    /// Run the editor — consumes the app and drives the winit event loop.
    pub fn run() -> Result<()> {
        // Capture log output for the console panel
        let log_capture  = UiLogCapture::new();
        let log_entries  = log_capture.install();

        log::info!("[Editor] Starting Atlas Workspace");

        // Create event loop and window.
        // `Arc` lets both the event loop closure and `UiRenderer` share the
        // window handle without lifetime complications.
        let event_loop = EventLoop::new()?;
        let window = Arc::new(
            WindowBuilder::new()
                .with_title("Atlas Workspace")
                .with_inner_size(winit::dpi::LogicalSize::new(1920u32, 1080u32))
                .with_min_inner_size(winit::dpi::LogicalSize::new(800u32, 600u32))
                .build(&event_loop)?,
        );

        let mut app = match EditorApp::init(window.clone(), log_entries) {
            Ok(a) => a,
            Err(e) => {
                log::error!("[Editor] Init failed: {e}");
                return Err(e);
            }
        };

        log::info!("[Editor] Backend: {}", app.render_backend_name());

        // ── Event loop ───────────────────────────────────────────────────
        event_loop.run(move |event, elwt| {
            elwt.set_control_flow(ControlFlow::Poll);

            match &event {
                Event::WindowEvent { event: we, .. } => {
                    // Feed to egui first
                    let consumed = app.ui.handle_event(&*window, we);

                    if !consumed {
                        match we {
                            WindowEvent::CloseRequested => elwt.exit(),
                            WindowEvent::Resized(size) => {
                                app.on_resize(size.width, size.height);
                            }
                            WindowEvent::RedrawRequested => {
                                app.draw_frame(&*window);
                            }
                            WindowEvent::KeyboardInput { event, .. } => {
                                use winit::event::ElementState::Pressed;
                                if event.state == Pressed {
                                    app.handle_key(event, &*window);
                                }
                            }
                            _ => {}
                        }
                    }
                }
                Event::AboutToWait => {
                    window.request_redraw();
                }
                _ => {}
            }
        })?;

        Ok(())
    }

    // ── frame ────────────────────────────────────────────────────────────

    fn draw_frame(&mut self, window: &Window) {
        // Poll PIE session each frame
        self.session.poll();

        // 1. Begin egui frame
        let ctx = self.ui.begin_frame(window).clone();

        // 2. Draw editor UI
        self.draw_ui(&ctx);

        // 3. End egui frame → tessellate
        let output = self.ui.end_frame(window);

        // 4. Paint egui output to the window via the wgpu renderer.
        //    If the wgpu renderer is unavailable (no GPU), fall back to the
        //    headless Vulkan path (which is a no-op but keeps the loop alive).
        if let Some(ref mut renderer) = self.ui_renderer {
            renderer.paint(output);
        } else if let Err(e) = self.render.draw_frame(None) {
            log::error!("[Editor] draw_frame error: {e}");
        }
    }

    /// Forward a resize event to both the headless render loop and the wgpu
    /// surface so neither falls out of sync with the window dimensions.
    fn on_resize(&mut self, width: u32, height: u32) {
        self.render.on_resize(width, height);
        if let Some(ref mut renderer) = self.ui_renderer {
            renderer.on_resize(width, height);
        }
    }

    fn draw_ui(&mut self, ctx: &egui::Context) {
        self.menu_bar(ctx);

        // Outliner returns events to process after the borrow ends
        let outliner_events = self.outliner.show(ctx, &self.world, &mut self.selection);
        for event in outliner_events {
            match event {
                OutlinerEvent::SpawnEntity => {
                    self.commands.execute(Box::new(SpawnEntityCommand::new()), &mut self.world);
                }
                OutlinerEvent::DeleteEntity(id) => {
                    self.commands.execute(Box::new(DeleteEntityCommand::new(id)), &mut self.world);
                    self.selection.clear();
                }
            }
        }

        // Properties returns events (e.g. rename)
        let props_events = self.properties.show(ctx, &mut self.world, &self.selection);
        for event in props_events {
            match event {
                PropertiesEvent::Rename { name } => {
                    if let Some(entity) = self.selection.primary() {
                        self.commands.execute(
                            Box::new(RenameEntityCommand::new(entity, name)),
                            &mut self.world,
                        );
                    }
                }
            }
        }

        // Asset browser + console share the bottom panel; console takes priority
        if self.console.open {
            self.console.show(ctx);
        }
        if self.asset_panel.open {
            self.asset_panel.show(ctx, &self.assets);
        }

        if self.viewport.open {
            self.viewport.show(ctx, &mut self.scene, &self.world, &mut self.selection);
        }

        // Status bar
        if let Some(msg) = &self.status_message {
            egui::TopBottomPanel::bottom("status_bar").show(ctx, |ui| {
                ui.label(egui::RichText::new(msg).small().color(egui::Color32::from_rgb(200, 220, 200)));
            });
        }

        // About dialog
        if self.show_about {
            egui::Window::new("About Atlas Workspace")
                .collapsible(false)
                .resizable(false)
                .show(ctx, |ui| {
                    ui.label("Atlas Workspace — a Rust-native game editor");
                    ui.label("Version 0.1.0");
                    if ui.button("Close").clicked() { self.show_about = false; }
                });
        }
    }

    fn menu_bar(&mut self, ctx: &egui::Context) {
        egui::TopBottomPanel::top("menu_bar").show(ctx, |ui| {
            egui::menu::bar(ui, |ui| {
                ui.menu_button("File", |ui| {
                    if ui.button("New Scene").clicked() {
                        self.world = World::new();
                        self.selection.clear();
                        self.commands.clear();
                        self.last_scene_path = None;
                        self.status_message = Some("New scene created.".into());
                        ui.close_menu();
                    }
                    if ui.button("Open Scene…").clicked() {
                        self.open_scene_dialog();
                        ui.close_menu();
                    }
                    ui.separator();
                    if ui.add_enabled(
                        self.last_scene_path.is_some(),
                        egui::Button::new("Save Scene"),
                    ).clicked() {
                        self.save_scene_to_last_path();
                        ui.close_menu();
                    }
                    if ui.button("Save Scene As…").clicked() {
                        self.save_scene_dialog();
                        ui.close_menu();
                    }
                    ui.separator();
                    if ui.button("Exit").clicked() {
                        std::process::exit(0);
                    }
                });

                ui.menu_button("Edit", |ui| {
                    let can_undo = self.commands.can_undo();
                    let can_redo = self.commands.can_redo();

                    let undo_label = match self.commands.undo_description() {
                        Some(d) => format!("Undo: {d}"),
                        None    => "Undo".into(),
                    };
                    if ui.add_enabled(can_undo, egui::Button::new(undo_label)).clicked() {
                        self.commands.undo(&mut self.world);
                        ui.close_menu();
                    }

                    let redo_label = match self.commands.redo_description() {
                        Some(d) => format!("Redo: {d}"),
                        None    => "Redo".into(),
                    };
                    if ui.add_enabled(can_redo, egui::Button::new(redo_label)).clicked() {
                        self.commands.redo(&mut self.world);
                        ui.close_menu();
                    }

                    ui.separator();
                    if ui.button("Spawn Entity").clicked() {
                        self.commands.execute(Box::new(SpawnEntityCommand::new()), &mut self.world);
                        ui.close_menu();
                    }
                });

                // ── Play / Build toolbar ─────────────────────────────────────
                ui.separator();
                let is_playing = self.session.is_playing();
                let play_label = if is_playing { "⏹ Stop" } else { "▶ Play (F5)" };
                let play_color = if is_playing {
                    egui::Color32::from_rgb(220, 80, 80)
                } else {
                    egui::Color32::from_rgb(80, 200, 80)
                };

                if ui.add(egui::Button::new(
                    egui::RichText::new(play_label).color(play_color).strong()
                )).clicked() {
                    if is_playing {
                        self.session.stop_pie();
                    } else {
                        self.session.start_pie(&self.world);
                    }
                }

                let build_label = if self.build_sys.is_building() { "⟳ Building…" } else { "🔨 Build Game" };
                if ui.add_enabled(
                    !self.build_sys.is_building(),
                    egui::Button::new(build_label),
                ).clicked() {
                    self.build_sys.start_build(false);
                }

                // Show PIE state badge
                let pie_text = match self.session.pie_state() {
                    PieState::Idle     => String::new(),
                    PieState::Starting => "PIE: Starting…".into(),
                    PieState::Running  => "PIE: Running".into(),
                    PieState::Stopping => "PIE: Stopping…".into(),
                    PieState::Error(e) => format!("PIE Error: {e}"),
                };
                if !pie_text.is_empty() {
                    ui.separator();
                    ui.label(egui::RichText::new(&pie_text).small().color(egui::Color32::YELLOW));
                }

                ui.menu_button("Window", |ui| {
                    ui.checkbox(&mut self.outliner.open,    "Outliner");
                    ui.checkbox(&mut self.properties.open,  "Properties");
                    ui.checkbox(&mut self.viewport.open,    "Viewport");
                    ui.checkbox(&mut self.asset_panel.open, "Asset Browser");
                    ui.checkbox(&mut self.console.open,     "Console");
                });

                ui.menu_button("Help", |ui| {
                    if ui.button("About…").clicked() {
                        self.show_about = true;
                        ui.close_menu();
                    }
                });

                // Right-side: backend indicator + scene path
                ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                    ui.label(
                        egui::RichText::new(self.render_backend_name())
                            .color(egui::Color32::GRAY)
                            .small()
                    );
                    if let Some(path) = &self.last_scene_path {
                        ui.separator();
                        ui.label(
                            egui::RichText::new(path.display().to_string())
                                .color(egui::Color32::GRAY)
                                .small()
                        );
                    }
                });
            });
        });
    }

    // ── Scene I/O helpers ─────────────────────────────────────────────────────

    fn open_scene_dialog(&mut self) {
        let path = self.last_scene_path
            .clone()
            .unwrap_or_else(|| std::path::PathBuf::from("scene.json"));
        match scene_serial::load_scene(&mut self.world, &path) {
            Ok(()) => {
                self.selection.clear();
                self.commands.clear();
                self.last_scene_path = Some(path.clone());
                self.status_message  = Some(format!("Opened: {}", path.display()));
                log::info!("[Editor] Scene loaded from {:?}", path);
            }
            Err(e) => {
                self.status_message = Some(format!("Open failed: {e}"));
                log::error!("[Editor] Open scene failed: {e}");
            }
        }
    }

    fn save_scene_to_last_path(&mut self) {
        if let Some(path) = self.last_scene_path.clone() {
            match scene_serial::save_scene(&self.world, &path) {
                Ok(()) => {
                    self.status_message = Some(format!("Saved: {}", path.display()));
                }
                Err(e) => {
                    self.status_message = Some(format!("Save failed: {e}"));
                    log::error!("[Editor] Save failed: {e}");
                }
            }
        }
    }

    fn save_scene_dialog(&mut self) {
        let path = self.last_scene_path
            .clone()
            .unwrap_or_else(|| std::path::PathBuf::from("scene.json"));
        match scene_serial::save_scene(&self.world, &path) {
            Ok(()) => {
                self.last_scene_path = Some(path.clone());
                self.status_message  = Some(format!("Saved: {}", path.display()));
                log::info!("[Editor] Scene saved to {:?}", path);
            }
            Err(e) => {
                self.status_message = Some(format!("Save failed: {e}"));
                log::error!("[Editor] Save scene failed: {e}");
            }
        }
    }

    // ── Key input ─────────────────────────────────────────────────────────────

    fn handle_key(&mut self, event: &winit::event::KeyEvent, _window: &Window) {
        use winit::keyboard::{Key, NamedKey};
        let pressed = event.state == winit::event::ElementState::Pressed;
        if !pressed { return; }
        match &event.logical_key {
            Key::Named(NamedKey::F5) => {
                if self.session.is_playing() {
                    self.session.stop_pie();
                } else {
                    self.session.start_pie(&self.world);
                }
            }
            Key::Character(c) => match c.as_str() {
                "z" | "Z" => { self.commands.undo(&mut self.world); }
                "y" | "Y" => { self.commands.redo(&mut self.world); }
                // Ctrl+S — save to last path, or use save-as dialog if no path yet
                "s" | "S" => {
                    if self.last_scene_path.is_some() {
                        self.save_scene_to_last_path();
                    } else {
                        self.save_scene_dialog();
                    }
                }
                _ => {}
            }
            _ => {}
        }
    }

    fn render_backend_name(&self) -> &'static str {
        match &self.ui_renderer {
            Some(_) => "wgpu",
            None    => "Headless",
        }
    }
}

impl Drop for EditorApp {
    fn drop(&mut self) {
        self.render.wait_idle();
    }
}

// ── Theme ────────────────────────────────────────────────────────────────────

fn egui_dark_style() -> egui::Style {
    let mut style = egui::Style::default();
    style.visuals = egui::Visuals::dark();
    // Slightly reduce window rounding for a more professional look
    style.visuals.window_rounding = egui::Rounding::same(4.0);
    style
}
