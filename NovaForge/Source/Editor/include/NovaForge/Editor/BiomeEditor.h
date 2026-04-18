#pragma once
// NF::Editor — biome editor
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

enum class BiomeType : uint8_t {
    Forest, Desert, Tundra, Swamp, Ocean, Mountain, Plains, Volcanic, Magical, Urban
};

inline const char* biomeTypeName(BiomeType t) {
    switch (t) {
        case BiomeType::Forest:   return "Forest";
        case BiomeType::Desert:   return "Desert";
        case BiomeType::Tundra:   return "Tundra";
        case BiomeType::Swamp:    return "Swamp";
        case BiomeType::Ocean:    return "Ocean";
        case BiomeType::Mountain: return "Mountain";
        case BiomeType::Plains:   return "Plains";
        case BiomeType::Volcanic: return "Volcanic";
        case BiomeType::Magical:  return "Magical";
        case BiomeType::Urban:    return "Urban";
    }
    return "Unknown";
}

enum class BiomeTemperature : uint8_t {
    Freezing, Cold, Temperate, Warm, Hot, Scorching
};

inline const char* biomeTemperatureName(BiomeTemperature t) {
    switch (t) {
        case BiomeTemperature::Freezing:  return "Freezing";
        case BiomeTemperature::Cold:      return "Cold";
        case BiomeTemperature::Temperate: return "Temperate";
        case BiomeTemperature::Warm:      return "Warm";
        case BiomeTemperature::Hot:       return "Hot";
        case BiomeTemperature::Scorching: return "Scorching";
    }
    return "Unknown";
}

enum class BiomeMoistureLevel : uint8_t {
    Arid, Dry, Moderate, Humid, Saturated
};

inline const char* biomeMoistureLevelName(BiomeMoistureLevel m) {
    switch (m) {
        case BiomeMoistureLevel::Arid:      return "Arid";
        case BiomeMoistureLevel::Dry:       return "Dry";
        case BiomeMoistureLevel::Moderate:  return "Moderate";
        case BiomeMoistureLevel::Humid:     return "Humid";
        case BiomeMoistureLevel::Saturated: return "Saturated";
    }
    return "Unknown";
}

class BiomeEntry {
public:
    explicit BiomeEntry(uint32_t id, const std::string& name, BiomeType type)
        : m_id(id), m_name(name), m_type(type) {}

    void setTemperature(BiomeTemperature v)  { m_temperature = v; }
    void setMoisture(BiomeMoistureLevel v)   { m_moisture = v; }
    void setElevation(float v)               { m_elevation = v; }
    void setUnderwater(bool v)               { m_isUnderwater = v; }

    [[nodiscard]] uint32_t            id()          const { return m_id; }
    [[nodiscard]] const std::string&  name()        const { return m_name; }
    [[nodiscard]] BiomeType           type()        const { return m_type; }
    [[nodiscard]] BiomeTemperature    temperature() const { return m_temperature; }
    [[nodiscard]] BiomeMoistureLevel  moisture()    const { return m_moisture; }
    [[nodiscard]] float               elevation()   const { return m_elevation; }
    [[nodiscard]] bool                isUnderwater() const { return m_isUnderwater; }

private:
    uint32_t           m_id;
    std::string        m_name;
    BiomeType          m_type        = BiomeType::Forest;
    BiomeTemperature   m_temperature = BiomeTemperature::Temperate;
    BiomeMoistureLevel m_moisture    = BiomeMoistureLevel::Moderate;
    float              m_elevation   = 0.0f;
    bool               m_isUnderwater = false;
};

class BiomeEditor {
public:
    bool addBiome(const BiomeEntry& biome) {
        for (const auto& b : m_biomes)
            if (b.id() == biome.id()) return false;
        m_biomes.push_back(biome);
        return true;
    }

    bool removeBiome(uint32_t id) {
        for (auto it = m_biomes.begin(); it != m_biomes.end(); ++it) {
            if (it->id() == id) { m_biomes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] BiomeEntry* findBiome(uint32_t id) {
        for (auto& b : m_biomes)
            if (b.id() == id) return &b;
        return nullptr;
    }

    [[nodiscard]] size_t biomeCount() const { return m_biomes.size(); }

    [[nodiscard]] size_t countByType(BiomeType t) const {
        size_t n = 0;
        for (const auto& b : m_biomes) if (b.type() == t) ++n;
        return n;
    }

    [[nodiscard]] size_t countByTemperature(BiomeTemperature t) const {
        size_t n = 0;
        for (const auto& b : m_biomes) if (b.temperature() == t) ++n;
        return n;
    }

    [[nodiscard]] size_t countUnderwater() const {
        size_t n = 0;
        for (const auto& b : m_biomes) if (b.isUnderwater()) ++n;
        return n;
    }

    void setShowTemperatureMap(bool v)    { m_showTemperatureMap = v; }
    void setShowMoistureMap(bool v)       { m_showMoistureMap = v; }
    void setShowElevationMap(bool v)      { m_showElevationMap = v; }
    void setActiveBlendRadius(float v)    { m_activeBlendRadius = v; }

    [[nodiscard]] bool  showTemperatureMap() const { return m_showTemperatureMap; }
    [[nodiscard]] bool  showMoistureMap()    const { return m_showMoistureMap; }
    [[nodiscard]] bool  showElevationMap()   const { return m_showElevationMap; }
    [[nodiscard]] float activeBlendRadius()  const { return m_activeBlendRadius; }

private:
    std::vector<BiomeEntry> m_biomes;
    bool  m_showTemperatureMap = false;
    bool  m_showMoistureMap    = false;
    bool  m_showElevationMap   = false;
    float m_activeBlendRadius  = 50.0f;
};

} // namespace NF
