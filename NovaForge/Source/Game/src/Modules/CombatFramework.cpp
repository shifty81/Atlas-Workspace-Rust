#include "CombatFramework.h"
#include <algorithm>

namespace atlas::gameplay {

void CombatFramework::RegisterUnit(uint32_t entityID, const CombatStats& stats) {
    m_units[entityID] = stats;
}

void CombatFramework::RemoveUnit(uint32_t entityID) {
    m_units.erase(entityID);
}

bool CombatFramework::HasUnit(uint32_t entityID) const {
    return m_units.count(entityID) > 0;
}

const CombatStats* CombatFramework::GetStats(uint32_t entityID) const {
    auto it = m_units.find(entityID);
    return it != m_units.end() ? &it->second : nullptr;
}

CombatStats* CombatFramework::GetStatsMutable(uint32_t entityID) {
    auto it = m_units.find(entityID);
    return it != m_units.end() ? &it->second : nullptr;
}

size_t CombatFramework::UnitCount() const {
    return m_units.size();
}

CombatResult CombatFramework::ResolveCombat(uint32_t attackerID, uint32_t defenderID) {
    CombatResult result;

    auto atkIt = m_units.find(attackerID);
    auto defIt = m_units.find(defenderID);
    if (atkIt == m_units.end() || defIt == m_units.end()) {
        return result;
    }

    float effectiveDamage = std::max(0.0f, atkIt->second.damage - defIt->second.armor);
    defIt->second.health -= effectiveDamage;
    result.damageDealt = effectiveDamage;
    result.targetDestroyed = defIt->second.health <= 0.0f;
    return result;
}

void CombatFramework::Clear() {
    m_units.clear();
}

} // namespace atlas::gameplay
