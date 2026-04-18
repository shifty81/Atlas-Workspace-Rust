#include "FactionSystem.h"
#include <algorithm>

namespace atlas::gameplay {

FactionID FactionSystem::RegisterFaction(const std::string& name, const std::string& tag) {
    FactionDescriptor desc;
    desc.id = m_nextID++;
    desc.name = name;
    desc.tag = tag.empty() ? name : tag;
    m_factions[desc.id] = desc;
    return desc.id;
}

bool FactionSystem::HasFaction(FactionID id) const {
    return m_factions.count(id) > 0;
}

const FactionDescriptor* FactionSystem::GetFaction(FactionID id) const {
    auto it = m_factions.find(id);
    return it != m_factions.end() ? &it->second : nullptr;
}

size_t FactionSystem::FactionCount() const {
    return m_factions.size();
}

void FactionSystem::SetRelation(FactionID a, FactionID b, FactionRelation rel) {
    m_relations[PairKey(a, b)] = rel;
}

FactionRelation FactionSystem::GetRelation(FactionID a, FactionID b) const {
    auto it = m_relations.find(PairKey(a, b));
    return it != m_relations.end() ? it->second : FactionRelation::Neutral;
}

void FactionSystem::Clear() {
    m_factions.clear();
    m_relations.clear();
    m_nextID = 1;
}

uint64_t FactionSystem::PairKey(FactionID a, FactionID b) {
    FactionID lo = std::min(a, b);
    FactionID hi = std::max(a, b);
    return (static_cast<uint64_t>(lo) << 32) | hi;
}

} // namespace atlas::gameplay
