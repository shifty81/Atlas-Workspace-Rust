#pragma once
#include "ProceduralMaterialGraph.h"

namespace atlas::procedural {

MaterialData GenerateSolidColor(uint32_t width, uint32_t height,
                                float r, float g, float b, float a = 1.0f);
MaterialData GenerateCheckerboard(uint32_t width, uint32_t height, uint32_t tileSize,
                                  float r1, float g1, float b1,
                                  float r2, float g2, float b2);
MaterialData GenerateNoiseTexture(uint32_t width, uint32_t height, uint64_t seed, float scale);
MaterialData BlendMaterials(const MaterialData& a, const MaterialData& b, float factor);
MaterialData ComputeNormalMap(const MaterialData& src, float strength);

// Procedural non-repeating texture generation using domain warping
MaterialData GenerateProceduralTexture(uint32_t width, uint32_t height, uint64_t seed,
                                       float baseFrequency, int octaves, float warpStrength);

}
