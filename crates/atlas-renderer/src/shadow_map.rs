#[derive(Debug, Clone)]
pub struct ShadowCascade {
    pub near_plane: f32,
    pub far_plane: f32,
    pub resolution: u32,
    pub view_matrix: [f32; 16],
    pub proj_matrix: [f32; 16],
    pub id: u32,
}

#[derive(Debug, Clone, Default)]
pub struct LightDirection {
    pub x: f32,
    pub y: f32,
    pub z: f32,
}

#[derive(Debug, Clone)]
pub struct ShadowMapConfig {
    pub cascade_count: u32,
    pub base_resolution: u32,
    pub bias: f32,
    pub normal_bias: f32,
    pub soft_shadows: bool,
    pub pcf_samples: u32,
}

impl Default for ShadowMapConfig {
    fn default() -> Self {
        Self { cascade_count: 3, base_resolution: 1024, bias: 0.005, normal_bias: 0.02, soft_shadows: true, pcf_samples: 4 }
    }
}

fn identity_mat() -> [f32; 16] {
    [1.0,0.0,0.0,0.0, 0.0,1.0,0.0,0.0, 0.0,0.0,1.0,0.0, 0.0,0.0,0.0,1.0]
}

fn ortho_mat(near: f32, far: f32) -> [f32; 16] {
    let mut m = identity_mat();
    m[10] = -2.0 / (far - near);
    m[14] = -(far + near) / (far - near);
    m
}

#[derive(Debug, Default)]
pub struct ShadowMap {
    config: ShadowMapConfig,
    cascades: Vec<ShadowCascade>,
    light_direction: LightDirection,
    initialized: bool,
    bound: bool,
}

impl ShadowMap {
    pub fn new() -> Self { Self::default() }

    pub fn init(&mut self, config: ShadowMapConfig) {
        let count = config.cascade_count as usize;
        self.config = config;
        self.cascades = (0..count).map(|i| ShadowCascade {
            near_plane: 0.0, far_plane: 0.0,
            resolution: self.config.base_resolution,
            view_matrix: identity_mat(),
            proj_matrix: identity_mat(),
            id: i as u32,
        }).collect();
        self.initialized = true;
    }

    pub fn shutdown(&mut self) { self.initialized = false; self.bound = false; }

    pub fn set_light_direction(&mut self, x: f32, y: f32, z: f32) {
        self.light_direction = LightDirection { x, y, z };
    }
    pub fn light_direction(&self) -> LightDirection { self.light_direction.clone() }

    pub fn update_cascades(&mut self, camera_near: f32, camera_far: f32, _fov_deg: f32, _aspect_ratio: f32) {
        let count = self.config.cascade_count as usize;
        let near = camera_near;
        let far = camera_far;
        let lambda = 0.5f32;
        let mut prev_near = near;
        for i in 0..count {
            let ratio = (i + 1) as f32 / count as f32;
            let log_split = near * (far / near).powf(ratio);
            let uniform_split = near + (far - near) * ratio;
            let split = lambda * log_split + (1.0 - lambda) * uniform_split;
            if let Some(cascade) = self.cascades.get_mut(i) {
                cascade.near_plane = prev_near;
                cascade.far_plane = split;
                cascade.proj_matrix = ortho_mat(prev_near, split);
            }
            prev_near = split;
        }
    }

    pub fn get_cascade(&self, index: u32) -> Option<&ShadowCascade> {
        self.cascades.get(index as usize)
    }

    pub fn cascade_count(&self) -> u32 { self.cascades.len() as u32 }
    pub fn config(&self) -> &ShadowMapConfig { &self.config }
    pub fn bind_for_writing(&mut self, _cascade_index: u32) { self.bound = true; }
    pub fn bind_for_reading(&mut self) { self.bound = true; }
    pub fn unbind(&mut self) { self.bound = false; }
    pub fn is_initialized(&self) -> bool { self.initialized }
    pub fn is_bound(&self) -> bool { self.bound }
}
