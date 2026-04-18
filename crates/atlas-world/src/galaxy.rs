//! Galaxy layout generation.

use atlas_pcg::{PcgDomain, PcgManager, SeedLevel};
use serde::{Deserialize, Serialize};

use crate::star_system::{StarSystem, StarSystemConfig};

/// Configuration for a galaxy.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct GalaxyConfig {
    pub seed:         u64,
    pub system_count: u32,
    pub arms:         u32,
    pub radius_ly:    f32,
}

impl Default for GalaxyConfig {
    fn default() -> Self {
        Self { seed: 42, system_count: 100, arms: 4, radius_ly: 50_000.0 }
    }
}

/// A cluster of stars used for spatial partitioning.
#[derive(Clone, Debug)]
pub struct StarCluster {
    pub x: f32,
    pub y: f32,
    pub z: f32,
    pub density: f32,
}

/// A procedurally-generated galaxy.
pub struct Galaxy {
    pub config:   GalaxyConfig,
    pub systems:  Vec<StarSystem>,
    pub clusters: Vec<StarCluster>,
}

impl Galaxy {
    /// Generate a galaxy from the given configuration.
    pub fn generate(config: GalaxyConfig) -> Self {
        let mut pcg = PcgManager::new(config.seed);

        // Generate spiral arm clusters
        let clusters = Self::generate_clusters(&config, &mut pcg);

        // Generate star systems
        let systems = Self::generate_systems(&config, &mut pcg);

        Self { config, systems, clusters }
    }

    fn generate_clusters(config: &GalaxyConfig, pcg: &mut PcgManager) -> Vec<StarCluster> {
        let ctx = pcg.create_context(PcgDomain::Galaxy, SeedLevel::Sector, 0xC0DE_CAFE);
        let arm_count = config.arms;
        let mut clusters = Vec::new();

        for arm in 0..arm_count {
            let arm_ctx = ctx.child(arm as u64);
            let mut arm_rng = arm_ctx.rng;
            let points_per_arm = 20u32;
            for p in 0..points_per_arm {
                let t = p as f32 / points_per_arm as f32;
                let angle = (arm as f32 / arm_count as f32) * std::f32::consts::TAU
                    + t * std::f32::consts::TAU * 2.0;
                let dist = t * config.radius_ly;
                let spread = arm_rng.next_float_range(0.0, config.radius_ly * 0.08);
                let x = angle.cos() * dist + arm_rng.next_float_range(-spread, spread);
                let y = arm_rng.next_float_range(-500.0, 500.0);
                let z = angle.sin() * dist + arm_rng.next_float_range(-spread, spread);
                clusters.push(StarCluster { x, y, z, density: arm_rng.next_float() });
            }
        }
        clusters
    }

    fn generate_systems(config: &GalaxyConfig, pcg: &mut PcgManager) -> Vec<StarSystem> {
        let mut systems = Vec::new();
        for i in 0..config.system_count {
            let mut ctx = pcg.create_context(PcgDomain::System, SeedLevel::System, i as u64);
            let system_seed = ctx.rng.next();
            let planet_count = ctx.rng.next_int_range(0, 8) as u32;
            let asteroid_belts = ctx.rng.next_int_range(0, 3) as u32;
            let sys_cfg = StarSystemConfig {
                seed:            system_seed,
                planet_count,
                asteroid_belts,
                position:        [
                    ctx.rng.next_float_range(-config.radius_ly, config.radius_ly),
                    ctx.rng.next_float_range(-1_000.0, 1_000.0),
                    ctx.rng.next_float_range(-config.radius_ly, config.radius_ly),
                ],
            };
            systems.push(StarSystem::generate(sys_cfg));
        }
        systems
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn galaxy_has_systems() {
        let g = Galaxy::generate(GalaxyConfig { seed: 7, system_count: 10, arms: 2, radius_ly: 1000.0 });
        assert_eq!(g.systems.len(), 10);
    }

    #[test]
    fn galaxy_deterministic() {
        let cfg = GalaxyConfig::default();
        let a = Galaxy::generate(cfg.clone());
        let b = Galaxy::generate(cfg);
        assert_eq!(a.systems.len(), b.systems.len());
    }
}
