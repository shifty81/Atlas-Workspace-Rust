#include "../engine/world/NoiseGenerator.h"
#include <iostream>
#include <cassert>
#include <cmath>
#include <set>

using namespace atlas::world;

void test_perlin_range() {
    // Perlin noise should return values roughly in [-1, 1]
    float minVal = 1.0f, maxVal = -1.0f;
    for (int i = 0; i < 1000; ++i) {
        float x = static_cast<float>(i) * 0.13f;
        float y = static_cast<float>(i) * 0.17f;
        float val = NoiseGenerator::Perlin2D(x, y);
        if (val < minVal) minVal = val;
        if (val > maxVal) maxVal = val;
    }
    // Should have some range
    assert(maxVal > minVal);
    assert(minVal >= -2.0f && maxVal <= 2.0f);

    std::cout << "[PASS] test_perlin_range" << std::endl;
}

void test_perlin_deterministic() {
    float v1 = NoiseGenerator::Perlin2D(1.5f, 2.3f, 42);
    float v2 = NoiseGenerator::Perlin2D(1.5f, 2.3f, 42);
    assert(v1 == v2);

    std::cout << "[PASS] test_perlin_deterministic" << std::endl;
}

void test_perlin_seed_variation() {
    float v1 = NoiseGenerator::Perlin2D(1.5f, 2.3f, 42);
    float v2 = NoiseGenerator::Perlin2D(1.5f, 2.3f, 99);
    // Different seeds should (very likely) produce different values
    assert(v1 != v2);

    std::cout << "[PASS] test_perlin_seed_variation" << std::endl;
}

void test_fbm_octaves() {
    // FBM with 1 octave should be same as Perlin (normalized)
    float perlin = NoiseGenerator::Perlin2D(3.0f, 4.0f, 0);
    float fbm1 = NoiseGenerator::FBM2D(3.0f, 4.0f, 1, 2.0f, 0.5f, 0);
    assert(std::abs(perlin - fbm1) < 0.001f);

    // More octaves should still be in reasonable range
    float fbm4 = NoiseGenerator::FBM2D(3.0f, 4.0f, 4, 2.0f, 0.5f, 0);
    assert(fbm4 >= -2.0f && fbm4 <= 2.0f);

    std::cout << "[PASS] test_fbm_octaves" << std::endl;
}

void test_fbm_deterministic() {
    float v1 = NoiseGenerator::FBM2D(5.0f, 7.0f, 4, 2.0f, 0.5f, 123);
    float v2 = NoiseGenerator::FBM2D(5.0f, 7.0f, 4, 2.0f, 0.5f, 123);
    assert(v1 == v2);

    std::cout << "[PASS] test_fbm_deterministic" << std::endl;
}

void test_perlin_spatial_variation() {
    // Different positions should produce different values
    // Use non-integer coordinates because Perlin noise returns 0 at grid points
    float v1 = NoiseGenerator::Perlin2D(0.5f, 0.5f);
    float v2 = NoiseGenerator::Perlin2D(10.5f, 10.5f);
    float v3 = NoiseGenerator::Perlin2D(100.5f, 100.5f);

    // At least 2 of 3 should be different (very high probability)
    bool allSame = (v1 == v2) && (v2 == v3);
    assert(!allSame);

    std::cout << "[PASS] test_perlin_spatial_variation" << std::endl;
}
