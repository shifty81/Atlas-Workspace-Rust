//! Asteroid belt and field generation.

use atlas_pcg::{PcgDomain, PcgManager, SeedLevel};
use serde::{Deserialize, Serialize};

/// Configuration for an asteroid belt / field.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct AsteroidConfig {
    pub seed:            u64,
    pub asteroid_count:  u32,
    pub inner_radius_au: f32,
    pub outer_radius_au: f32,
}

impl Default for AsteroidConfig {
    fn default() -> Self {
        Self { seed: 42, asteroid_count: 100, inner_radius_au: 2.2, outer_radius_au: 3.2 }
    }
}

/// A single asteroid.
#[derive(Clone, Debug)]
pub struct Asteroid {
    pub id:          u32,
    /// Orbital angle in radians.
    pub angle:       f32,
    /// Orbital distance in AU.
    pub distance_au: f32,
    /// Radius in km.
    pub radius_km:   f32,
    /// Composition richness (0–1).
    pub richness:    f32,
    /// Metal content index (0–1, higher = more valuable ore).
    pub metal:       f32,
    /// Ice content (0–1).
    pub ice:         f32,
}

/// A complete asteroid belt.
pub struct AsteroidBelt {
    pub config:     AsteroidConfig,
    pub asteroids:  Vec<Asteroid>,
}

impl AsteroidBelt {
    pub fn generate(config: AsteroidConfig) -> Self {
        let pcg = PcgManager::new(config.seed);
        let ctx = pcg.create_context(PcgDomain::Asteroid, SeedLevel::Object, 0xBEE1_7BE1); // "BELT"

        let mut asteroids = Vec::with_capacity(config.asteroid_count as usize);
        for i in 0..config.asteroid_count {
            let child = ctx.child(i as u64);
            let mut rng = child.rng;
            asteroids.push(Asteroid {
                id:          i + 1,
                angle:       rng.next_float_range(0.0, std::f32::consts::TAU),
                distance_au: rng.next_float_range(config.inner_radius_au, config.outer_radius_au),
                radius_km:   rng.next_float_range(0.1, 500.0),
                richness:    rng.next_float(),
                metal:       rng.next_float(),
                ice:         rng.next_float(),
            });
        }

        Self { config, asteroids }
    }

    /// Total count of asteroids.
    pub fn count(&self) -> usize { self.asteroids.len() }

    /// Asteroids with metal content above the threshold.
    pub fn rich_metal_asteroids(&self, min_metal: f32) -> Vec<&Asteroid> {
        self.asteroids.iter().filter(|a| a.metal >= min_metal).collect()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn belt_count_correct() {
        let belt = AsteroidBelt::generate(AsteroidConfig { seed: 5, asteroid_count: 50, ..Default::default() });
        assert_eq!(belt.count(), 50);
    }

    #[test]
    fn belt_deterministic() {
        let cfg = AsteroidConfig::default();
        let a = AsteroidBelt::generate(cfg.clone());
        let b = AsteroidBelt::generate(cfg);
        assert_eq!(a.count(), b.count());
        assert!((a.asteroids[0].distance_au - b.asteroids[0].distance_au).abs() < 1e-4);
    }
}
