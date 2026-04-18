//! Vulkan instance, physical device, logical device, queues, and surface.
//!
//! [`VulkanContext`] is the root object for all Vulkan state. On headless
//! targets (or when `ATLAS_HEADLESS=1`) it falls back to a null stub so the
//! binary can run in CI without a GPU.

use std::ffi::{CStr, CString};

use crate::types::{RenderConfig, RendererError, RendererResult};

// ── validation layer name ───────────────────────────────────────────────────

const VALIDATION_LAYER: &CStr =
    unsafe { CStr::from_bytes_with_nul_unchecked(b"VK_LAYER_KHRONOS_validation\0") };

// ── queue family bookkeeping ────────────────────────────────────────────────

/// Logical queue family indices selected for this device + surface pair.
#[derive(Clone, Copy, Debug)]
pub struct QueueFamilyIndices {
    pub graphics: u32,
    pub compute:  u32,
    pub transfer: u32,
    pub present:  u32,
}

// ── inner Vulkan state (feature-gated) ─────────────────────────────────────

#[cfg(feature = "vulkan")]
struct VulkanHandles {
    entry:        ash::Entry,
    instance:     ash::Instance,
    debug_utils:  Option<(ash::extensions::ext::DebugUtils, ash::vk::DebugUtilsMessengerEXT)>,
    surface_fn:   ash::extensions::khr::Surface,
    surface:      ash::vk::SurfaceKHR,
    phys_device:  ash::vk::PhysicalDevice,
    device:       ash::Device,
    queue_families: QueueFamilyIndices,
    graphics_q:   ash::vk::Queue,
    compute_q:    ash::vk::Queue,
    transfer_q:   ash::vk::Queue,
    present_q:    ash::vk::Queue,
}

#[cfg(feature = "vulkan")]
impl Drop for VulkanHandles {
    fn drop(&mut self) {
        unsafe {
            self.device.device_wait_idle().ok();
            self.device.destroy_device(None);
            if let Some((ref du, msg)) = self.debug_utils {
                du.destroy_debug_utils_messenger(msg, None);
            }
            self.surface_fn.destroy_surface(self.surface, None);
            self.instance.destroy_instance(None);
        }
    }
}

// ── public VulkanContext ────────────────────────────────────────────────────

enum ContextInner {
    Headless,
    #[cfg(feature = "vulkan")]
    Real(Box<VulkanHandles>),
}

/// Core Vulkan context.
///
/// Constructed with [`VulkanContext::new`]; pass a live [`winit::window::Window`]
/// so the Vulkan surface can be created immediately.  On headless builds the
/// inner state is a zero-cost stub.
pub struct VulkanContext {
    config: RenderConfig,
    inner:  ContextInner,
}

impl VulkanContext {
    /// Initialise the Vulkan context against `window`.
    ///
    /// Falls back to headless if:
    /// * `config.headless` is `true`, **or**
    /// * the Vulkan loader is not found at runtime, **or**
    /// * `cfg!(not(feature = "vulkan"))`.
    pub fn new(config: RenderConfig) -> RendererResult<Self> {
        Self::new_with_window(config, None::<&winit::window::Window>)
    }

    /// Variant that accepts a live window for surface creation (M1 → M2).
    pub fn new_with_window<W>(config: RenderConfig, window: Option<&W>) -> RendererResult<Self>
    where
        W: raw_window_handle::HasRawWindowHandle + raw_window_handle::HasRawDisplayHandle,
    {
        log::info!("[Renderer] Initialising — {}", config.title);

        if config.width == 0 || config.height == 0 {
            return Err(RendererError::Other("resolution must be non-zero".into()));
        }
        if config.frames_in_flight == 0 || config.frames_in_flight > 4 {
            return Err(RendererError::Other("frames_in_flight must be in [1, 4]".into()));
        }

        #[cfg(feature = "vulkan")]
        if !config.headless {
            if let Some(win) = window {
                match Self::init_vulkan(&config, win) {
                    Ok(handles) => {
                        log::info!("[Renderer] Vulkan context ready (GPU)");
                        return Ok(Self {
                            config,
                            inner: ContextInner::Real(Box::new(handles)),
                        });
                    }
                    Err(e) => {
                        log::warn!("[Renderer] Vulkan init failed ({e}) — falling back to headless");
                    }
                }
            } else {
                log::warn!("[Renderer] No window provided — headless mode");
            }
        }

        log::info!("[Renderer] Running in headless / stub mode");
        Ok(Self { config, inner: ContextInner::Headless })
    }

