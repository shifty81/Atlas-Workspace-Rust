#include "ProceduralMaterialNodes.h"
#include <cmath>

namespace atlas::procedural {

MaterialData GenerateSolidColor(uint32_t width, uint32_t height,
                                float r, float g, float b, float a) {
    MaterialData mat;
    mat.width = width;
    mat.height = height;
    size_t pc = mat.PixelCount();

    mat.albedo.resize(pc * 4);
    mat.normal.resize(pc * 3);
    mat.roughness.resize(pc);
    mat.metallic.resize(pc);

    for (size_t i = 0; i < pc; ++i) {
        mat.albedo[i * 4 + 0] = r;
        mat.albedo[i * 4 + 1] = g;
        mat.albedo[i * 4 + 2] = b;
        mat.albedo[i * 4 + 3] = a;

        // Flat normal pointing up (0, 0, 1) in tangent space
        mat.normal[i * 3 + 0] = 0.0f;
        mat.normal[i * 3 + 1] = 0.0f;
        mat.normal[i * 3 + 2] = 1.0f;

        mat.roughness[i] = 0.5f;
        mat.metallic[i] = 0.0f;
    }

    return mat;
}

MaterialData GenerateCheckerboard(uint32_t width, uint32_t height, uint32_t tileSize,
                                  float r1, float g1, float b1,
                                  float r2, float g2, float b2) {
    MaterialData mat;
    mat.width = width;
    mat.height = height;
    size_t pc = mat.PixelCount();

    mat.albedo.resize(pc * 4);
    mat.normal.resize(pc * 3);
    mat.roughness.resize(pc);
    mat.metallic.resize(pc);

    uint32_t ts = tileSize > 0 ? tileSize : 1;

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            size_t i = static_cast<size_t>(y) * width + x;
            bool even = ((x / ts) + (y / ts)) % 2 == 0;

            mat.albedo[i * 4 + 0] = even ? r1 : r2;
            mat.albedo[i * 4 + 1] = even ? g1 : g2;
            mat.albedo[i * 4 + 2] = even ? b1 : b2;
            mat.albedo[i * 4 + 3] = 1.0f;

            mat.normal[i * 3 + 0] = 0.0f;
            mat.normal[i * 3 + 1] = 0.0f;
            mat.normal[i * 3 + 2] = 1.0f;

            mat.roughness[i] = 0.5f;
            mat.metallic[i] = 0.0f;
        }
    }

    return mat;
}

MaterialData GenerateNoiseTexture(uint32_t width, uint32_t height, uint64_t seed, float scale) {
    MaterialData mat;
    mat.width = width;
    mat.height = height;
    size_t pc = mat.PixelCount();

    mat.albedo.resize(pc * 4);
    mat.normal.resize(pc * 3);
    mat.roughness.resize(pc);
    mat.metallic.resize(pc);

    // Xorshift64 deterministic RNG — same pattern as ProceduralMeshNodes.cpp
    uint64_t state = seed ^ 0x5DEECE66DULL;
    if (state == 0) state = 1;

    auto xorshift = [&]() -> float {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        return static_cast<float>(state & 0xFFFFu) / 65535.0f;
    };

    for (size_t i = 0; i < pc; ++i) {
        float v = xorshift() * scale;
        if (v > 1.0f) v = 1.0f;
        if (v < 0.0f) v = 0.0f;

        mat.albedo[i * 4 + 0] = v;
        mat.albedo[i * 4 + 1] = v;
        mat.albedo[i * 4 + 2] = v;
        mat.albedo[i * 4 + 3] = 1.0f;

        mat.normal[i * 3 + 0] = 0.0f;
        mat.normal[i * 3 + 1] = 0.0f;
        mat.normal[i * 3 + 2] = 1.0f;

        mat.roughness[i] = v;
        mat.metallic[i] = 0.0f;
    }

    return mat;
}

MaterialData BlendMaterials(const MaterialData& a, const MaterialData& b, float factor) {
    MaterialData mat;
    if (a.width != b.width || a.height != b.height) return mat;
    if (!a.IsValid() || !b.IsValid()) return mat;

    mat.width = a.width;
    mat.height = a.height;
    size_t pc = mat.PixelCount();
    float t = factor;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    float inv = 1.0f - t;

    mat.albedo.resize(pc * 4);
    mat.normal.resize(pc * 3);
    mat.roughness.resize(pc);
    mat.metallic.resize(pc);

    for (size_t i = 0; i < pc * 4; ++i) {
        mat.albedo[i] = a.albedo[i] * inv + b.albedo[i] * t;
    }

    for (size_t i = 0; i < pc * 3; ++i) {
        mat.normal[i] = a.normal[i] * inv + b.normal[i] * t;
    }

    // Renormalize blended normals
    for (size_t i = 0; i < pc; ++i) {
        size_t ni = i * 3;
        float nx = mat.normal[ni], ny = mat.normal[ni + 1], nz = mat.normal[ni + 2];
        float len = std::sqrt(nx * nx + ny * ny + nz * nz);
        if (len > 0.0f) {
            mat.normal[ni] = nx / len;
            mat.normal[ni + 1] = ny / len;
            mat.normal[ni + 2] = nz / len;
        }
    }

    for (size_t i = 0; i < pc; ++i) {
        mat.roughness[i] = a.roughness[i] * inv + b.roughness[i] * t;
        mat.metallic[i] = a.metallic[i] * inv + b.metallic[i] * t;
    }

    return mat;
}

