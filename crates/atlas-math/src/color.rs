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
