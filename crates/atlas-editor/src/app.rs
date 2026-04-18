//! [`EditorApp`] — the editor application (M5).
//!
//! Orchestrates the winit event loop, egui UI, Vulkan renderer, ECS world,
//! and all five editor panels.  In headless mode (no GPU / no display) the
//! GPU calls are skipped so CI tests pass.

use std::sync::{Arc, Mutex};

use anyhow::Result;
use atlas_asset::AssetRegistry;
use atlas_ecs::World;
use atlas_renderer::{
    ClearColor, FrameSync, RenderConfig, RenderLoop, Swapchain, SwapchainConfig, VulkanContext,
};
use atlas_ui::{log_capture::LogEntry, UiContext, UiLogCapture};
use winit::{
    event::{Event, WindowEvent},
    event_loop::{ControlFlow, EventLoop},
    window::{Window, WindowBuilder},
};

use crate::{
    command::CommandStack,
    panels::{AssetBrowserPanel, ConsolePanel, OutlinerPanel, PropertiesPanel, ViewportPanel},
    scene_renderer::SceneRenderer,
    selection::SelectionState,
};

// ── EditorApp ───────────────────────────────────────────────────────────────

/// Main editor application.  Drives the winit + egui + Vulkan frame loop.
pub struct EditorApp {
    // ECS
    world:     World,
    assets:    AssetRegistry,
    // Rendering
    render:    RenderLoop,
    // UI
    ui:        UiContext,
    // Editor state
    selection: SelectionState,
    commands:  CommandStack,
    scene:     SceneRenderer,
    // Panels
    outliner:    OutlinerPanel,
    properties:  PropertiesPanel,
    viewport:    ViewportPanel,
    asset_panel: AssetBrowserPanel,
    console:     ConsolePanel,
    // Misc
    log_entries: Arc<Mutex<Vec<LogEntry>>>,
    show_about:  bool,
}

impl EditorApp {
    /// Create the application.  Requires the window to have been created
    /// already so egui and Vulkan can both reference it.
    fn init(window: &Window, log_entries: Arc<Mutex<Vec<LogEntry>>>) -> Result<Self> {
        let mut config = RenderConfig::default();
        config.title = "Atlas Workspace — Editor".into();

        // Build GPU context (falls back to headless gracefully)
        let ctx = VulkanContext::new_with_window(config.clone(), Some(window))?;
        let headless = ctx.is_headless();

        // Swapchain
        let (w, h) = {
            let sz = window.inner_size();
            (sz.width.max(1), sz.height.max(1))
        };
        let sc_cfg = SwapchainConfig {
            width: w, height: h,
            ..Default::default()
        };

        let swapchain = if headless {
            Swapchain::new_headless(sc_cfg)?
        } else {
            #[cfg(feature = "vulkan")]
            { Swapchain::new_vulkan(sc_cfg, &ctx)? }
            #[cfg(not(feature = "vulkan"))]
            { Swapchain::new_headless(sc_cfg)? }
        };

        // Frame sync
        let frame_sync = if headless {
            FrameSync::new_headless(config.frames_in_flight)
        } else {
            #[cfg(feature = "vulkan")]
            {
                let device = ctx.device().expect("GPU context must have device");
                let gfx_family = ctx.queue_families().expect("queues").graphics;
                FrameSync::new_vulkan(config.frames_in_flight, device, gfx_family)?
            }
            #[cfg(not(feature = "vulkan"))]
            { FrameSync::new_headless(config.frames_in_flight) }
        };

        let render = RenderLoop::new(ctx, swapchain, frame_sync);

        // egui
        let ui = UiContext::new(window);

        // Style — dark theme
        ui.egui_ctx().set_style(egui_dark_style());

        Ok(Self {
            world: World::new(),
            assets: AssetRegistry::new(),
            render,
            ui,
            selection:   SelectionState::new(),
            commands:    CommandStack::default(),
            scene:       SceneRenderer::new(),
            outliner:    OutlinerPanel::new(),
            properties:  PropertiesPanel::new(),
            viewport:    ViewportPanel::new(),
            asset_panel: AssetBrowserPanel::new(),
            console:     ConsolePanel::new(log_entries.clone()),
            log_entries,
            show_about:  false,
        })
    }

