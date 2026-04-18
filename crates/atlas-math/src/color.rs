//! RGBA colour type.

use serde::{Deserialize, Serialize};

/// Linear RGBA colour (components in [0, 1]).
#[derive(Clone, Copy, Debug, PartialEq, Serialize, Deserialize)]
pub struct Color {
    pub r: f32,
    pub g: f32,
    pub b: f32,
    pub a: f32,
}

impl Color {
    pub const WHITE:       Color = Color::rgba(1.0, 1.0, 1.0, 1.0);
    pub const BLACK:       Color = Color::rgba(0.0, 0.0, 0.0, 1.0);
    pub const TRANSPARENT: Color = Color::rgba(0.0, 0.0, 0.0, 0.0);
    pub const RED:         Color = Color::rgba(1.0, 0.0, 0.0, 1.0);
    pub const GREEN:       Color = Color::rgba(0.0, 1.0, 0.0, 1.0);
    pub const BLUE:        Color = Color::rgba(0.0, 0.0, 1.0, 1.0);

    pub const fn rgba(r: f32, g: f32, b: f32, a: f32) -> Self {
        Self { r, g, b, a }
    }

    pub const fn rgb(r: f32, g: f32, b: f32) -> Self {
        Self::rgba(r, g, b, 1.0)
    }

    /// Convert from 8-bit RGBA components.
    pub fn from_u8(r: u8, g: u8, b: u8, a: u8) -> Self {
        Self::rgba(
            r as f32 / 255.0,
            g as f32 / 255.0,
            b as f32 / 255.0,
            a as f32 / 255.0,
        )
    }

    /// Convert to 8-bit packed `0xRRGGBBAA`.
    pub fn to_u32(self) -> u32 {
        let r = (self.r.clamp(0.0, 1.0) * 255.0) as u32;
        let g = (self.g.clamp(0.0, 1.0) * 255.0) as u32;
        let b = (self.b.clamp(0.0, 1.0) * 255.0) as u32;
        let a = (self.a.clamp(0.0, 1.0) * 255.0) as u32;
        (r << 24) | (g << 16) | (b << 8) | a
    }

    /// Linear interpolation between two colours.
    pub fn lerp(self, other: Color, t: f32) -> Color {
        Color::rgba(
            self.r + (other.r - self.r) * t,
            self.g + (other.g - self.g) * t,
            self.b + (other.b - self.b) * t,
            self.a + (other.a - self.a) * t,
        )
    }
}

impl Default for Color {
    fn default() -> Self {
        Color::WHITE
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn constants_correct() {
        assert_eq!(Color::WHITE.r, 1.0);
        assert_eq!(Color::BLACK.r, 0.0);
        assert_eq!(Color::TRANSPARENT.a, 0.0);
        assert_eq!(Color::RED.r, 1.0);
        assert_eq!(Color::RED.g, 0.0);
    }

    #[test]
    fn default_is_white() {
        let c = Color::default();
        assert_eq!(c, Color::WHITE);
    }

    #[test]
    fn rgb_alpha_is_one() {
        let c = Color::rgb(0.5, 0.5, 0.5);
        assert_eq!(c.a, 1.0);
    }

    #[test]
    fn from_u8_round_trip() {
        let c = Color::from_u8(255, 128, 0, 255);
        assert_eq!(c.r, 1.0);
        assert!((c.g - 128.0 / 255.0).abs() < 1e-4);
        assert_eq!(c.b, 0.0);
        assert_eq!(c.a, 1.0);
    }

    #[test]
    fn to_u32_packed() {
        let u = Color::WHITE.to_u32();
        assert_eq!(u, 0xFFFFFFFF);
        let u2 = Color::BLACK.to_u32();
        assert_eq!(u2, 0x000000FF); // r=0 g=0 b=0 a=255
    }

    #[test]
    fn lerp_midpoint() {
        let c = Color::BLACK.lerp(Color::WHITE, 0.5);
        assert!((c.r - 0.5).abs() < 1e-5);
        assert!((c.g - 0.5).abs() < 1e-5);
        assert!((c.b - 0.5).abs() < 1e-5);
        assert_eq!(c.a, 1.0);
    }

    #[test]
    fn lerp_endpoints() {
        assert_eq!(Color::RED.lerp(Color::BLUE, 0.0), Color::RED);
        assert_eq!(Color::RED.lerp(Color::BLUE, 1.0), Color::BLUE);
    }
}
