#pragma once
/**
 * @file lowpoly_style.h
 * @brief Low-poly visual style enforcement for Nova Forge interiors and meshes.
 *
 * All FPS interior geometry must conform to:
 *   - Flat shading at mesh level (duplicated verts per face, face normals only)
 *   - Hard edges only — no bevels
 *   - Poly budget: corridor 12–24, room 30–60, door 6–12
 *   - Allowed primitives: Box, Wedge, Trapezoid, Hex prism (≤8 sides)
 *   - Forbidden: cylinders > 8 sides, spheres, smooth curves
 *   - Color palette: 6–10 colours per biome
 *   - No roughness maps, no normal maps
 *   - Max 2 materials per mesh
 *
 * If ValidateLowPolyStyle() returns false, the mesh must be regenerated,
 * simplified, or rejected.
 */

#include <string>
#include <vector>
#include <array>
#include <cstdint>

namespace atlas::gameplay {

// ── Mesh descriptor for style validation ────────────────────────────────────

struct LowPolyMesh {
    std::string          id;
    int                  faceCount     = 0;
    bool                 hasFlatNormals = false;  ///< face-normal per face
    bool                 hasBevelEdges  = false;  ///< forbidden
    int                  materialCount  = 1;
    bool                 hasRoughnessMap = false; ///< forbidden
    bool                 hasNormalMap    = false; ///< forbidden
    int                  maxSidedCylinder = 0;   ///< 0 = no cylinder; >8 = violation
};

// ── Per-context budgets ──────────────────────────────────────────────────────

struct LowPolyBudget {
    int maxFaces     = 60;
    int maxMaterials = 2;
};

inline LowPolyBudget CorridorBudget()  { return { 24, 2 }; }
inline LowPolyBudget RoomBudget()      { return { 60, 2 }; }
inline LowPolyBudget DoorBudget()      { return { 12, 1 }; }

// ── Biome colour palette ─────────────────────────────────────────────────────

struct BiomePalette {
    std::string             biomeID;
    std::vector<std::array<uint8_t, 3>> colours; ///< RGB, 6–10 entries

    bool IsValid() const {
        return colours.size() >= 6 && colours.size() <= 10;
    }
};

// ── Validation result ────────────────────────────────────────────────────────

struct LowPolyValidationResult {
    bool        valid = true;
    std::string error;            ///< empty when valid
};

// ── LowPolyStyleValidator ────────────────────────────────────────────────────

/**
 * Validates that a mesh conforms to the Nova Forge low-poly visual style.
 *
 * Failure → regenerate, simplify, or reject.
 */
class LowPolyStyleValidator {
public:
    LowPolyStyleValidator() = default;

    /**
     * Validate a mesh against the given poly budget.
     * @param mesh   Mesh descriptor to validate.
     * @param budget Budget (use CorridorBudget(), RoomBudget(), etc.).
     * @return Validation result with error description on failure.
     */
    LowPolyValidationResult Validate(const LowPolyMesh& mesh,
                                     const LowPolyBudget& budget = RoomBudget()) const;

    /**
     * Validate that a biome palette has 6–10 colours.
     */
    LowPolyValidationResult ValidatePalette(const BiomePalette& palette) const;

    // ── Quick boolean shorthand ───────────────────────────────────────

    bool IsValid(const LowPolyMesh& mesh,
                 const LowPolyBudget& budget = RoomBudget()) const {
        return Validate(mesh, budget).valid;
    }

    // ── Constants ─────────────────────────────────────────────────────

    static constexpr int MAX_CYLINDER_SIDES = 8;
    static constexpr int MIN_PALETTE_COLOURS = 6;
    static constexpr int MAX_PALETTE_COLOURS = 10;
};

} // namespace atlas::gameplay
