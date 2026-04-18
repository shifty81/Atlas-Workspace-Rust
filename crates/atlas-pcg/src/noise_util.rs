//! Noise utility functions for PCG.
//!
//! Wraps the `noise` crate to expose fbm and ridged-multifractal noise.
//!
//! **Octave limit**: the `noise` crate pre-allocates sources up to the
//! requested octave count, so `octaves` is clamped to `[1, 32]` and built
//! via the `MultiFractal` builder to ensure the source vector is correctly
//! sized.

use noise::{Fbm, MultiFractal, NoiseFn, Perlin, RidgedMulti};

/// Fractional Brownian Motion noise at point `(x, y, z)`.
///
/// - `octaves`: number of noise layers (1–32)
/// - `frequency`: base frequency
/// - `persistence`: amplitude falloff per octave
/// - `lacunarity`: frequency multiplier per octave
pub fn fbm(x: f64, y: f64, z: f64, octaves: usize, frequency: f64, persistence: f64, lacunarity: f64) -> f64 {
    let n = Fbm::<Perlin>::new(0)
        .set_octaves(octaves.clamp(1, 32))
        .set_frequency(frequency)
        .set_persistence(persistence)
        .set_lacunarity(lacunarity);
    n.get([x, y, z])
}

/// Ridged-multifractal noise at point `(x, y, z)`.
pub fn ridged_multifractal(x: f64, y: f64, z: f64, octaves: usize, frequency: f64) -> f64 {
    let n = RidgedMulti::<Perlin>::new(0)
        .set_octaves(octaves.clamp(1, 32))
        .set_frequency(frequency);
    n.get([x, y, z])
}

/// Simple 2-D Perlin noise in `[-1, 1]`.
pub fn perlin_2d(x: f64, y: f64) -> f64 {
    let p = Perlin::new(0);
    p.get([x, y])
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn fbm_range() {
        for i in 0..100 {
            let v = fbm(i as f64 * 0.1, 0.0, 0.0, 4, 1.0, 0.5, 2.0);
            assert!(v.is_finite(), "fbm returned non-finite {}", v);
        }
    }

    #[test]
    fn fbm_high_octaves() {
        // Should not panic at 8 octaves
        let v = fbm(1.0, 2.0, 3.0, 8, 1.0, 0.5, 2.0);
        assert!(v.is_finite());
    }

    #[test]
    fn ridged_range() {
        for i in 0..100 {
            let v = ridged_multifractal(i as f64 * 0.1, 0.0, 0.0, 4, 1.0);
            assert!(v.is_finite());
        }
    }
}
