#pragma once
/**
 * @file legend_system.h
 * @brief Roguelike Legend & Reputation system — Phase 24.
 *
 * Three memory tiers:
 *   Persistent Legend — never forgotten (signature playstyle, boss archetypes
 *                       defeated, campaign-ending choices)
 *   Campaign Memory   — fades slowly (faction treatment, mission success
 *                       patterns, lethal biomes)
 *   Run Memory        — ephemeral (camping, weapon overuse, retreat patterns)
 *
 * Promotion rules — a memory becomes a Legend only if it:
 *   1. Appears across multiple runs
 *   2. Survives different biomes
 *   3. Meaningfully affects difficulty
 *
 * Explicitly forgotten by design:
 *   individual deaths, exact room layouts, specific weapon loadouts,
 *   RNG outcomes, momentary mistakes.
 *
 * Boss mutation (max 2 mutations per boss; never removes all weaknesses):
 *   High explosives  → Shield phasing
 *   High precision   → Moving weakpoints
 *   High stealth     → Area scans
 *   Aggression       → Counter-rush modules
 */

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <cstdint>

namespace atlas::gameplay {

// ── Player reputation profile ────────────────────────────────────────────────

struct PlayerReputation {
    float aggression    = 0.0f;  ///< 0–1
    float stealth       = 0.0f;  ///< 0–1
    float precision     = 0.0f;  ///< 0–1
    float authority     = 0.0f;  ///< 0–1 (fleet command usage)
    float chaos         = 0.0f;  ///< 0–1 (unpredictable play)

    void Clamp();
};

// ── Memory taxonomy ──────────────────────────────────────────────────────────

enum class MemoryTier : uint8_t {
    RUN,       ///< Ephemeral — cleared at run end
    CAMPAIGN,  ///< Fades slowly — persists across runs in a campaign
    LEGEND     ///< Never forgotten — shapes world generation
};

inline const char* MemoryTierName(MemoryTier t) {
    switch (t) {
        case MemoryTier::RUN:      return "Run";
        case MemoryTier::CAMPAIGN: return "Campaign";
        case MemoryTier::LEGEND:   return "Legend";
    }
    return "Unknown";
}

struct LegendMemory {
    std::string  key;              ///< e.g. "high_explosives", "precision_sniper"
    MemoryTier   tier = MemoryTier::RUN;
    float        strength   = 0.0f; ///< 0–1; grows across runs
    int          runCount   = 0;    ///< number of runs this appeared in
    int          biomeCount = 0;    ///< distinct biomes this survived in
    std::string  description;
};

// ── Boss mutation ────────────────────────────────────────────────────────────

enum class BossMutation : uint8_t {
    SHIELD_PHASING,     ///< Response to high explosives
    MOVING_WEAKPOINTS,  ///< Response to high precision
    AREA_SCANS,         ///< Response to high stealth
    COUNTER_RUSH        ///< Response to high aggression
};

inline const char* BossMutationName(BossMutation m) {
    switch (m) {
        case BossMutation::SHIELD_PHASING:    return "Shield Phasing";
        case BossMutation::MOVING_WEAKPOINTS: return "Moving Weakpoints";
        case BossMutation::AREA_SCANS:        return "Area Scans";
        case BossMutation::COUNTER_RUSH:      return "Counter Rush";
    }
    return "Unknown";
}

// ── LegendSystem ─────────────────────────────────────────────────────────────

/**
 * Tracks player reputation, manages memory tier promotion, and derives
 * boss mutations from accumulated legend pressure.
 */
class LegendSystem {
public:
    LegendSystem() = default;

    // ── Reputation ────────────────────────────────────────────────────

    /** Apply a reputation delta from a player action. */
    void RecordAction(float aggression, float stealth,
                      float precision, float authority, float chaos);

    const PlayerReputation& Reputation() const { return m_reputation; }

    // ── Memory management ─────────────────────────────────────────────

    /**
     * Record a memory event.
     * @param key        Unique behaviour key.
     * @param tier       Starting memory tier (usually RUN).
     * @param strength   How strongly this event expressed (0–1).
     * @param biomeID    Biome where this occurred.
     */
    void RecordMemory(const std::string& key, MemoryTier tier,
                      float strength, const std::string& biomeID = "");

    /**
     * End-of-run promotion pass — eligible RUN memories may become CAMPAIGN;
     * eligible CAMPAIGN memories may become LEGEND.
     * @param runsCompleted Total runs finished so far.
     */
    void PromoteMemories(int runsCompleted);

    /** Clear all RUN-tier memories (called at run end before promotion). */
    void ClearRunMemories();

    const std::vector<LegendMemory>& Memories() const { return m_memories; }
    int MemoryCount(MemoryTier tier) const;

    /** Retrieve a memory by key (nullptr if not found). */
    const LegendMemory* FindMemory(const std::string& key) const;

    // ── Boss mutations ────────────────────────────────────────────────

    /**
     * Derive active boss mutations from current reputation.
     * Maximum 2 mutations returned.
     */
    std::vector<BossMutation> DeriveBossMutations() const;

    // ── Constants ─────────────────────────────────────────────────────

    static constexpr int   MIN_RUNS_FOR_CAMPAIGN = 2;  ///< runs before RUN→CAMPAIGN
    static constexpr int   MIN_RUNS_FOR_LEGEND   = 4;  ///< runs before CAMPAIGN→LEGEND
    static constexpr int   MIN_BIOMES_FOR_LEGEND  = 2;  ///< distinct biomes required
    static constexpr float LEGEND_STRENGTH_THRESHOLD = 0.6f;
    static constexpr float MUTATION_THRESHOLD = 0.65f; ///< reputation threshold
    static constexpr int   MAX_BOSS_MUTATIONS = 2;

private:
    PlayerReputation           m_reputation;
    std::vector<LegendMemory>  m_memories;
    std::unordered_map<std::string, int> m_biomesSeen; ///< key → distinct biome count
};

} // namespace atlas::gameplay
