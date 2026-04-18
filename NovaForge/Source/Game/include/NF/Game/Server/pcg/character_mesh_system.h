#ifndef NOVAFORGE_PCG_CHARACTER_MESH_SYSTEM_H
#define NOVAFORGE_PCG_CHARACTER_MESH_SYSTEM_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include <cstdint>
#include <string>
#include <vector>
#include <map>

namespace atlas {
namespace pcg {

enum class Race : uint32_t {
    TerranDescendant,
    SynthBorn,
    PureAlien,
    HybridEvolutionary
};

/**
 * @brief Body archetype for character generation.
 *
 * Extends the original Race-based system to support a wider range of
 * physical forms — from fully organic humanoids to completely
 * cybernetic robots.
 */
enum class BodyType : uint32_t {
    Organic,       ///< Fully biological humanoid.
    Augmented,     ///< Human with cybernetic implants (partial).
    Cybernetic,    ///< Majority cybernetic body with organic brain.
    FullSynth,     ///< Fully synthetic humanoid — no organic parts.
    MechFrame,     ///< Heavy industrial robot (non-humanoid proportions).
};

/**
 * @brief Cybernetic limb replacement descriptor.
 */
struct CyberLimb {
    std::string slot;      ///< "left_arm", "right_arm", "left_leg", etc.
    float       strength;  ///< Multiplier on base strength.
    float       speed;     ///< Multiplier on base movement speed.
    bool        is_weapon; ///< true if limb has integrated weapon.
};

struct CharacterSliders {
    float height = 0.5f;
    float limb_length = 0.5f;
    float torso_proportion = 0.5f;
    float head_shape = 0.5f;
    float build = 0.5f;
    bool is_male = true;
};

struct GeneratedCharacter {
    uint64_t character_id;
    Race race;
    BodyType bodyType;
    CharacterSliders sliders;
    float total_height;
    float arm_span;
    float torso_height;
    float leg_height;
    float head_radius;
    float learning_rate;
    float diplomacy_bonus;
    float automation_bonus;
    float resilience;
    // ── Cybernetic fields ──────────────────────────────
    float cyber_percentage;             ///< 0.0 (organic) – 1.0 (full synth).
    float strength_multiplier;          ///< Base 1.0; cybernetics modify.
    float speed_multiplier;             ///< Base 1.0; cybernetics modify.
    int   integrated_weapon_count;      ///< Number of weapon-limbs.
    std::vector<CyberLimb> cyberLimbs;  ///< Replaced/augmented limbs.

    // ── Reference mesh & morph fields ─────────────────
    std::string referenceMeshArchive;   ///< Path to zip archive with base mesh (e.g. human.zip).
    float uniformScale;                 ///< Uniform scale applied to the reference mesh.
    std::map<std::string, float> morphWeights; ///< Named morph-target weights (0..1).
};

class CharacterMeshSystem {
public:
    /** Original generate — organic characters. */
    GeneratedCharacter generate(uint64_t seed, Race race, const CharacterSliders& sliders) const;

    /**
     * @brief Generate a character with explicit body type.
     *
     * Supports the full range from fully organic to fully synthetic.
     * Cybernetic limbs are procedurally assigned based on body type
     * and seed.
     */
    GeneratedCharacter generate(uint64_t seed, Race race,
                                BodyType body,
                                const CharacterSliders& sliders) const;

    static void applyRacialTraits(GeneratedCharacter& character);
    static float computeHeight(const CharacterSliders& sliders);

    /** Human-readable body type name. */
    static std::string bodyTypeName(BodyType bt);

    /**
     * @brief Set a zip archive containing a reference mesh for
     *        human-type character generation (e.g. human.zip).
     *
     * When set, generated characters will carry this archive path
     * so the renderer can extract and use the mesh at load time.
     */
    void setReferenceMeshArchive(const std::string& archivePath);

    /** @return Current reference mesh archive path (empty if none). */
    const std::string& referenceMeshArchive() const;

private:
    static void applyCybernetics(DeterministicRNG& rng,
                                 GeneratedCharacter& character,
                                 BodyType body);

    std::string referenceMeshArchive_;  ///< Optional zip with base mesh.
};

} // namespace pcg
} // namespace atlas

#endif // NOVAFORGE_PCG_CHARACTER_MESH_SYSTEM_H