    /// Human-readable backend description.
    pub fn backend_description(&self) -> &'static str {
        match self.inner {
            ContextInner::Headless => "Headless (stub)",
            #[cfg(feature = "vulkan")]
            ContextInner::Real(_) => "Vulkan (ash)",
        }
    }

    pub fn config(&self) -> &RenderConfig { &self.config }

    pub fn is_headless(&self) -> bool {
        matches!(self.inner, ContextInner::Headless)
    }

    pub fn has_validation_layers(&self) -> bool {
        self.config.validation_layers
    }

    /// Return the logical device, or `None` in headless mode.
    #[cfg(feature = "vulkan")]
    pub fn device(&self) -> Option<&ash::Device> {
        match &self.inner {
            ContextInner::Real(h) => Some(&h.device),
            ContextInner::Headless => None,
        }
    }

    /// Return the ash Instance, or `None` in headless mode.
    #[cfg(feature = "vulkan")]
    pub fn instance(&self) -> Option<&ash::Instance> {
        match &self.inner {
            ContextInner::Real(h) => Some(&h.instance),
            ContextInner::Headless => None,
        }
    }

    /// Return the surface KHR handle, or `None` in headless mode.
    #[cfg(feature = "vulkan")]
    pub fn surface(&self) -> Option<ash::vk::SurfaceKHR> {
        match &self.inner {
            ContextInner::Real(h) => Some(h.surface),
            ContextInner::Headless => None,
        }
    }

    /// Return the surface extension loader, or `None` in headless mode.
    #[cfg(feature = "vulkan")]
    pub fn surface_fn(&self) -> Option<&ash::extensions::khr::Surface> {
        match &self.inner {
            ContextInner::Real(h) => Some(&h.surface_fn),
            ContextInner::Headless => None,
        }
    }

    /// Return the physical device, or null handle in headless mode.
    #[cfg(feature = "vulkan")]
    pub fn physical_device(&self) -> ash::vk::PhysicalDevice {
        match &self.inner {
            ContextInner::Real(h) => h.phys_device,
            ContextInner::Headless => ash::vk::PhysicalDevice::null(),
        }
    }

    /// Return the queue family indices.
    pub fn queue_families(&self) -> Option<QueueFamilyIndices> {
        match &self.inner {
            ContextInner::Headless => None,
            #[cfg(feature = "vulkan")]
            ContextInner::Real(h) => Some(h.queue_families),
        }
    }

    /// Return the graphics queue, or null handle in headless mode.
    #[cfg(feature = "vulkan")]
    pub fn graphics_queue(&self) -> ash::vk::Queue {
        match &self.inner {
            ContextInner::Real(h) => h.graphics_q,
            ContextInner::Headless => ash::vk::Queue::null(),
        }
    }

    /// Return the present queue, or null handle in headless mode.
    #[cfg(feature = "vulkan")]
    pub fn present_queue(&self) -> ash::vk::Queue {
        match &self.inner {
            ContextInner::Real(h) => h.present_q,
            ContextInner::Headless => ash::vk::Queue::null(),
        }
    }
}

// ── Vulkan initialisation helpers ───────────────────────────────────────────

