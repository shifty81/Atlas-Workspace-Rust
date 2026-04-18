#pragma once
/**
 * @file fps_enemy.h
 * @brief FPS enemy archetypes and squad role definitions.
 *
 * Three enemy archetypes:
 *   Grunt      — pressure, advance–cover–fire loop
 *   Breacher   — flush, push aggressively, break stalemates
 *   Technician — support, deploy turret, repair, retreat when shot
 *
 * Squad roles define how individual enemies coordinate:
 *   Breacher   — rushes doors, throws opening attack
 *   Suppressor — anchors sightlines, forces the player into cover
 *   Flanker    — delayed entry, punishes tunnel vision
 */

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace atlas::gameplay {

// ── Enemy archetype ──────────────────────────────────────────────────────────

enum class EnemyArchetype : uint8_t {
    GRUNT,       ///< Raider grunt — pressure
    BREACHER,    ///< Breacher — flush / break stalemates
    TECHNICIAN   ///< Technician — support / turret / repair
};

inline const char* EnemyArchetypeName(EnemyArchetype t) {
    switch (t) {
        case EnemyArchetype::GRUNT:       return "Grunt";
        case EnemyArchetype::BREACHER:    return "Breacher";
        case EnemyArchetype::TECHNICIAN:  return "Technician";
    }
    return "Unknown";
}

// ── Squad role ───────────────────────────────────────────────────────────────

enum class SquadRole : uint8_t {
    BREACHER,    ///< Rushes doors, throws opening attack
    SUPPRESSOR,  ///< Anchors sightlines, forces cover
    FLANKER      ///< Delayed entry, punishes tunnel vision
};

inline const char* SquadRoleName(SquadRole r) {
    switch (r) {
        case SquadRole::BREACHER:   return "Breacher";
        case SquadRole::SUPPRESSOR: return "Suppressor";
        case SquadRole::FLANKER:    return "Flanker";
    }
    return "Unknown";
}

// ── Enemy stats ──────────────────────────────────────────────────────────────

struct EnemyStats {
    float health       = 100.0f;
    float moveSpeed    = 3.5f;   ///< m/s
    float attackDamage = 15.0f;
    float attackRange  = 20.0f;  ///< metres
    float detectionRange = 30.0f;
    bool  hasShield    = false;
    float shieldHP     = 0.0f;
};

inline EnemyStats DefaultStatsForArchetype(EnemyArchetype archetype) {
    switch (archetype) {
        case EnemyArchetype::GRUNT:
            return { 80.0f, 3.5f, 12.0f, 25.0f, 30.0f, false, 0.0f };
        case EnemyArchetype::BREACHER:
            return { 120.0f, 5.0f, 20.0f, 8.0f, 20.0f, true, 40.0f };
        case EnemyArchetype::TECHNICIAN:
            return { 60.0f, 2.5f, 8.0f, 30.0f, 35.0f, false, 0.0f };
    }
    return {};
}

// ── Enemy AI state ───────────────────────────────────────────────────────────

enum class EnemyAIState : uint8_t {
    IDLE,
    PATROLLING,
    INVESTIGATING,
    ENGAGING,
    RETREATING,
    DEPLOYING_TURRET,  ///< Technician only
    REPAIRING          ///< Technician only
};

// ── FPS enemy entity ─────────────────────────────────────────────────────────

struct FPSEnemy {
    std::string    id;
    EnemyArchetype archetype = EnemyArchetype::GRUNT;
    SquadRole      squadRole = SquadRole::SUPPRESSOR;
    EnemyStats     stats;
    EnemyAIState   aiState   = EnemyAIState::IDLE;
    std::string    squadID;

    bool IsAlive() const { return stats.health > 0.0f; }
};

// ── FPS enemy squad ──────────────────────────────────────────────────────────

/**
 * A small squad of 2–4 enemies with coordinated roles.
 *
 * Squad entry rules:
 *   - Exactly one Breacher (or none in early rooms)
 *   - 1–2 Suppressors to anchor lines
 *   - 0–1 Flanker (delayed entry, second wave)
 *   - Optional Technician for support squads
 */
struct EnemySquad {
    std::string            id;
    std::vector<FPSEnemy>  members;

    /** Create a standard 3-member squad for the given encounter difficulty. */
    static EnemySquad CreateStandard(const std::string& squadID, int difficulty = 1);

    /** Create a technician-backed support squad. */
    static EnemySquad CreateSupport(const std::string& squadID);

    int AliveCount() const;
    bool AllDown() const { return AliveCount() == 0; }

    void ApplyDamage(const std::string& memberID, float dmg);
    FPSEnemy* FindMember(const std::string& memberID);
};

} // namespace atlas::gameplay
