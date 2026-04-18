//! Planetary base zone layout generator.
//!
//! Rust port of the C++ `atlas::procedural::PlanetaryBase`.

use crate::rng::DeterministicRng;

/// Zone types in a planetary base.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[repr(u8)]
pub enum BaseZoneType {
    Landing       = 0,
    Habitat       = 1,
    Mining        = 2,
    Refinery      = 3,
    Defense       = 4,
    Research      = 5,
    Power         = 6,
    Communication = 7,
    Storage       = 8,
    Medical       = 9,
}

impl BaseZoneType {
    pub const COUNT: usize = 10;
    pub fn all() -> [Self; 10] {
        [
            Self::Landing, Self::Habitat, Self::Mining, Self::Refinery,
            Self::Defense, Self::Research, Self::Power, Self::Communication,
            Self::Storage, Self::Medical,
        ]
    }
}

/// A single zone in a planetary base.
#[derive(Clone, Debug)]
pub struct BaseZone {
    pub id:          u32,
    pub zone_type:   BaseZoneType,
    pub x:           f32,
    pub y:           f32,
    pub radius:      f32,
    pub level:       u8,
    pub operational: bool,
    pub name:        String,
}

impl BaseZone {
    /// Area of the zone (π r²).
    pub fn area(&self) -> f32 {
        std::f32::consts::PI * self.radius * self.radius
    }
}

/// Configuration for base generation.
#[derive(Clone, Debug)]
pub struct PlanetaryBaseConfig {
    pub max_zones:        u32,
    pub base_radius:      f32,
    pub min_zone_spacing: f32,
    pub seed:             u64,
}

impl Default for PlanetaryBaseConfig {
    fn default() -> Self {
        Self {
            max_zones:        20,
            base_radius:      200.0,
            min_zone_spacing: 5.0,
            seed:             42,
        }
    }
}

/// Manages a collection of zones forming a planetary base.
pub struct PlanetaryBase {
    config:  PlanetaryBaseConfig,
    zones:   Vec<BaseZone>,
    next_id: u32,
}

impl PlanetaryBase {
    pub fn new() -> Self {
        Self { config: PlanetaryBaseConfig::default(), zones: Vec::new(), next_id: 1 }
    }

    /// Initialise with a configuration, discarding existing zones.
    pub fn init(&mut self, cfg: PlanetaryBaseConfig) {
        self.config = cfg;
        self.zones.clear();
        self.next_id = 1;
    }

    /// Add a zone if placement is valid.  Returns the zone ID, or `0`.
    pub fn add_zone(&mut self, zone_type: BaseZoneType, x: f32, y: f32, radius: f32) -> u32 {
        if !self.is_zone_spacing_valid(x, y, radius) {
            return 0;
        }
        // Also check that it fits inside the base radius
        let dist = (x * x + y * y).sqrt();
        if dist + radius > self.config.base_radius {
            return 0;
        }
        let id = self.next_id;
        self.next_id += 1;
        self.zones.push(BaseZone {
            id, zone_type, x, y, radius,
            level: 1, operational: true,
            name: format!("{:?}-{}", zone_type, id),
        });
        id
    }

    /// Remove a zone by ID.
    pub fn remove_zone(&mut self, zone_id: u32) {
        self.zones.retain(|z| z.id != zone_id);
    }

    /// Look up a zone by ID.
    pub fn get_zone(&self, zone_id: u32) -> Option<&BaseZone> {
        self.zones.iter().find(|z| z.id == zone_id)
    }

    /// Mutable zone lookup.
    pub fn get_zone_mut(&mut self, zone_id: u32) -> Option<&mut BaseZone> {
        self.zones.iter_mut().find(|z| z.id == zone_id)
    }

    pub fn zone_count(&self) -> usize { self.zones.len() }

    /// IDs of all zones of the given type.
    pub fn find_zones_by_type(&self, zone_type: BaseZoneType) -> Vec<u32> {
        self.zones.iter()
            .filter(|z| z.zone_type == zone_type)
            .map(|z| z.id)
            .collect()
    }

    /// Upgrade a zone's level (max 5).
    pub fn upgrade_zone(&mut self, zone_id: u32) {
        if let Some(z) = self.get_zone_mut(zone_id) {
            if z.level < 5 {
                z.level += 1;
            }
        }
    }

