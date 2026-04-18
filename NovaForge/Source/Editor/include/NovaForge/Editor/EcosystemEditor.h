#pragma once
// NF::Editor — ecosystem editor
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

enum class OrganismType : uint8_t {
    Flora, Fauna, Fungi, Aquatic, Insect, Undead, Construct, Spirit
};

inline const char* organismTypeName(OrganismType t) {
    switch (t) {
        case OrganismType::Flora:     return "Flora";
        case OrganismType::Fauna:     return "Fauna";
        case OrganismType::Fungi:     return "Fungi";
        case OrganismType::Aquatic:   return "Aquatic";
        case OrganismType::Insect:    return "Insect";
        case OrganismType::Undead:    return "Undead";
        case OrganismType::Construct: return "Construct";
        case OrganismType::Spirit:    return "Spirit";
    }
    return "Unknown";
}

enum class FoodChainRole : uint8_t {
    Producer, PrimaryConsumer, SecondaryConsumer, Decomposer, Apex
};

inline const char* foodChainRoleName(FoodChainRole r) {
    switch (r) {
        case FoodChainRole::Producer:          return "Producer";
        case FoodChainRole::PrimaryConsumer:   return "PrimaryConsumer";
        case FoodChainRole::SecondaryConsumer: return "SecondaryConsumer";
        case FoodChainRole::Decomposer:        return "Decomposer";
        case FoodChainRole::Apex:              return "Apex";
    }
    return "Unknown";
}

enum class SpawnRule : uint8_t {
    Cluster, Scattered, Lone, Pack, Seasonal, Underground
};

inline const char* spawnRuleName(SpawnRule r) {
    switch (r) {
        case SpawnRule::Cluster:     return "Cluster";
        case SpawnRule::Scattered:   return "Scattered";
        case SpawnRule::Lone:        return "Lone";
        case SpawnRule::Pack:        return "Pack";
        case SpawnRule::Seasonal:    return "Seasonal";
        case SpawnRule::Underground: return "Underground";
    }
    return "Unknown";
}

class Organism {
public:
    explicit Organism(uint32_t id, const std::string& name, OrganismType type)
        : m_id(id), m_name(name), m_type(type) {}

    void setRole(FoodChainRole v)         { m_role = v; }
    void setSpawnRule(SpawnRule v)        { m_spawnRule = v; }
    void setDensity(float v)             { m_density = v; }
    void setTerritoryRadius(float v)     { m_territoryRadius = v; }
    void setNocturnal(bool v)            { m_isNocturnal = v; }

    [[nodiscard]] uint32_t           id()              const { return m_id; }
    [[nodiscard]] const std::string& name()            const { return m_name; }
    [[nodiscard]] OrganismType       type()            const { return m_type; }
    [[nodiscard]] FoodChainRole      role()            const { return m_role; }
    [[nodiscard]] SpawnRule          spawnRule()       const { return m_spawnRule; }
    [[nodiscard]] float              density()         const { return m_density; }
    [[nodiscard]] float              territoryRadius() const { return m_territoryRadius; }
    [[nodiscard]] bool               isNocturnal()     const { return m_isNocturnal; }

private:
    uint32_t     m_id;
    std::string  m_name;
    OrganismType m_type            = OrganismType::Flora;
    FoodChainRole m_role           = FoodChainRole::Producer;
    SpawnRule    m_spawnRule       = SpawnRule::Scattered;
    float        m_density         = 1.0f;
    float        m_territoryRadius = 10.0f;
    bool         m_isNocturnal     = false;
};

class EcosystemEditor {
public:
    bool addOrganism(const Organism& organism) {
        for (const auto& o : m_organisms)
            if (o.id() == organism.id()) return false;
        m_organisms.push_back(organism);
        return true;
    }

    bool removeOrganism(uint32_t id) {
        for (auto it = m_organisms.begin(); it != m_organisms.end(); ++it) {
            if (it->id() == id) { m_organisms.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Organism* findOrganism(uint32_t id) {
        for (auto& o : m_organisms)
            if (o.id() == id) return &o;
        return nullptr;
    }

    [[nodiscard]] size_t organismCount() const { return m_organisms.size(); }

    [[nodiscard]] size_t countByType(OrganismType t) const {
        size_t n = 0;
        for (const auto& o : m_organisms) if (o.type() == t) ++n;
        return n;
    }

    [[nodiscard]] size_t countByRole(FoodChainRole r) const {
        size_t n = 0;
        for (const auto& o : m_organisms) if (o.role() == r) ++n;
        return n;
    }

    [[nodiscard]] size_t countNocturnal() const {
        size_t n = 0;
        for (const auto& o : m_organisms) if (o.isNocturnal()) ++n;
        return n;
    }

    void setShowFoodChain(bool v)    { m_isShowFoodChain = v; }
    void setShowTerritories(bool v)  { m_isShowTerritories = v; }
    void setShowSpawnZones(bool v)   { m_isShowSpawnZones = v; }
    void setSimulationSpeed(float v) { m_simulationSpeed = v; }

    [[nodiscard]] bool  isShowFoodChain()   const { return m_isShowFoodChain; }
    [[nodiscard]] bool  isShowTerritories() const { return m_isShowTerritories; }
    [[nodiscard]] bool  isShowSpawnZones()  const { return m_isShowSpawnZones; }
    [[nodiscard]] float simulationSpeed()   const { return m_simulationSpeed; }

private:
    std::vector<Organism> m_organisms;
    bool  m_isShowFoodChain   = true;
    bool  m_isShowTerritories = false;
    bool  m_isShowSpawnZones  = false;
    float m_simulationSpeed   = 1.0f;
};

} // namespace NF