#[cfg(feature = "vulkan")]
impl VulkanContext {
    fn init_vulkan<W>(config: &RenderConfig, window: &W) -> RendererResult<VulkanHandles>
    where
        W: raw_window_handle::HasRawWindowHandle + raw_window_handle::HasRawDisplayHandle,
    {
        use ash::{extensions::{ext, khr}, vk, Entry, Instance};

        // 1. Load Vulkan entry point dynamically (graceful failure if no ICD)
        let entry = unsafe {
            Entry::load().map_err(|e| RendererError::Vulkan(format!("loader: {e}")))?
        };

        // 2. Collect required instance extensions from the window surface
        let surface_exts = unsafe {
            ash_window::enumerate_required_extensions(window.raw_display_handle())
                .map_err(|e| RendererError::Vulkan(format!("surface exts: {e}")))?
        };
        let mut instance_exts: Vec<*const i8> = surface_exts.to_vec();
        if config.validation_layers {
            instance_exts.push(ext::DebugUtils::name().as_ptr());
        }

        // 3. Collect layers
        let mut layer_ptrs: Vec<*const i8> = Vec::new();
        if config.validation_layers {
            if Self::layer_available(&entry, VALIDATION_LAYER) {
                layer_ptrs.push(VALIDATION_LAYER.as_ptr());
            } else {
                log::warn!("[Renderer] VK_LAYER_KHRONOS_validation not available");
            }
        }

        // 4. Create instance
        let app_name    = CString::new(config.title.as_str()).unwrap_or_default();
        let engine_name = CString::new("Atlas Renderer").unwrap();
        let app_info = vk::ApplicationInfo {
            p_application_name: app_name.as_ptr(),
            application_version: vk::make_api_version(0, 0, 1, 0),
            p_engine_name: engine_name.as_ptr(),
            engine_version: vk::make_api_version(0, 0, 1, 0),
            api_version: vk::make_api_version(0, 1, 2, 0),
            ..Default::default()
        };
        let inst_ci = vk::InstanceCreateInfo {
            p_application_info: &app_info,
            enabled_layer_count: layer_ptrs.len() as u32,
            pp_enabled_layer_names: if layer_ptrs.is_empty() { std::ptr::null() } else { layer_ptrs.as_ptr() },
            enabled_extension_count: instance_exts.len() as u32,
            pp_enabled_extension_names: instance_exts.as_ptr(),
            ..Default::default()
        };
        let instance: Instance = unsafe {
            entry.create_instance(&inst_ci, None)
                .map_err(|e| RendererError::Vulkan(format!("create_instance: {e}")))?
        };

        // 5. Debug messenger
        let debug_utils = if config.validation_layers {
            let du = ext::DebugUtils::new(&entry, &instance);
            let msg_ci = vk::DebugUtilsMessengerCreateInfoEXT {
                message_severity:
                    vk::DebugUtilsMessageSeverityFlagsEXT::ERROR
                    | vk::DebugUtilsMessageSeverityFlagsEXT::WARNING,
                message_type:
                    vk::DebugUtilsMessageTypeFlagsEXT::GENERAL
                    | vk::DebugUtilsMessageTypeFlagsEXT::VALIDATION
                    | vk::DebugUtilsMessageTypeFlagsEXT::PERFORMANCE,
                pfn_user_callback: Some(vulkan_debug_callback),
                ..Default::default()
            };
            match unsafe { du.create_debug_utils_messenger(&msg_ci, None) } {
                Ok(m) => Some((du, m)),
                Err(e) => {
                    log::warn!("[Renderer] Debug messenger unavailable: {e}");
                    None
                }
            }
        } else {
            None
        };

        // 6. Surface
        let surface = unsafe {
            ash_window::create_surface(
                &entry, &instance,
                window.raw_display_handle(),
                window.raw_window_handle(),
                None,
            ).map_err(|e| RendererError::Vulkan(format!("create_surface: {e}")))?
        };
        let surface_fn = khr::Surface::new(&entry, &instance);

        // 7. Physical device
        let phys_devices = unsafe {
            instance.enumerate_physical_devices()
                .map_err(|e| RendererError::Vulkan(format!("enumerate_physical_devices: {e}")))?
        };
        if phys_devices.is_empty() {
            return Err(RendererError::NoSuitableGpu);
        }
        let (phys_device, queue_families) = Self::select_physical_device(
            &instance, &surface_fn, surface, &phys_devices,
        )?;

        let phys_props = unsafe { instance.get_physical_device_properties(phys_device) };
        let name = unsafe { CStr::from_ptr(phys_props.device_name.as_ptr()) };
        log::info!("[Renderer] GPU: {:?}  driver={}", name, phys_props.driver_version);

        // 8. Logical device + queues
        let (device, graphics_q, compute_q, transfer_q, present_q) =
            Self::create_device(&instance, phys_device, queue_families)?;

        log::info!(
            "[Renderer] Queues — gfx={} compute={} transfer={} present={}",
            queue_families.graphics, queue_families.compute,
            queue_families.transfer, queue_families.present,
        );

        Ok(VulkanHandles {
            entry, instance, debug_utils, surface_fn, surface,
            phys_device, device, queue_families,
            graphics_q, compute_q, transfer_q, present_q,
        })
    }

