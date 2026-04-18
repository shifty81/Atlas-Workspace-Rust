#pragma once
// NF::Editor — World partition zone editor
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

enum class WorldPartitionCellSize : uint8_t {
    Small, Medium, Large, Custom
};

inline const char* worldPartitionCellSizeName(WorldPartitionCellSize s) {
    switch (s) {
        case WorldPartitionCellSize::Small:  return "Small";
        case WorldPartitionCellSize::Medium: return "Medium";
        case WorldPartitionCellSize::Large:  return "Large";
        case WorldPartitionCellSize::Custom: return "Custom";
    }
    return "Unknown";
}

enum class WorldPartitionLoadStrategy : uint8_t {
    Distance, Region, AlwaysLoaded, DataLayer, Custom
};

inline const char* worldPartitionLoadStrategyName(WorldPartitionLoadStrategy s) {
    switch (s) {
        case WorldPartitionLoadStrategy::Distance:     return "Distance";
        case WorldPartitionLoadStrategy::Region:       return "Region";
        case WorldPartitionLoadStrategy::AlwaysLoaded: return "AlwaysLoaded";
        case WorldPartitionLoadStrategy::DataLayer:    return "DataLayer";
        case WorldPartitionLoadStrategy::Custom:       return "Custom";
    }
    return "Unknown";
}

enum class WorldPartitionRegionState : uint8_t {
    Active, Inactive, Loading, Unloading, Error
};

inline const char* worldPartitionRegionStateName(WorldPartitionRegionState s) {
    switch (s) {
        case WorldPartitionRegionState::Active:    return "Active";
        case WorldPartitionRegionState::Inactive:  return "Inactive";
        case WorldPartitionRegionState::Loading:   return "Loading";
        case WorldPartitionRegionState::Unloading: return "Unloading";
        case WorldPartitionRegionState::Error:     return "Error";
    }
    return "Unknown";
}

class WorldPartitionRegion {
public:
    explicit WorldPartitionRegion(const std::string& name)
        : m_name(name) {}

    void setState(WorldPartitionRegionState s)          { m_state        = s; }
    void setLoadStrategy(WorldPartitionLoadStrategy s)  { m_loadStrategy = s; }
    void setCellSize(WorldPartitionCellSize sz)         { m_cellSize     = sz; }
    void setExtentX(float x)                            { m_extentX      = x; }
    void setExtentY(float y)                            { m_extentY      = y; }
    void setEnabled(bool v)                             { m_enabled      = v; }

    [[nodiscard]] const std::string&         name()         const { return m_name;        }
    [[nodiscard]] WorldPartitionRegionState  state()        const { return m_state;       }
    [[nodiscard]] WorldPartitionLoadStrategy loadStrategy() const { return m_loadStrategy;}
    [[nodiscard]] WorldPartitionCellSize     cellSize()     const { return m_cellSize;    }
    [[nodiscard]] float                      extentX()      const { return m_extentX;     }
    [[nodiscard]] float                      extentY()      const { return m_extentY;     }
    [[nodiscard]] bool                       isEnabled()    const { return m_enabled;     }
    [[nodiscard]] bool                       isActive()     const {
        return m_state == WorldPartitionRegionState::Active;
    }

private:
    std::string                  m_name;
    WorldPartitionRegionState    m_state        = WorldPartitionRegionState::Inactive;
    WorldPartitionLoadStrategy   m_loadStrategy = WorldPartitionLoadStrategy::Distance;
    WorldPartitionCellSize       m_cellSize     = WorldPartitionCellSize::Medium;
    float                        m_extentX      = 1000.0f;
    float                        m_extentY      = 1000.0f;
    bool                         m_enabled      = true;
};

class WorldPartition {
public:
    static constexpr size_t MAX_REGIONS = 128;

    [[nodiscard]] bool addRegion(const WorldPartitionRegion& region) {
        for (auto& r : m_regions) if (r.name() == region.name()) return false;
        if (m_regions.size() >= MAX_REGIONS) return false;
        m_regions.push_back(region);
        return true;
    }

    [[nodiscard]] bool removeRegion(const std::string& name) {
        for (auto it = m_regions.begin(); it != m_regions.end(); ++it) {
            if (it->name() == name) { m_regions.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] WorldPartitionRegion* findRegion(const std::string& name) {
        for (auto& r : m_regions) if (r.name() == name) return &r;
        return nullptr;
    }

    [[nodiscard]] size_t regionCount()  const { return m_regions.size(); }
    [[nodiscard]] size_t activeCount()  const {
        size_t c = 0; for (auto& r : m_regions) if (r.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t countByState(WorldPartitionRegionState s) const {
        size_t c = 0; for (auto& r : m_regions) if (r.state() == s) ++c; return c;
    }
    [[nodiscard]] size_t countByLoadStrategy(WorldPartitionLoadStrategy s) const {
        size_t c = 0; for (auto& r : m_regions) if (r.loadStrategy() == s) ++c; return c;
    }

private:
    std::vector<WorldPartitionRegion> m_regions;
};

} // namespace NF
