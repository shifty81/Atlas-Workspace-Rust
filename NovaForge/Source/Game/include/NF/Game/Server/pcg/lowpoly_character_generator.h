#ifndef ATLAS_PCG_LOWPOLY_CHARACTER_GENERATOR_H
#define ATLAS_PCG_LOWPOLY_CHARACTER_GENERATOR_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas {
namespace pcg {

// ── Modular body-part slots (the "LEGO" approach) ───────────────────

/**
 * @brief Anatomical slots for the modular assembly system.
 *
 * Each slot can hold one mesh variant that is snapped onto the shared
 * skeleton at a predefined joint.  The slot set matches the base
 * skeleton from character_skeleton.h so any combination is valid.
 */
enum class BodySlot : uint32_t {
    Head,
    Torso,
    ArmLeft,
    ArmRight,
    LegLeft,
    LegRight,
    Hands,
    Feet,
};

static constexpr int BODY_SLOT_COUNT = 8;

/** Human-readable name for a body slot. */
const char* bodySlotName(BodySlot slot);

// ── Clothing / accessory layer ──────────────────────────────────────

/**
 * @brief Equippable clothing and gear categories.
 *
 * Clothing pieces are layered on top of the body mesh at the same
 * skeleton attachment points.  The generator picks random items per
 * slot according to the character archetype.
 */
enum class ClothingSlot : uint32_t {
    Hat,
    Jacket,
    Shirt,
    Pants,
    Shoes,
    Gloves,
    Backpack,
    FaceAccessory,
};

static constexpr int CLOTHING_SLOT_COUNT = 8;

/** Human-readable name for a clothing slot. */
const char* clothingSlotName(ClothingSlot slot);

// ── Character archetype (survivor theme) ────────────────────────────

/**
 * @brief High-level character archetype that biases the generator.
 *
 * Archetypes influence which clothing pieces, color palettes, and
 * accessories are likely to be selected.
 */
enum class CharacterArchetype : uint32_t {
    Survivor,      ///< Rugged, makeshift gear (SurrounDead-style).
    Militia,       ///< Military-surplus, practical.
    Civilian,      ///< Everyday clothing, neutral colors.
    Scavenger,     ///< Patchwork, mixed-source clothing.
    Medic,         ///< Medical gear with white/red accents.
};

static constexpr int ARCHETYPE_COUNT = 5;

/** Human-readable name for an archetype. */
const char* archetypeName(CharacterArchetype archetype);

// ── Color palette (SurrounDead-style flat / vertex-color approach) ──

/**
 * @brief A single entry in the color palette atlas.
 *
 * Stored as linear RGB [0,1].  At runtime, each face/vertex is mapped
 * to one palette entry — no traditional UV textures are needed.
 */
struct PaletteColor {
    float r, g, b;
};

/**
 * @brief Predefined low-poly palette for a specific body region.
 *
 * Each region (skin, clothing, hair, etc.) has a small set of color
 * options that the generator picks from deterministically.
 */
struct PaletteRegion {
    std::string              regionName;  ///< e.g. "skin", "shirt", "pants"
    std::vector<PaletteColor> colors;    ///< Candidate colors for this region.
    int                       chosen;     ///< Index into colors (set by generator).
};

// ── Modular mesh piece ──────────────────────────────────────────────

/**
 * @brief A single low-poly mesh piece in the modular assembly.
 *
 * Represents one exchangeable body part or clothing item.  The mesh
 * file path is a reference to a Blender-exported .obj or engine-native
 * format that the renderer will load.
 *
 * Design notes for SurrounDead-style low-poly:
 *   • meshFile points to a pre-decimated mesh (< 500 tris per piece)
 *   • flatShaded = true for the characteristic angular look
 *   • paletteIndex maps into the color atlas rather than a UV texture
 */
struct LowPolyMeshPiece {
    std::string meshFile;       ///< Path to the low-poly .obj / native mesh.
    std::string variant;        ///< Variant name (e.g. "survivor_torso_A").
    int         paletteIndex;   ///< Index into GeneratedLowPolyCharacter::palette.
    float       scaleX, scaleY, scaleZ;
    float       offsetX, offsetY, offsetZ;
    bool        flatShaded;     ///< true → per-face normals (blocky look).
    bool        useVertexColors;///< true → color from palette, no UV texture.
};

// ── FPS arm configuration ───────────────────────────────────────────

/**
 * @brief Describes which mesh pieces are visible in first-person view.
 *
 * In FPS mode only the arms (and optionally gloves / sleeves) are
 * rendered.  This struct lists the pieces the renderer should draw
 * when the camera is in first-person.
 */
struct FPSArmConfig {
    std::string leftArmMesh;    ///< Mesh for left FPS arm.
    std::string rightArmMesh;   ///< Mesh for right FPS arm.
    std::string leftGloveMesh;  ///< Optional glove overlay.
    std::string rightGloveMesh;
    std::string leftSleeveMesh; ///< Optional sleeve overlay.
    std::string rightSleeveMesh;
    int         skinPaletteIndex;  ///< Palette entry for exposed skin.
    int         glovePaletteIndex; ///< Palette entry for glove color.
    int         sleevePaletteIndex;///< Palette entry for sleeve color.
};

// ── Complete generated character ────────────────────────────────────

/**
 * @brief Complete procedurally generated low-poly character.
 *
 * Contains all data needed for the renderer to assemble and display
 * the character — body meshes, clothing meshes, color palette, and
 * FPS arm configuration.  Every field is deterministically derived
 * from the seed so the same seed always produces the same character.
 */
struct GeneratedLowPolyCharacter {
    uint64_t            characterId;
    CharacterArchetype  archetype;
    bool                isMale;

