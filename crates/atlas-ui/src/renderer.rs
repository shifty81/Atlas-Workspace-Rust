//! [`UiRenderer`] — wgpu-backed egui renderer (M16).
//!
//! Owns the wgpu surface, device, queue, and the [`egui_wgpu::Renderer`].
//! Called once per frame by `EditorApp::draw_frame` to paint egui output
//! directly to the winit window surface.
//!
//! The VulkanContext in `atlas-renderer` is initialised headless so it does
//! not compete with wgpu for the window surface.  It remains available for
//! off-screen 3D rendering in future milestones.

use std::sync::Arc;

use winit::window::Window;

use crate::context::FrameOutput;

/// Background clear colour (linear RGBA) that matches the dark-grey theme.
const CLEAR_COLOR: wgpu::Color = wgpu::Color { r: 0.08, g: 0.08, b: 0.08, a: 1.0 };

// ── UiRenderer ───────────────────────────────────────────────────────────────

/// Renders egui paint jobs to a wgpu window surface.
///
/// Typical lifecycle:
/// ```ignore
/// let renderer = UiRenderer::new(window.clone())?;
/// // each frame:
/// renderer.paint(frame_output);
/// // on resize:
/// renderer.on_resize(new_width, new_height);
/// ```
pub struct UiRenderer {
    /// Kept alive so the surface handle remains valid for the program's
    /// lifetime.  `Arc` lets both the event loop and this struct share the
    /// same `Window`.
    _window:        Arc<Window>,
    surface:        wgpu::Surface<'static>,
    device:         wgpu::Device,
    queue:          wgpu::Queue,
    surface_config: wgpu::SurfaceConfiguration,
    renderer:       egui_wgpu::Renderer,
}

impl UiRenderer {
    /// Create a renderer for `window`.
    ///
    /// Blocks briefly while wgpu negotiates the adapter and device.
    /// Returns an error if no suitable GPU is available.
    pub fn new(window: Arc<Window>) -> anyhow::Result<Self> {
        let instance = wgpu::Instance::new(wgpu::InstanceDescriptor::default());

        // `Arc<Window>` satisfies `Into<SurfaceTarget<'static>>` so the
        // resulting `Surface<'static>` is safe to store without a lifetime.
        let surface: wgpu::Surface<'static> = instance
            .create_surface(window.clone())
            .map_err(|e| anyhow::anyhow!("wgpu surface: {e}"))?;

        let adapter = pollster::block_on(instance.request_adapter(
            &wgpu::RequestAdapterOptions {
                power_preference:       wgpu::PowerPreference::HighPerformance,
                compatible_surface:     Some(&surface),
                force_fallback_adapter: false,
            },
        ))
        .ok_or_else(|| anyhow::anyhow!("no suitable GPU adapter found"))?;

        log::info!(
            "[UiRenderer] adapter: {} ({:?})",
            adapter.get_info().name,
            adapter.get_info().backend,
        );

        let (device, queue) = pollster::block_on(adapter.request_device(
            &wgpu::DeviceDescriptor {
                label:              Some("atlas-ui"),
                required_features:  wgpu::Features::empty(),
                required_limits:    wgpu::Limits::default(),
            },
            None,
        ))
        .map_err(|e| anyhow::anyhow!("wgpu device: {e}"))?;

        let size  = window.inner_size();
        let caps  = surface.get_capabilities(&adapter);

        // Prefer sRGB for correct colour reproduction; fall back to whatever
        // the surface supports.
        let format = caps
            .formats
            .iter()
            .copied()
            .find(|f| f.is_srgb())
            .unwrap_or(caps.formats[0]);

        let surface_config = wgpu::SurfaceConfiguration {
            usage:                        wgpu::TextureUsages::RENDER_ATTACHMENT,
            format,
            width:                        size.width.max(1),
            height:                       size.height.max(1),
            present_mode:                 wgpu::PresentMode::AutoVsync,
            // 2 frames of latency is a balanced default: one frame buffered
            // while the previous is on-screen.  Lower values reduce input lag
            // but increase GPU stalls; higher values smooth pacing at the cost
            // of responsiveness.
            desired_maximum_frame_latency: 2,
            alpha_mode:                   caps.alpha_modes[0],
            view_formats:                 vec![],
        };
        surface.configure(&device, &surface_config);

        let renderer = egui_wgpu::Renderer::new(&device, format, None, 1);

        Ok(Self {
            _window: window,
            surface,
            device,
            queue,
            surface_config,
            renderer,
        })
    }

    /// Reconfigure the wgpu surface after a window resize.
    pub fn on_resize(&mut self, width: u32, height: u32) {
        if width == 0 || height == 0 { return; }
        self.surface_config.width  = width;
        self.surface_config.height = height;
        self.surface.configure(&self.device, &self.surface_config);
    }

    /// Paint a completed egui frame to the window surface.
    pub fn paint(&mut self, output: FrameOutput) {
        let frame = match self.surface.get_current_texture() {
            Ok(f)  => f,
            Err(e) => {
                log::warn!("[UiRenderer] failed to acquire swap-chain frame: {e}");
                return;
            }
        };
        let view = frame.texture.create_view(&wgpu::TextureViewDescriptor::default());

        let screen = egui_wgpu::ScreenDescriptor {
            size_in_pixels:   [self.surface_config.width, self.surface_config.height],
            pixels_per_point: output.pixels_per_point,
        };

        let mut encoder = self.device.create_command_encoder(
            &wgpu::CommandEncoderDescriptor { label: Some("egui_encoder") },
        );

        // Upload new / updated textures (e.g. font atlas on first frame).
        for (id, delta) in &output.full_output.textures_delta.set {
            self.renderer.update_texture(&self.device, &self.queue, *id, delta);
        }

        // Write vertex and index data for this frame's paint jobs.
        self.renderer.update_buffers(
            &self.device,
            &self.queue,
            &mut encoder,
            &output.paint_jobs,
            &screen,
        );

        // Render egui primitives in a single render pass.
        {
            let mut rp = encoder.begin_render_pass(&wgpu::RenderPassDescriptor {
                label: Some("egui_pass"),
                color_attachments: &[Some(wgpu::RenderPassColorAttachment {
                    view:           &view,
                    resolve_target: None,
                    ops: wgpu::Operations {
                        load:  wgpu::LoadOp::Clear(CLEAR_COLOR),
                        store: wgpu::StoreOp::Store,
                    },
                })],
                depth_stencil_attachment: None,
                timestamp_writes:         None,
                occlusion_query_set:      None,
            });
            self.renderer.render(&mut rp, &output.paint_jobs, &screen);
        }

        self.queue.submit(std::iter::once(encoder.finish()));
        frame.present();

        // Release textures that egui no longer references.
        for id in &output.full_output.textures_delta.free {
            self.renderer.free_texture(id);
        }
    }
}
