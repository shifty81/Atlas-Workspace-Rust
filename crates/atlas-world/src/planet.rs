//! Planet surface, atmosphere, and biome generation.

use atlas_pcg::{PcgDomain, PcgManager, SeedLevel, TerrainConfig, TerrainGenerator};
use serde::{Deserialize, Serialize};

/// Planet type classification.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub enum PlanetType {
    Rocky,
    Oceanic,
    Desert,
    Arctic,
    Volcanic,
    GasGiant,
    IceGiant,
    Barren,
}

/// Surface biome.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub enum Biome {
    Tundra,
    Boreal,
    Temperate,
    Tropical,
    Desert,
    Ocean,
    Volcanic,
    Arctic,
    None,
}

/// Atmospheric composition.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Atmosphere {
    pub pressure_atm:  f32,
    pub oxygen_pct:    f32,
    pub nitrogen_pct:  f32,
    pub co2_pct:       f32,
    pub temperature_k: f32,
    pub breathable:    bool,
}

impl Atmosphere {
    pub fn thin() -> Self {
        Self { pressure_atm: 0.01, oxygen_pct: 0.0, nitrogen_pct: 95.0, co2_pct: 5.0,
               temperature_k: 200.0, breathable: false }
    }

    pub fn earth_like() -> Self {
        Self { pressure_atm: 1.0, oxygen_pct: 21.0, nitrogen_pct: 78.0, co2_pct: 0.04,
               temperature_k: 288.0, breathable: true }
    }

    pub fn toxic() -> Self {
        Self { pressure_atm: 90.0, oxygen_pct: 0.0, nitrogen_pct: 3.5, co2_pct: 96.5,
               temperature_k: 735.0, breathable: false }
    }
}

/// Planet generation configuration.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct PlanetConfig {
    pub seed:             u64,
    pub orbital_radius_au: f32,
}

impl Default for PlanetConfig {
    fn default() -> Self {
        Self { seed: 42, orbital_radius_au: 1.0 }
    }
}

/// A procedurally-generated planet.
pub struct Planet {
    pub config:            PlanetConfig,
    pub planet_type:       PlanetType,
    pub radius_km:         f32,
    pub mass_earth:        f32,
    pub gravity_g:         f32,
    pub biome:             Biome,
    pub atmosphere:        Atmosphere,
    pub has_rings:         bool,
    pub moon_count:        u32,
    pub surface_height_map: Option<Vec<f32>>, // lazy-loaded
}

impl Planet {
    /// Generate a planet from configuration.
    pub fn generate(config: PlanetConfig) -> Self {
        let pcg = PcgManager::new(config.seed);
        let mut ctx = pcg.create_context(PcgDomain::Planet, SeedLevel::Object, 0xBA5E_CAFE); // "BASE"

        let planet_type = Self::roll_planet_type(&mut ctx.rng, config.orbital_radius_au);
        let radius_km   = ctx.rng.next_float_range(1_000.0, 80_000.0);
        let radius_ratio = radius_km / 6_371.0;
        let mass_earth  = radius_ratio.powi(2) * ctx.rng.next_float_range(0.5, 2.0);
        let gravity_g   = mass_earth / radius_ratio.powi(2);
        let biome       = Self::biome_for(planet_type, &mut ctx.rng);
        let atmosphere  = Self::gen_atmosphere(planet_type, &mut ctx.rng);
        let has_rings   = ctx.rng.next_bool(0.1);
        let moon_count  = ctx.rng.next_u32(5);

        Self {
            config, planet_type, radius_km, mass_earth, gravity_g,
            biome, atmosphere, has_rings, moon_count,
            surface_height_map: None,
        }
    }

    /// Lazily generate the surface heightmap (expensive — call only when needed).
    pub fn generate_heightmap(&mut self) {
        let gen = TerrainGenerator::new(TerrainConfig {
            width:      256,
            height:     256,
            cell_size:  self.radius_km * 0.001,
            max_height: self.radius_km * 0.05,
            octaves:    8,
            frequency:  2.0,
            persistence: 0.5,
            lacunarity:  2.0,
            seed:       self.config.seed,
        });
        let hm = gen.generate();
        self.surface_height_map = Some(hm.data);
    }

    // ── Private ──────────────────────────────────────────────────────────

    fn roll_planet_type(rng: &mut atlas_pcg::DeterministicRng, orbital_au: f32) -> PlanetType {
        // Rough habitable zone: 0.7–1.5 AU
        if orbital_au < 0.3 {
            return PlanetType::Volcanic;
        }
        if orbital_au > 5.0 {
            return if rng.next_bool(0.5) { PlanetType::GasGiant } else { PlanetType::IceGiant };
        }
        match rng.next_u32(8) {
            0 => PlanetType::Rocky,
            1 => PlanetType::Oceanic,
            2 => PlanetType::Desert,
            3 => PlanetType::Arctic,
            4 => PlanetType::Volcanic,
            5 => PlanetType::GasGiant,
            6 => PlanetType::IceGiant,
            _ => PlanetType::Barren,
        }
    }

    fn biome_for(planet_type: PlanetType, rng: &mut atlas_pcg::DeterministicRng) -> Biome {
        match planet_type {
            PlanetType::Rocky    => [Biome::Tundra, Biome::Desert, Biome::Temperate][rng.next_u32(3) as usize],
            PlanetType::Oceanic  => Biome::Ocean,
            PlanetType::Desert   => Biome::Desert,
            PlanetType::Arctic   => Biome::Arctic,
            PlanetType::Volcanic => Biome::Volcanic,
            PlanetType::Barren   => Biome::None,
            _                    => Biome::None,
        }
    }

    fn gen_atmosphere(planet_type: PlanetType, rng: &mut atlas_pcg::DeterministicRng) -> Atmosphere {
        match planet_type {
            PlanetType::Rocky   => {
                if rng.next_bool(0.15) { Atmosphere::earth_like() }
                else { Atmosphere::thin() }
            }
            PlanetType::Oceanic  => Atmosphere::earth_like(),
            PlanetType::Volcanic => Atmosphere::toxic(),
            PlanetType::GasGiant | PlanetType::IceGiant => Atmosphere {
                pressure_atm:  rng.next_float_range(10.0, 1000.0),
                oxygen_pct:    0.0,
                nitrogen_pct:  rng.next_float_range(5.0, 20.0),
                co2_pct:       rng.next_float_range(0.0, 5.0),
                temperature_k: rng.next_float_range(50.0, 200.0),
                breathable:    false,
            },
            _ => Atmosphere::thin(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn planet_generation_deterministic() {
        let cfg = PlanetConfig::default();
        let a = Planet::generate(cfg.clone());
        let b = Planet::generate(cfg);
        assert_eq!(a.planet_type, b.planet_type);
        assert!((a.radius_km - b.radius_km).abs() < 1e-2);
    }

    #[test]
    fn heightmap_range() {
        let mut planet = Planet::generate(PlanetConfig { seed: 77, orbital_radius_au: 1.0 });
        planet.generate_heightmap();
        let hm = planet.surface_height_map.as_ref().unwrap();
        let max_h = planet.radius_km * 0.05;
        for &h in hm {
            assert!(h >= 0.0 && h <= max_h + 0.1, "h={}", h);
        }
    }
}