    /// Run the editor — consumes the app and drives the winit event loop.
    pub fn run() -> Result<()> {
        // Capture log output for the console panel
        let log_capture  = UiLogCapture::new();
        let log_entries  = log_capture.install();

        log::info!("[Editor] Starting Atlas Workspace");

        // Create event loop and window
        let event_loop = EventLoop::new()?;
        let window = WindowBuilder::new()
            .with_title("Atlas Workspace")
            .with_inner_size(winit::dpi::LogicalSize::new(1920u32, 1080u32))
            .with_min_inner_size(winit::dpi::LogicalSize::new(800u32, 600u32))
            .build(&event_loop)?;

        let mut app = match EditorApp::init(&window, log_entries) {
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
                    let consumed = app.ui.handle_event(&window, we);

                    if !consumed {
                        match we {
                            WindowEvent::CloseRequested => elwt.exit(),
                            WindowEvent::Resized(size) => {
                                app.render.on_resize(size.width, size.height);
                            }
                            WindowEvent::KeyboardInput { event, .. } => {
                                use winit::event::ElementState::Pressed;
                                if event.state == Pressed {
                                    app.handle_key(event, &window);
                                }
                            }
                            _ => {}
                        }
                    }
                }
                Event::AboutToWait => {
                    window.request_redraw();
                }
                Event::WindowEvent { event: WindowEvent::RedrawRequested, .. } => {
                    app.draw_frame(&window);
                }
                _ => {}
            }
        })?;

        Ok(())
    }

    // ── frame ────────────────────────────────────────────────────────────

    fn draw_frame(&mut self, window: &Window) {
        // 1. Begin egui frame
        let ctx = self.ui.begin_frame(window).clone();

        // 2. Draw editor UI
        self.draw_ui(&ctx);

        // 3. End egui frame → tessellate
        let _output = self.ui.end_frame(window);

        // 4. Submit GPU frame
        // TODO M4: pass _output.paint_jobs to Vulkan egui renderer
        if let Err(e) = self.render.draw_frame(None) {
            log::error!("[Editor] draw_frame error: {e}");
        }
    }

    fn draw_ui(&mut self, ctx: &egui::Context) {
        self.menu_bar(ctx);
        self.outliner.show(ctx, &self.world, &mut self.selection);
        self.properties.show(ctx, &self.world, &self.selection);

        // Asset browser + console share the bottom panel; console takes priority
        // (shown if both open, they stack)
        if self.console.open {
            self.console.show(ctx);
        }
        if self.asset_panel.open {
            self.asset_panel.show(ctx, &self.assets);
        }

        if self.viewport.open {
            self.viewport.show(ctx, &mut self.scene, &self.world, &mut self.selection);
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
                        ui.close_menu();
                    }
                    if ui.button("Open Scene…").clicked() {
                        log::info!("[Editor] Open — not yet implemented");
                        ui.close_menu();
                    }
                    if ui.button("Save Scene…").clicked() {
                        log::info!("[Editor] Save — not yet implemented");
                        ui.close_menu();
                    }
                    ui.separator();
                    if ui.button("Exit").clicked() {
                        // The event loop will handle the actual exit via CloseRequested
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
                });

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

                // Right-side: backend indicator
                ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                    ui.label(
                        egui::RichText::new(self.render_backend_name())
                            .color(egui::Color32::GRAY)
                            .small()
                    );
                });
            });
        });
    }

    fn handle_key(&mut self, event: &winit::event::KeyEvent, _window: &Window) {
        use winit::keyboard::{Key, NamedKey};
        match &event.logical_key {
            Key::Named(NamedKey::F5)  => { log::info!("[Editor] F5 — play (stub)"); }
            Key::Character(c)         => match c.as_str() {
                "z" | "Z" => { self.commands.undo(&mut self.world); }
                "y" | "Y" => { self.commands.redo(&mut self.world); }
                _ => {}
            }
            _ => {}
        }
    }

    fn render_backend_name(&self) -> &'static str {
        // We can't directly ask the render loop, so indicate headless vs GPU
        if std::env::var("ATLAS_HEADLESS").is_ok() { "Headless" } else { "Vulkan" }
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
