#ifndef NOVAFORGE_PCG_ASTEROID_FIELD_GENERATOR_H
#define NOVAFORGE_PCG_ASTEROID_FIELD_GENERATOR_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include <vector>
#include <string>
#include <cstdint>

namespace atlas {
namespace pcg {

// ── Asteroid mineral types ──────────────────────────────────────────
enum class AsteroidType : uint32_t {
    Ferrite,      ///< Common, high-sec.
    Galvite,      ///< Common, high-sec.
    Cryolite,     ///< Uncommon, mid-sec.
    Silvane,   ///< Uncommon, mid-sec.
    Duskite,         ///< Rare, low-sec.
    Heliore,       ///< Rare, low-sec.
    Jaspet,        ///< Very rare, null-sec.
    Mercoxit,      ///< Exceedingly rare, null-sec only.
};

// ── Single asteroid in a belt ───────────────────────────────────────
struct AsteroidNode {
    int           asteroidId;
    AsteroidType  type;
    float         radius;        ///< Metres.
    float         mineralYield;  ///< Units of ore available.
    float         posX, posY, posZ;
    float         scaleFactor;   ///< Uniform scale multiplier for the mesh (default 1.0).
    std::string   meshArchive;   ///< Optional zip archive with rock mesh (e.g. rocks_stylized_color.zip).
};

// ── Generated asteroid field / belt ─────────────────────────────────
struct AsteroidField {
    uint64_t                   fieldId;
    std::vector<AsteroidNode>  asteroids;
    float                      fieldRadius;   ///< Bounding radius of the entire field.
    int                        totalAsteroids;
    float                      totalYield;
};

/**
 * @brief Deterministic asteroid-belt generator.
 *
 * Places asteroids in a disc-shaped region with mineral types
 * determined by the security level parameter.  The same PCGContext
 * always produces the same field layout.
 */
class AsteroidFieldGenerator {
public:
    /**
     * @brief Generate a belt with a procedurally chosen asteroid count.
     * @param ctx       PCG context (seed + version).
     * @param secLevel  Security level 0.0 (null) – 1.0 (high-sec).
     */
    static AsteroidField generate(const PCGContext& ctx, float secLevel);

    /**
     * @brief Generate a belt with an explicit asteroid count.
     * @param ctx            PCG context.
     * @param asteroidCount  Number of asteroids to place.
     * @param secLevel       Security level 0.0 – 1.0.
     */
    static AsteroidField generate(const PCGContext& ctx,
                                  int asteroidCount,
                                  float secLevel);

    /**
     * @brief Generate a belt using a reference rock mesh archive.
     *
     * Each asteroid will carry the archive path and a per-asteroid
     * scale factor derived from its procedural radius so the renderer
     * can load and scale the rock mesh.
     *
     * @param ctx            PCG context.
     * @param asteroidCount  Number of asteroids to place.
     * @param secLevel       Security level 0.0 – 1.0.
     * @param rockMeshArchive  Path to zip with rock mesh (e.g. rocks_stylized_color.zip).
     */
    static AsteroidField generate(const PCGContext& ctx,
                                  int asteroidCount,
                                  float secLevel,
                                  const std::string& rockMeshArchive);

    /** Compute total mineral yield for a field. */
    static float calculateTotalYield(const AsteroidField& field);

private:
    static AsteroidType selectType(DeterministicRNG& rng, float secLevel);
    static float        rollRadius(DeterministicRNG& rng, AsteroidType type);
    static float        rollYield(DeterministicRNG& rng, AsteroidType type);
};

} // namespace pcg
} // namespace atlas

#endif // NOVAFORGE_PCG_ASTEROID_FIELD_GENERATOR_H
