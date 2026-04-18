//! Star system generation.

use atlas_pcg::{PcgDomain, PcgManager, SeedLevel};
use serde::{Deserialize, Serialize};

use crate::asteroid::{AsteroidBelt, AsteroidConfig};
use crate::planet::{Planet, PlanetConfig};

/// Star spectral type.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub enum StarType {
    O, B, A, F, G, K, M, // main sequence
    WhiteDwarf,
    NeutronStar,
    BlackHole,
}

impl StarType {
    /// Relative luminosity multiplier.
    pub fn luminosity(self) -> f32 {
        match self {
            Self::O          => 100_000.0,
            Self::B          => 10_000.0,
            Self::A          => 80.0,
            Self::F          => 4.0,
            Self::G          => 1.0,
            Self::K          => 0.4,
            Self::M          => 0.04,
            Self::WhiteDwarf => 0.001,
            Self::NeutronStar => 0.00001,
            Self::BlackHole  => 0.0,
        }
    }
}

/// Star system configuration.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct StarSystemConfig {
    pub seed:           u64,
    pub planet_count:   u32,
    pub asteroid_belts: u32,
    pub position:       [f32; 3],
}

impl Default for StarSystemConfig {
    fn default() -> Self {
        Self { seed: 42, planet_count: 3, asteroid_belts: 1, position: [0.0; 3] }
    }
}

/// A procedurally-generated star system.
pub struct StarSystem {
    pub config:          StarSystemConfig,
    pub star_type:       StarType,
    pub star_mass_solar: f32,
    pub planets:         Vec<Planet>,
    pub asteroid_belts:  Vec<AsteroidBelt>,
}

impl StarSystem {
    pub fn generate(config: StarSystemConfig) -> Self {
        let pcg = PcgManager::new(config.seed);

        // Determine star type
        let mut star_ctx = pcg.create_context(PcgDomain::System, SeedLevel::Object, 0x5741_5253); // "STAR"
        let star_type = Self::roll_star_type(&mut star_ctx.rng);
        let star_mass = star_ctx.rng.next_float_range(0.08, 150.0);

        // Generate planets
        let mut planets = Vec::new();
        for i in 0..config.planet_count {
            let mut ctx = pcg.create_context(PcgDomain::Planet, SeedLevel::Object, i as u64);
            let planet_seed = ctx.rng.next();
            let orbital_radius_au = ctx.rng.next_float_range(0.1, 40.0);
            let planet_cfg = PlanetConfig {
                seed:             planet_seed,
                orbital_radius_au,
            };
            planets.push(Planet::generate(planet_cfg));
        }

        // Generate asteroid belts
        let mut asteroid_belts = Vec::new();
        for i in 0..config.asteroid_belts {
            let mut ctx = pcg.create_context(PcgDomain::Asteroid, SeedLevel::Object, i as u64);
            let belt_seed = ctx.rng.next();
            let belt_cfg = AsteroidConfig {
                seed:            belt_seed,
                asteroid_count:  ctx.rng.next_int_range(20, 500) as u32,
                inner_radius_au: ctx.rng.next_float_range(1.0, 5.0),
                outer_radius_au: ctx.rng.next_float_range(5.0, 15.0),
            };
            asteroid_belts.push(AsteroidBelt::generate(belt_cfg));
        }

        Self { config, star_type, star_mass_solar: star_mass, planets, asteroid_belts }
    }

    fn roll_star_type(rng: &mut atlas_pcg::DeterministicRng) -> StarType {
        match rng.next_u32(100) {
            0..=3   => StarType::M,     // 4%  (bias towards common types last for match ordering)
            4..=7   => StarType::K,
            8..=17  => StarType::G,
            18..=32 => StarType::F,
            33..=52 => StarType::A,
            53..=72 => StarType::B,
            73..=92 => StarType::O,
            93..=95 => StarType::WhiteDwarf,
            96..=98 => StarType::NeutronStar,
            _       => StarType::BlackHole,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn system_has_correct_planet_count() {
        let cfg = StarSystemConfig { seed: 42, planet_count: 5, asteroid_belts: 0, position: [0.0; 3] };
        let sys = StarSystem::generate(cfg);
        assert_eq!(sys.planets.len(), 5);
    }

    #[test]
    fn system_deterministic() {
        let cfg = StarSystemConfig::default();
        let a = StarSystem::generate(cfg.clone());
        let b = StarSystem::generate(cfg);
        assert_eq!(a.star_type, b.star_type);
        assert!((a.star_mass_solar - b.star_mass_solar).abs() < 1e-4);
    }
}