    pub fn config(&self) -> &PlanetaryBaseConfig { &self.config }

    /// True if the base has at least Landing + Power + Habitat zones.
    pub fn has_required_zones(&self) -> bool {
        let has = |t| self.zones.iter().any(|z| z.zone_type == t);
        has(BaseZoneType::Landing) && has(BaseZoneType::Power) && has(BaseZoneType::Habitat)
    }

    /// Check whether a new zone can be placed without overlap.
    pub fn is_zone_spacing_valid(&self, x: f32, y: f32, radius: f32) -> bool {
        let min_dist = self.config.min_zone_spacing;
        for z in &self.zones {
            let dx = z.x - x;
            let dy = z.y - y;
            let dist = (dx * dx + dy * dy).sqrt();
            if dist < z.radius + radius + min_dist {
                return false;
            }
        }
        true
    }

    /// Procedurally generate a base layout from the given seed.
    pub fn generate(&mut self, seed: u64) {
        self.clear();
        self.config.seed = seed;
        let mut rng = DeterministicRng::new(seed);

        let zone_types = BaseZoneType::all();
        let max = self.config.max_zones;
        let base_r = self.config.base_radius;

        // Always place landing and power first
        for &required in &[BaseZoneType::Landing, BaseZoneType::Power, BaseZoneType::Habitat] {
            let radius = rng.next_float_range(8.0, 20.0);
            for _ in 0..100 {
                let angle = rng.next_float_range(0.0, std::f32::consts::TAU);
                let dist  = rng.next_float_range(0.0, base_r - radius - 5.0);
                let x = angle.cos() * dist;
                let y = angle.sin() * dist;
                if self.add_zone(required, x, y, radius) != 0 {
                    break;
                }
            }
        }

        // Fill remaining slots with random zone types
        let remaining = max as usize - self.zones.len();
        for _ in 0..remaining {
            let t_idx = rng.next_u32(BaseZoneType::COUNT as u32) as usize;
            let t = zone_types[t_idx];
            let radius = rng.next_float_range(5.0, 18.0);
            for _ in 0..50 {
                let angle = rng.next_float_range(0.0, std::f32::consts::TAU);
                let dist  = rng.next_float_range(0.0, base_r - radius - 5.0);
                let x = angle.cos() * dist;
                let y = angle.sin() * dist;
                if self.add_zone(t, x, y, radius) != 0 {
                    break;
                }
            }
        }
    }

    /// Remove all zones.
    pub fn clear(&mut self) {
        self.zones.clear();
        self.next_id = 1;
    }

    pub fn operational_zone_count(&self) -> usize {
        self.zones.iter().filter(|z| z.operational).count()
    }

    pub fn total_area(&self) -> f32 {
        self.zones.iter().map(|z| z.area()).sum()
    }
}

impl Default for PlanetaryBase {
    fn default() -> Self { Self::new() }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn add_zone_valid() {
        let mut base = PlanetaryBase::new();
        let id = base.add_zone(BaseZoneType::Landing, 0.0, 0.0, 10.0);
        assert_ne!(id, 0);
        assert_eq!(base.zone_count(), 1);
    }

    #[test]
    fn overlap_rejected() {
        let mut base = PlanetaryBase::new();
        base.add_zone(BaseZoneType::Landing, 0.0, 0.0, 20.0);
        let id2 = base.add_zone(BaseZoneType::Habitat, 1.0, 1.0, 20.0);
        assert_eq!(id2, 0); // should be rejected due to overlap
    }

    #[test]
    fn generate_has_required_zones() {
        let mut base = PlanetaryBase::new();
        base.generate(42);
        assert!(base.has_required_zones());
    }

    #[test]
    fn generate_deterministic() {
        let mut a = PlanetaryBase::new();
        let mut b = PlanetaryBase::new();
        a.generate(999);
        b.generate(999);
        assert_eq!(a.zone_count(), b.zone_count());
    }

    #[test]
    fn upgrade_zone_max_5() {
        let mut base = PlanetaryBase::new();
        let id = base.add_zone(BaseZoneType::Power, 50.0, 0.0, 10.0);
        for _ in 0..10 {
            base.upgrade_zone(id);
        }
        assert_eq!(base.get_zone(id).unwrap().level, 5);
    }
}
