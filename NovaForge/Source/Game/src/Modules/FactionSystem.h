#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace atlas::gameplay {

using FactionID = uint32_t;

enum class FactionRelation {
    Neutral,
    Friendly,
    Hostile,
    Allied,
};

struct FactionDescriptor {
    FactionID id = 0;
    std::string name;
    std::string tag;
};

class FactionSystem {
public:
    FactionID RegisterFaction(const std::string& name, const std::string& tag = "");
    bool HasFaction(FactionID id) const;
    const FactionDescriptor* GetFaction(FactionID id) const;
    size_t FactionCount() const;

    void SetRelation(FactionID a, FactionID b, FactionRelation rel);
    FactionRelation GetRelation(FactionID a, FactionID b) const;

    void Clear();

private:
    std::unordered_map<FactionID, FactionDescriptor> m_factions;
    std::unordered_map<uint64_t, FactionRelation> m_relations;
    FactionID m_nextID = 1;

    static uint64_t PairKey(FactionID a, FactionID b);
};

} // namespace atlas::gameplay
