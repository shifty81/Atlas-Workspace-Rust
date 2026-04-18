//! PCG domain enumeration and seed-hierarchy types.
//!
//! Faithful Rust port of the C++ `atlas::procedural::PCGDomain` /
//! `atlas::procedural::PCGContext`.

use crate::rng::DeterministicRng;

/// The 16 isolated procedural generation domains.
///
/// Each domain has its own RNG stream derived from the universe seed, so that
/// changes to one domain never affect another.
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
#[repr(u8)]
pub enum PcgDomain {
    Universe  = 0,
    Galaxy    = 1,
    System    = 2,
    Sector    = 3,
    Planet    = 4,
    Asteroid  = 5,
    Ship      = 6,
    Station   = 7,
    Npc       = 8,
    Fleet     = 9,
    Loot      = 10,
    Mission   = 11,
    Anomaly   = 12,
    Economy   = 13,
    Weather   = 14,
    Terrain   = 15,
}

impl PcgDomain {
    /// Total number of domains.
    pub const COUNT: usize = 16;

    /// Human-readable name.
    pub fn name(self) -> &'static str {
        match self {
            Self::Universe => "Universe",
            Self::Galaxy   => "Galaxy",
            Self::System   => "System",
            Self::Sector   => "Sector",
            Self::Planet   => "Planet",
            Self::Asteroid => "Asteroid",
            Self::Ship     => "Ship",
            Self::Station  => "Station",
            Self::Npc      => "NPC",
            Self::Fleet    => "Fleet",
            Self::Loot     => "Loot",
            Self::Mission  => "Mission",
            Self::Anomaly  => "Anomaly",
            Self::Economy  => "Economy",
            Self::Weather  => "Weather",
            Self::Terrain  => "Terrain",
        }
    }

    /// Iterate all variants.
    pub fn all() -> impl Iterator<Item = Self> {
        [
            Self::Universe, Self::Galaxy,  Self::System,  Self::Sector,
            Self::Planet,   Self::Asteroid, Self::Ship,   Self::Station,
            Self::Npc,      Self::Fleet,   Self::Loot,    Self::Mission,
            Self::Anomaly,  Self::Economy, Self::Weather, Self::Terrain,
        ]
        .into_iter()
    }
}

/// Seed-hierarchy level.  Seeds cascade downward:
/// `Universe → Galaxy → System → Sector → Object`.
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord)]
#[repr(u8)]
pub enum SeedLevel {
    Universe = 0,
    Galaxy   = 1,
    System   = 2,
    Sector   = 3,
    Object   = 4,
}

impl SeedLevel {
    pub const COUNT: usize = 5;

    fn next(self) -> Option<Self> {
        match self {
            Self::Universe => Some(Self::Galaxy),
            Self::Galaxy   => Some(Self::System),
            Self::System   => Some(Self::Sector),
            Self::Sector   => Some(Self::Object),
            Self::Object   => None,
        }
    }
}

/// A scoped PCG context tied to a specific domain and hierarchy level.
///
/// Provides a deterministic RNG stream completely isolated from all other
/// domain/level combinations.
pub struct PcgContext {
    pub domain: PcgDomain,
    pub level:  SeedLevel,
    pub seed:   u64,
    pub rng:    DeterministicRng,
}

impl PcgContext {
    pub fn new(domain: PcgDomain, level: SeedLevel, seed: u64) -> Self {
        Self { domain, level, seed, rng: DeterministicRng::new(seed) }
    }

    /// Create a child context at the next hierarchy level, mixed with a
    /// location-specific `salt`.
    pub fn child(&self, location_salt: u64) -> PcgContext {
        let child_seed = self.seed
            ^ location_salt
                .wrapping_mul(6_364_136_223_846_793_005)
                .wrapping_add(1);
        let child_level = self.level.next().unwrap_or(self.level);
        PcgContext::new(self.domain, child_level, child_seed)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn domain_names_unique() {
        use std::collections::HashSet;
        let names: HashSet<&str> = PcgDomain::all().map(|d| d.name()).collect();
        assert_eq!(names.len(), PcgDomain::COUNT);
    }

    #[test]
    fn child_context_differs() {
        let ctx = PcgContext::new(PcgDomain::Planet, SeedLevel::Universe, 42);
        let c1 = ctx.child(100);
        let c2 = ctx.child(200);
        assert_ne!(c1.seed, c2.seed);
    }

    #[test]
    fn seed_level_ordering() {
        assert!(SeedLevel::Universe < SeedLevel::Galaxy);
        assert!(SeedLevel::Sector < SeedLevel::Object);
    }
}
