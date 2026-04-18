//! Top-level universe container.

use atlas_pcg::{PcgDomain, PcgManager, SeedLevel};
use serde::{Deserialize, Serialize};

use crate::galaxy::{Galaxy, GalaxyConfig};

/// Configuration for universe generation.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct UniverseConfig {
    /// Universe-wide deterministic seed.
    pub seed:          u64,
    /// Number of galaxies to generate.
    pub galaxy_count:  u32,
    /// Version tag (bump to invalidate cached content).
    pub version:       u32,
}

impl Default for UniverseConfig {
    fn default() -> Self {
        Self { seed: 42, galaxy_count: 1, version: 1 }
    }
}

/// A complete procedurally-generated universe.
pub struct Universe {
    pub config:  UniverseConfig,
    pub galaxies: Vec<Galaxy>,
    pcg:         PcgManager,
}

impl Universe {
    /// Generate a universe from the given configuration.
    pub fn generate(config: UniverseConfig) -> Self {
        let mut pcg = PcgManager::new(config.seed);
        pcg.set_version(config.version);

        let mut galaxies = Vec::new();
        for i in 0..config.galaxy_count {
            let mut ctx = pcg.create_context(PcgDomain::Galaxy, SeedLevel::Galaxy, i as u64);
            let galaxy_seed = ctx.rng.next();
            let galaxy_cfg = GalaxyConfig {
                seed:        galaxy_seed,
                system_count: ctx.rng.next_int_range(50, 200) as u32,
                arms:        ctx.rng.next_int_range(2, 6) as u32,
                radius_ly:   ctx.rng.next_float_range(10_000.0, 100_000.0),
            };
            galaxies.push(Galaxy::generate(galaxy_cfg));
        }

        Self { config, galaxies, pcg }
    }

    /// Universe seed.
    pub fn seed(&self) -> u64 { self.config.seed }

    /// PCG manager — allows creating deterministic contexts for any domain.
    pub fn pcg(&self) -> &PcgManager { &self.pcg }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn generate_single_galaxy() {
        let u = Universe::generate(UniverseConfig { seed: 1, galaxy_count: 1, version: 1 });
        assert_eq!(u.galaxies.len(), 1);
    }

    #[test]
    fn generate_deterministic() {
        let a = Universe::generate(UniverseConfig::default());
        let b = Universe::generate(UniverseConfig::default());
        assert_eq!(a.galaxies.len(), b.galaxies.len());
        assert_eq!(a.galaxies[0].systems.len(), b.galaxies[0].systems.len());
    }
}