    // ── Body meshes (modular "doll" structure) ───────
    std::vector<LowPolyMeshPiece> bodyParts;  ///< One per BodySlot.

    // ── Clothing / accessory meshes ──────────────────
    std::vector<LowPolyMeshPiece> clothing;   ///< Selected gear pieces.

    // ── Color palette atlas ──────────────────────────
    std::vector<PaletteRegion>    palette;     ///< Color atlas regions.

    // ── Flat-shading & rendering hints ───────────────
    bool  flatShaded;         ///< Master switch for angular low-poly look.
    bool  useVertexColors;    ///< Master switch for palette-based coloring.
    int   maxTriCount;        ///< Poly budget for the assembled character.

    // ── FPS arm data ─────────────────────────────────
    FPSArmConfig fpsArms;

    // ── Skeleton reference ───────────────────────────
    std::string skeletonId;   ///< "base_humanoid" — consistent across all parts.

    // ── Optional Blender source archive ──────────────
    std::string blenderSourceArchive; ///< Path to .blend / .zip with source assets.

    bool valid;               ///< true if assembly passed validation.
};

// ── Generator ───────────────────────────────────────────────────────

/**
 * @brief Deterministic low-poly character generator.
 *
 * Implements the SurrounDead-inspired modular assembly pipeline:
 *   1. Select archetype (biases clothing/palette choices)
 *   2. Pick body-part mesh variants per slot
 *   3. Pick clothing mesh variants per slot
 *   4. Build color palette and assign palette indices
 *   5. Configure FPS arm meshes
 *   6. Validate poly budget
 *
 * Same PCGContext → same character, forever.
 */
class LowPolyCharacterGenerator {
public:
    /** Generate with random archetype. */
    static GeneratedLowPolyCharacter generate(const PCGContext& ctx);

    /** Generate with explicit archetype. */
    static GeneratedLowPolyCharacter generate(const PCGContext& ctx,
                                               CharacterArchetype archetype);

    /** Generate with explicit archetype and gender. */
    static GeneratedLowPolyCharacter generate(const PCGContext& ctx,
                                               CharacterArchetype archetype,
                                               bool isMale);

    /** Human-readable archetype name. */
    static const char* archetypeClassName(CharacterArchetype a);

private:
    static CharacterArchetype selectArchetype(DeterministicRNG& rng);
    static void               buildBodyParts(DeterministicRNG& rng,
                                              GeneratedLowPolyCharacter& ch);
    static void               buildClothing(DeterministicRNG& rng,
                                             GeneratedLowPolyCharacter& ch);
    static void               buildPalette(DeterministicRNG& rng,
                                            GeneratedLowPolyCharacter& ch);
    static void               buildFPSArms(DeterministicRNG& rng,
                                            GeneratedLowPolyCharacter& ch);
    static bool               validatePolyBudget(
                                  const GeneratedLowPolyCharacter& ch);
};

} // namespace pcg
} // namespace atlas

#endif // ATLAS_PCG_LOWPOLY_CHARACTER_GENERATOR_H
