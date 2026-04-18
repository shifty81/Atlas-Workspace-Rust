#ifndef NOVAFORGE_SYSTEMS_TITAN_ASSEMBLY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_TITAN_ASSEMBLY_SYSTEM_H

#include <string>
#include <vector>
#include <cstdint>

namespace atlas {
namespace systems {

// ── Titan assembly pressure phases ──────────────────────────────────
enum class TitanPhase : uint32_t {
    Rumor,       // 0-20%  — vague chatter, no hard evidence
    Unease,      // 20-50% — NPC behavior shifts, resource diversion
    Fear,        // 50-80% — fleet mobilizations visible
    Acceptance,  // 80-100% — Titan nearly complete, crisis imminent
};

// ── Component storing per-region assembly state ─────────────────────
struct TitanAssemblyComponent {
    float    progress     = 0.0f;   // [0, 1] assembly fraction
    float    resourceRate = 0.01f;  // progress per tick (tunable)
    TitanPhase phase      = TitanPhase::Rumor;
    bool     disrupted    = false;  // player sabotaged this tick
    uint32_t disruptCount = 0;      // total times disrupted
};

/**
 * @brief Simulates pirate Titan assembly as a background pressure.
 *
 * Each tick:
 *   1. Advance progress by resourceRate (unless disrupted).
 *   2. Disruption slows progress and may regress phase.
 *   3. Phase thresholds: Rumor < 0.20, Unease < 0.50, Fear < 0.80, Acceptance.
 *   4. Progress never exceeds 1.0.
 */
class TitanAssemblySystem {
public:
    /** Advance assembly by one tick. */
    void tick(TitanAssemblyComponent& comp) const;

    /** Player disrupts the assembly (reduces progress). */
    void disrupt(TitanAssemblyComponent& comp, float amount) const;

    /** Human-readable phase name. */
    static std::string phaseName(TitanPhase phase);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_TITAN_ASSEMBLY_SYSTEM_H
