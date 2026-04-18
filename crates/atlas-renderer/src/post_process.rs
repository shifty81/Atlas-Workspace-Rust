#[derive(Debug, Clone, PartialEq)]
pub enum PostProcessEffect { Bloom, ToneMapping, Fxaa, Vignette, ChromaticAberration, FilmGrain }

#[derive(Debug, Clone, PartialEq)]
pub enum ToneMapOperator { Reinhard, Aces, Uncharted2, Filmic }

#[derive(Debug, Clone)]
pub struct BloomSettings {
    pub threshold: f32,
    pub intensity: f32,
    pub radius: f32,
    pub mip_count: u32,
    pub enabled: bool,
}

impl Default for BloomSettings {
    fn default() -> Self {
        Self { threshold: 1.0, intensity: 1.0, radius: 0.005, mip_count: 5, enabled: true }
    }
}

#[derive(Debug, Clone)]
pub struct ToneMappingSettings {
    pub op: ToneMapOperator,
    pub exposure: f32,
    pub gamma: f32,
    pub white_point: f32,
    pub enabled: bool,
}

impl Default for ToneMappingSettings {
    fn default() -> Self {
        Self { op: ToneMapOperator::Aces, exposure: 1.0, gamma: 2.2, white_point: 4.0, enabled: true }
    }
}

#[derive(Debug, Clone)]
pub struct PostProcessSettings {
    pub bloom: BloomSettings,
    pub tone_mapping: ToneMappingSettings,
    pub fxaa_enabled: bool,
    pub vignette_intensity: f32,
    pub chromatic_aberration_intensity: f32,
    pub film_grain_intensity: f32,
}

impl Default for PostProcessSettings {
    fn default() -> Self {
        Self {
            bloom: BloomSettings::default(),
            tone_mapping: ToneMappingSettings::default(),
            fxaa_enabled: true,
            vignette_intensity: 0.3,
            chromatic_aberration_intensity: 0.0,
            film_grain_intensity: 0.0,
        }
    }
}

#[derive(Debug, Default)]
pub struct PostProcessPipeline {
    settings: PostProcessSettings,
    width: u32,
    height: u32,
    output_texture_id: u32,
    initialized: bool,
}

impl PostProcessPipeline {
    pub fn new() -> Self { Self::default() }

    pub fn init(&mut self, w: u32, h: u32) {
        self.width = w;
        self.height = h;
        self.initialized = true;
    }

    pub fn shutdown(&mut self) { self.initialized = false; }
    pub fn resize(&mut self, w: u32, h: u32) { self.width = w; self.height = h; }

    pub fn set_settings(&mut self, settings: PostProcessSettings) { self.settings = settings; }
    pub fn settings(&self) -> &PostProcessSettings { &self.settings }

    pub fn is_effect_enabled(&self, effect: &PostProcessEffect) -> bool {
        match effect {
            PostProcessEffect::Bloom => self.settings.bloom.enabled,
            PostProcessEffect::ToneMapping => self.settings.tone_mapping.enabled,
            PostProcessEffect::Fxaa => self.settings.fxaa_enabled,
            PostProcessEffect::Vignette => self.settings.vignette_intensity > 0.0,
            PostProcessEffect::ChromaticAberration => self.settings.chromatic_aberration_intensity > 0.0,
            PostProcessEffect::FilmGrain => self.settings.film_grain_intensity > 0.0,
        }
    }

    pub fn set_effect_enabled(&mut self, effect: PostProcessEffect, enabled: bool) {
        match effect {
            PostProcessEffect::Bloom => self.settings.bloom.enabled = enabled,
            PostProcessEffect::ToneMapping => self.settings.tone_mapping.enabled = enabled,
            PostProcessEffect::Fxaa => self.settings.fxaa_enabled = enabled,
            PostProcessEffect::Vignette => {
                if !enabled { self.settings.vignette_intensity = 0.0; }
            },
            PostProcessEffect::ChromaticAberration => {
                if !enabled { self.settings.chromatic_aberration_intensity = 0.0; }
            },
            PostProcessEffect::FilmGrain => {
                if !enabled { self.settings.film_grain_intensity = 0.0; }
            },
        }
    }

    pub fn execute(&self) {
        log::debug!("execute post-process");
    }

    pub fn output_texture_id(&self) -> u32 { self.output_texture_id }
    pub fn is_initialized(&self) -> bool { self.initialized }

    pub fn effect_count(&self) -> u32 {
        let effects = [
            PostProcessEffect::Bloom, PostProcessEffect::ToneMapping,
            PostProcessEffect::Fxaa, PostProcessEffect::Vignette,
            PostProcessEffect::ChromaticAberration, PostProcessEffect::FilmGrain,
        ];
        effects.iter().filter(|e| self.is_effect_enabled(e)).count() as u32
    }
}
