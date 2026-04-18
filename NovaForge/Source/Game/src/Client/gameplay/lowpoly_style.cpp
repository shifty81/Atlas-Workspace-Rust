/**
 * @file lowpoly_style.cpp
 * @brief LowPolyStyleValidator implementation.
 */

#include "gameplay/lowpoly_style.h"

namespace atlas::gameplay {

LowPolyValidationResult LowPolyStyleValidator::Validate(
    const LowPolyMesh& mesh,
    const LowPolyBudget& budget) const
{
    // 1. Flat normals required
    if (!mesh.hasFlatNormals)
        return { false, "Mesh must use flat normals (no smooth shading)" };

    // 2. No bevel edges
    if (mesh.hasBevelEdges)
        return { false, "Bevelled edges are forbidden in low-poly style" };

    // 3. Poly budget
    if (mesh.faceCount > budget.maxFaces) {
        return { false,
                 "Face count " + std::to_string(mesh.faceCount) +
                 " exceeds budget " + std::to_string(budget.maxFaces) };
    }

    // 4. Material count
    if (mesh.materialCount > budget.maxMaterials) {
        return { false,
                 "Material count " + std::to_string(mesh.materialCount) +
                 " exceeds limit " + std::to_string(budget.maxMaterials) };
    }

    // 5. No PBR maps
    if (mesh.hasRoughnessMap)
        return { false, "Roughness maps are forbidden in low-poly style" };
    if (mesh.hasNormalMap)
        return { false, "Normal maps are forbidden in low-poly style" };

    // 6. Cylinder side count
    if (mesh.maxSidedCylinder > MAX_CYLINDER_SIDES) {
        return { false,
                 "Cylinder with " + std::to_string(mesh.maxSidedCylinder) +
                 " sides exceeds maximum of " +
                 std::to_string(MAX_CYLINDER_SIDES) };
    }

    return { true, "" };
}

LowPolyValidationResult LowPolyStyleValidator::ValidatePalette(
    const BiomePalette& palette) const
{
    if (palette.colours.size() < static_cast<size_t>(MIN_PALETTE_COLOURS) ||
        palette.colours.size() > static_cast<size_t>(MAX_PALETTE_COLOURS)) {
        return { false,
                 "Palette must have 6–10 colours (has " +
                 std::to_string(palette.colours.size()) + ")" };
    }
    return { true, "" };
}

} // namespace atlas::gameplay
