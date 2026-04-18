/**
 * @file fps_enemy.cpp
 * @brief FPS enemy squad factory implementations.
 */

#include "gameplay/fps_enemy.h"
#include <algorithm>

namespace atlas::gameplay {

// ── EnemySquad factory ───────────────────────────────────────────────────────

EnemySquad EnemySquad::CreateStandard(const std::string& squadID, int difficulty) {
    EnemySquad squad;
    squad.id = squadID;

    // Suppressor (anchor)
    FPSEnemy suppressor;
    suppressor.id        = squadID + "_sup0";
    suppressor.archetype = EnemyArchetype::GRUNT;
    suppressor.squadRole = SquadRole::SUPPRESSOR;
    suppressor.stats     = DefaultStatsForArchetype(EnemyArchetype::GRUNT);
    suppressor.squadID   = squadID;
    suppressor.stats.health *= (1.0f + 0.1f * (difficulty - 1));
    squad.members.push_back(suppressor);

    // Breacher (pressure)
    FPSEnemy breacher;
    breacher.id        = squadID + "_brch0";
    breacher.archetype = EnemyArchetype::BREACHER;
    breacher.squadRole = SquadRole::BREACHER;
    breacher.stats     = DefaultStatsForArchetype(EnemyArchetype::BREACHER);
    breacher.squadID   = squadID;
    breacher.stats.health *= (1.0f + 0.1f * (difficulty - 1));
    squad.members.push_back(breacher);

    // Flanker (second wave, only on difficulty ≥ 2)
    if (difficulty >= 2) {
        FPSEnemy flanker;
        flanker.id        = squadID + "_flnk0";
        flanker.archetype = EnemyArchetype::GRUNT;
        flanker.squadRole = SquadRole::FLANKER;
        flanker.stats     = DefaultStatsForArchetype(EnemyArchetype::GRUNT);
        flanker.squadID   = squadID;
        flanker.stats.moveSpeed *= 1.2f; // flankers are faster
        squad.members.push_back(flanker);
    }

    return squad;
}

EnemySquad EnemySquad::CreateSupport(const std::string& squadID) {
    EnemySquad squad;
    squad.id = squadID;

    // Two grunts + one technician
    for (int i = 0; i < 2; ++i) {
        FPSEnemy grunt;
        grunt.id        = squadID + "_g" + std::to_string(i);
        grunt.archetype = EnemyArchetype::GRUNT;
        grunt.squadRole = (i == 0) ? SquadRole::SUPPRESSOR : SquadRole::FLANKER;
        grunt.stats     = DefaultStatsForArchetype(EnemyArchetype::GRUNT);
        grunt.squadID   = squadID;
        squad.members.push_back(grunt);
    }

    FPSEnemy tech;
    tech.id        = squadID + "_tech0";
    tech.archetype = EnemyArchetype::TECHNICIAN;
    tech.squadRole = SquadRole::SUPPRESSOR;
    tech.stats     = DefaultStatsForArchetype(EnemyArchetype::TECHNICIAN);
    tech.squadID   = squadID;
    squad.members.push_back(tech);

    return squad;
}

int EnemySquad::AliveCount() const {
    int count = 0;
    for (const auto& m : members)
        if (m.IsAlive()) ++count;
    return count;
}

void EnemySquad::ApplyDamage(const std::string& memberID, float dmg) {
    FPSEnemy* member = FindMember(memberID);
    if (!member) return;
    member->stats.health = std::max(0.0f, member->stats.health - dmg);
    if (member->stats.health <= 0.0f && member->stats.shieldHP > 0.0f) {
        member->stats.shieldHP = std::max(0.0f,
                                          member->stats.shieldHP - dmg);
    }
}

FPSEnemy* EnemySquad::FindMember(const std::string& memberID) {
    for (auto& m : members)
        if (m.id == memberID) return &m;
    return nullptr;
}

} // namespace atlas::gameplay