    fn layer_available(entry: &ash::Entry, layer: &CStr) -> bool {
        match unsafe { entry.enumerate_instance_layer_properties() } {
            Ok(layers) => layers.iter().any(|l| {
                let n = unsafe { CStr::from_ptr(l.layer_name.as_ptr()) };
                n == layer
            }),
            Err(_) => false,
        }
    }

    /// Pick the best physical device (prefer discrete GPU).
    fn select_physical_device(
        instance:   &ash::Instance,
        surface_fn: &ash::extensions::khr::Surface,
        surface:    ash::vk::SurfaceKHR,
        devices:    &[ash::vk::PhysicalDevice],
    ) -> RendererResult<(ash::vk::PhysicalDevice, QueueFamilyIndices)> {
        use ash::vk;

        let mut best: Option<(ash::vk::PhysicalDevice, QueueFamilyIndices, u32)> = None;

        for &dev in devices {
            let props = unsafe { instance.get_physical_device_properties(dev) };
            let score: u32 = match props.device_type {
                vk::PhysicalDeviceType::DISCRETE_GPU   => 1000,
                vk::PhysicalDeviceType::INTEGRATED_GPU => 100,
                vk::PhysicalDeviceType::VIRTUAL_GPU    => 10,
                _ => 1,
            };

            if let Ok(qf) = Self::find_queue_families(instance, surface_fn, surface, dev) {
                if best.as_ref().map_or(true, |b| score > b.2) {
                    best = Some((dev, qf, score));
                }
            }
        }

        best.map(|(d, q, _)| (d, q)).ok_or(RendererError::NoSuitableGpu)
    }

    fn find_queue_families(
        instance:   &ash::Instance,
        surface_fn: &ash::extensions::khr::Surface,
        surface:    ash::vk::SurfaceKHR,
        dev:        ash::vk::PhysicalDevice,
    ) -> RendererResult<QueueFamilyIndices> {
        use ash::vk;

        let families = unsafe { instance.get_physical_device_queue_family_properties(dev) };

        let mut graphics: Option<u32>  = None;
        let mut compute:  Option<u32>  = None;
        let mut transfer: Option<u32>  = None;
        let mut present:  Option<u32>  = None;

        for (i, fam) in families.iter().enumerate() {
            let i = i as u32;
            if fam.queue_flags.contains(vk::QueueFlags::GRAPHICS) && graphics.is_none() {
                graphics = Some(i);
            }
            // Prefer dedicated compute / transfer queues
            if fam.queue_flags.contains(vk::QueueFlags::COMPUTE)
                && !fam.queue_flags.contains(vk::QueueFlags::GRAPHICS)
            {
                compute = Some(i);
            }
            if fam.queue_flags.contains(vk::QueueFlags::TRANSFER)
                && !fam.queue_flags.contains(vk::QueueFlags::GRAPHICS)
                && !fam.queue_flags.contains(vk::QueueFlags::COMPUTE)
            {
                transfer = Some(i);
            }
            if let Ok(true) = unsafe {
                surface_fn.get_physical_device_surface_support(dev, i, surface)
            } {
                if present.is_none() { present = Some(i); }
            }
        }

        // Fallbacks: if no dedicated compute/transfer, reuse graphics
        let graphics  = graphics.ok_or(RendererError::NoSuitableGpu)?;
        let compute   = compute.unwrap_or(graphics);
        let transfer  = transfer.unwrap_or(graphics);
        let present   = present.ok_or(RendererError::NoSuitableGpu)?;

        Ok(QueueFamilyIndices { graphics, compute, transfer, present })
    }

