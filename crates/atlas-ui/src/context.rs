//! [`UiContext`] — wraps egui context + winit event state (M4).

use egui::ViewportId;
use winit::{event::WindowEvent, window::Window};

/// Output of a completed egui frame.
pub struct FrameOutput {
    /// Full egui output (shapes, textures deltas, platform output, …).
    pub full_output: egui::FullOutput,
    /// Tessellated paint jobs ready for the GPU renderer.
    pub paint_jobs:  Vec<egui::ClippedPrimitive>,
    /// Pixels-per-point used for tessellation.
    pub pixels_per_point: f32,
}

/// Owns an [`egui::Context`] and the winit integration state.
///
/// Typical frame:
/// ```ignore
/// let ctx = ui_context.begin_frame(&window);
/// egui::CentralPanel::default().show(&ctx, |ui| { /* widgets */ });
/// let output = ui_context.end_frame();
/// // hand output.paint_jobs to the GPU renderer
/// ```
pub struct UiContext {
    ctx:    egui::Context,
    state:  egui_winit::State,
}

impl UiContext {
    /// Construct from a live winit `Window`.
    pub fn new(window: &Window) -> Self {
        let ctx = egui::Context::default();
        let state = egui_winit::State::new(
            ctx.clone(),
            ViewportId::ROOT,
            window,
            Some(window.scale_factor() as f32),
            None,
        );
        Self { ctx, state }
    }

    /// Pass a winit window event into egui before beginning the frame.
    /// Returns `true` if egui consumed the event (so the editor should skip it).
    pub fn handle_event(&mut self, window: &Window, event: &WindowEvent) -> bool {
        let response = self.state.on_window_event(window, event);
        response.consumed
    }

    /// Start an egui frame.  Returns the [`egui::Context`] to draw widgets on.
    pub fn begin_frame(&mut self, window: &Window) -> &egui::Context {
        let raw_input = self.state.take_egui_input(window);
        self.ctx.begin_frame(raw_input);
        &self.ctx
    }

    /// Finish the frame and produce tessellated paint data.
    pub fn end_frame(&mut self, window: &Window) -> FrameOutput {
        let full_output = self.ctx.end_frame();
        self.state.handle_platform_output(window, full_output.platform_output.clone());
        let ppp = self.ctx.pixels_per_point();
        let paint_jobs = self.ctx.tessellate(full_output.shapes.clone(), ppp);
        FrameOutput { full_output, paint_jobs, pixels_per_point: ppp }
    }

    /// Direct access to the egui context (for style changes, etc.).
    pub fn egui_ctx(&self) -> &egui::Context { &self.ctx }
}