MaterialData ComputeNormalMap(const MaterialData& src, float strength) {
    MaterialData mat;
    if (!src.IsValid()) return mat;

    mat.width = src.width;
    mat.height = src.height;
    size_t pc = mat.PixelCount();

    // Copy albedo, roughness, metallic from source
    mat.albedo = src.albedo;
    mat.roughness = src.roughness;
    mat.metallic = src.metallic;
    mat.normal.resize(pc * 3);

    // Use roughness channel as heightmap, compute Sobel-like normals
    auto getHeight = [&](int x, int y) -> float {
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x >= static_cast<int>(src.width)) x = static_cast<int>(src.width) - 1;
        if (y >= static_cast<int>(src.height)) y = static_cast<int>(src.height) - 1;
        return src.roughness[static_cast<size_t>(y) * src.width + x];
    };

    for (uint32_t y = 0; y < src.height; ++y) {
        for (uint32_t x = 0; x < src.width; ++x) {
            int ix = static_cast<int>(x);
            int iy = static_cast<int>(y);

            // Sobel operator for dx and dy
            float dx = (getHeight(ix + 1, iy - 1) + 2.0f * getHeight(ix + 1, iy) + getHeight(ix + 1, iy + 1))
                      -(getHeight(ix - 1, iy - 1) + 2.0f * getHeight(ix - 1, iy) + getHeight(ix - 1, iy + 1));
            float dy = (getHeight(ix - 1, iy + 1) + 2.0f * getHeight(ix, iy + 1) + getHeight(ix + 1, iy + 1))
                      -(getHeight(ix - 1, iy - 1) + 2.0f * getHeight(ix, iy - 1) + getHeight(ix + 1, iy - 1));

            float nx = -dx * strength;
            float ny = -dy * strength;
            float nz = 1.0f;

            float len = std::sqrt(nx * nx + ny * ny + nz * nz);
            if (len > 0.0f) {
                nx /= len;
                ny /= len;
                nz /= len;
            }

            size_t i = static_cast<size_t>(y) * src.width + x;
            mat.normal[i * 3 + 0] = nx;
            mat.normal[i * 3 + 1] = ny;
            mat.normal[i * 3 + 2] = nz;
        }
    }

    return mat;
}

MaterialData GenerateProceduralTexture(uint32_t width, uint32_t height, uint64_t seed,
                                       float baseFrequency, int octaves, float warpStrength) {
    MaterialData mat;
    mat.width = width;
    mat.height = height;
    size_t pc = mat.PixelCount();

    mat.albedo.resize(pc * 4);
    mat.normal.resize(pc * 3);
    mat.roughness.resize(pc);
    mat.metallic.resize(pc);

    // Deterministic xorshift64 for domain warping offsets
    uint64_t state = seed ^ 0xA5A5A5A5A5A5A5A5ULL;
    if (state == 0) state = 1;
    auto xorshift = [&]() -> float {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        return static_cast<float>(state & 0xFFFFu) / 65535.0f;
    };

    // Generate domain warp offsets for non-repeating patterns
    float warpOffsetX = xorshift() * 100.0f;
    float warpOffsetY = xorshift() * 100.0f;
    float colorShiftR = xorshift();
    float colorShiftG = xorshift();
    float colorShiftB = xorshift();

    int oct = octaves > 0 ? octaves : 4;
    float freq = baseFrequency > 0.0f ? baseFrequency : 0.05f;

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            size_t i = static_cast<size_t>(y) * width + x;
            float fx = static_cast<float>(x) / static_cast<float>(width);
            float fy = static_cast<float>(y) / static_cast<float>(height);

            // Domain warping: warp coordinates using noise-like function
            float wx = fx + warpStrength * std::sin(fy * 6.2832f + warpOffsetX);
            float wy = fy + warpStrength * std::cos(fx * 6.2832f + warpOffsetY);

            // Multi-octave value noise approximation
            float value = 0.0f;
            float amplitude = 1.0f;
            float totalAmp = 0.0f;
            float f = freq;
            // Well-known pseudo-random hash constants (Dave Hoskins)
            constexpr float kNoiseHashX = 12.9898f;
            constexpr float kNoiseHashY = 78.233f;
            constexpr float kNoiseHashMultiplier = 43758.5453f;
            for (int o = 0; o < oct; ++o) {
                float n = std::sin(wx * f * kNoiseHashX + wy * f * kNoiseHashY + static_cast<float>(seed & 0xFFFF)) * kNoiseHashMultiplier;
                n = n - std::floor(n); // fract
                value += n * amplitude;
                totalAmp += amplitude;
                amplitude *= 0.5f;
                f *= 2.0f;
            }
            value /= totalAmp;

            // Generate color variation per channel for non-repeating appearance
            float r = value * (0.5f + colorShiftR * 0.5f);
            float g = value * (0.5f + colorShiftG * 0.5f);
            float b = value * (0.5f + colorShiftB * 0.5f);

            mat.albedo[i * 4 + 0] = std::max(0.0f, std::min(1.0f, r));
            mat.albedo[i * 4 + 1] = std::max(0.0f, std::min(1.0f, g));
            mat.albedo[i * 4 + 2] = std::max(0.0f, std::min(1.0f, b));
            mat.albedo[i * 4 + 3] = 1.0f;

            mat.normal[i * 3 + 0] = 0.0f;
            mat.normal[i * 3 + 1] = 0.0f;
            mat.normal[i * 3 + 2] = 1.0f;

            mat.roughness[i] = value;
            mat.metallic[i] = 0.0f;
        }
    }

    return mat;
}

}