    fn create_device(
        instance:    &ash::Instance,
        phys_device: ash::vk::PhysicalDevice,
        qf:          QueueFamilyIndices,
    ) -> RendererResult<(ash::Device, ash::vk::Queue, ash::vk::Queue, ash::vk::Queue, ash::vk::Queue)>
    {
        use ash::vk;

        // Deduplicate queue family indices
        let mut unique: Vec<u32> = vec![qf.graphics];
        for &idx in &[qf.compute, qf.transfer, qf.present] {
            if !unique.contains(&idx) { unique.push(idx); }
        }

        let priorities = [1.0_f32];
        let queue_cis: Vec<vk::DeviceQueueCreateInfo> = unique.iter().map(|&family| {
            vk::DeviceQueueCreateInfo {
                queue_family_index: family,
                queue_count: 1,
                p_queue_priorities: priorities.as_ptr(),
                ..Default::default()
            }
        }).collect();

        let swapchain_ext = ash::extensions::khr::Swapchain::name();
        let ext_ptrs = [swapchain_ext.as_ptr()];

        let features = vk::PhysicalDeviceFeatures {
            sampler_anisotropy: vk::TRUE,
            fill_mode_non_solid: vk::TRUE,
            ..Default::default()
        };

        let dev_ci = vk::DeviceCreateInfo {
            queue_create_info_count:    queue_cis.len() as u32,
            p_queue_create_infos:       queue_cis.as_ptr(),
            enabled_extension_count:    ext_ptrs.len() as u32,
            pp_enabled_extension_names: ext_ptrs.as_ptr(),
            p_enabled_features:         &features,
            ..Default::default()
        };

        let device: ash::Device = unsafe {
            instance.create_device(phys_device, &dev_ci, None)
                .map_err(|e| RendererError::Vulkan(format!("create_device: {e}")))?
        };

        let gq = unsafe { device.get_device_queue(qf.graphics,  0) };
        let cq = unsafe { device.get_device_queue(qf.compute,   0) };
        let tq = unsafe { device.get_device_queue(qf.transfer,  0) };
        let pq = unsafe { device.get_device_queue(qf.present,   0) };

        Ok((device, gq, cq, tq, pq))
    }
}

// ── Vulkan debug callback ───────────────────────────────────────────────────

#[cfg(feature = "vulkan")]
unsafe extern "system" fn vulkan_debug_callback(
    severity: ash::vk::DebugUtilsMessageSeverityFlagsEXT,
    _type:    ash::vk::DebugUtilsMessageTypeFlagsEXT,
    data:     *const ash::vk::DebugUtilsMessengerCallbackDataEXT,
    _:        *mut std::ffi::c_void,
) -> ash::vk::Bool32 {
    if data.is_null() { return ash::vk::FALSE; }
    let msg = unsafe { CStr::from_ptr((*data).p_message) };
    if severity.contains(ash::vk::DebugUtilsMessageSeverityFlagsEXT::ERROR) {
        log::error!("[VkVal] {:?}", msg);
    } else {
        log::warn!("[VkVal] {:?}", msg);
    }
    ash::vk::FALSE
}

// ── Tests ───────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn headless_context_creation() {
        let mut cfg = RenderConfig::default();
        cfg.headless = true;
        let ctx = VulkanContext::new(cfg);
        assert!(ctx.is_ok());
        assert!(ctx.unwrap().is_headless());
    }

    #[test]
    fn context_rejects_zero_resolution() {
        let mut cfg = RenderConfig::default();
        cfg.headless = true;
        cfg.width = 0;
        assert!(VulkanContext::new(cfg).is_err());
    }

    #[test]
    fn context_rejects_bad_frames_in_flight() {
        let mut cfg = RenderConfig::default();
        cfg.headless = true;
        cfg.frames_in_flight = 0;
        assert!(VulkanContext::new(cfg).is_err());
    }
}
