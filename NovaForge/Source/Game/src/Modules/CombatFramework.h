#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace atlas::gameplay {

struct CombatStats {
    float damage = 0.0f;
    float armor = 0.0f;
    float health = 100.0f;
    float speed = 1.0f;
};

struct CombatResult {
    float damageDealt = 0.0f;
    bool targetDestroyed = false;
};

class CombatFramework {
public:
    void RegisterUnit(uint32_t entityID, const CombatStats& stats);
    void RemoveUnit(uint32_t entityID);
    bool HasUnit(uint32_t entityID) const;
    const CombatStats* GetStats(uint32_t entityID) const;
    CombatStats* GetStatsMutable(uint32_t entityID);
    size_t UnitCount() const;

    CombatResult ResolveCombat(uint32_t attackerID, uint32_t defenderID);

    void Clear();

private:
    std::unordered_map<uint32_t, CombatStats> m_units;
};

} // namespace atlas::gameplay
