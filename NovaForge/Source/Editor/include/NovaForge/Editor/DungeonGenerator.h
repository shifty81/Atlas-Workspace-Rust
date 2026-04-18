#pragma once
// NF::Editor — Dungeon generator
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

enum class DungeonGenerationAlgorithm : uint8_t {
    BSP, CellularAutomata, RandomWalk, Prefab, Voronoi, WaveFunctionCollapse
};

inline const char* dungeonGenerationAlgorithmName(DungeonGenerationAlgorithm a) {
    switch (a) {
        case DungeonGenerationAlgorithm::BSP:                  return "BSP";
        case DungeonGenerationAlgorithm::CellularAutomata:     return "CellularAutomata";
        case DungeonGenerationAlgorithm::RandomWalk:           return "RandomWalk";
        case DungeonGenerationAlgorithm::Prefab:               return "Prefab";
        case DungeonGenerationAlgorithm::Voronoi:              return "Voronoi";
        case DungeonGenerationAlgorithm::WaveFunctionCollapse: return "WaveFunctionCollapse";
    }
    return "Unknown";
}

enum class DungeonTheme : uint8_t {
    Cave, Castle, Ruins, Sewer, Forest, Temple, Tech, Cosmic
};

inline const char* dungeonThemeName(DungeonTheme t) {
    switch (t) {
        case DungeonTheme::Cave:    return "Cave";
        case DungeonTheme::Castle:  return "Castle";
        case DungeonTheme::Ruins:   return "Ruins";
        case DungeonTheme::Sewer:   return "Sewer";
        case DungeonTheme::Forest:  return "Forest";
        case DungeonTheme::Temple:  return "Temple";
        case DungeonTheme::Tech:    return "Tech";
        case DungeonTheme::Cosmic:  return "Cosmic";
    }
    return "Unknown";
}

enum class DungeonGenerationState : uint8_t {
    Idle, Generating, Validating, Complete, Failed
};

inline const char* dungeonGenerationStateName(DungeonGenerationState s) {
    switch (s) {
        case DungeonGenerationState::Idle:       return "Idle";
        case DungeonGenerationState::Generating: return "Generating";
        case DungeonGenerationState::Validating: return "Validating";
        case DungeonGenerationState::Complete:   return "Complete";
        case DungeonGenerationState::Failed:     return "Failed";
    }
    return "Unknown";
}

class DungeonGeneratorParams {
public:
    void setSeed(uint64_t v)          { m_seed      = v; }
    void setWidth(uint32_t v)         { m_width     = v; }
    void setHeight(uint32_t v)        { m_height    = v; }
    void setMinRooms(uint32_t v)      { m_minRooms  = v; }
    void setMaxRooms(uint32_t v)      { m_maxRooms  = v; }
    void setCorridorWidth(uint32_t v) { m_corridorW = v; }
    void setDensity(float v)          { m_density   = v; }

    [[nodiscard]] uint64_t  seed()          const { return m_seed;      }
    [[nodiscard]] uint32_t  width()         const { return m_width;     }
    [[nodiscard]] uint32_t  height()        const { return m_height;    }
    [[nodiscard]] uint32_t  minRooms()      const { return m_minRooms;  }
    [[nodiscard]] uint32_t  maxRooms()      const { return m_maxRooms;  }
    [[nodiscard]] uint32_t  corridorWidth() const { return m_corridorW; }
    [[nodiscard]] float     density()       const { return m_density;   }

private:
    uint64_t m_seed      = 0;
    uint32_t m_width     = 64;
    uint32_t m_height    = 64;
    uint32_t m_minRooms  = 5;
    uint32_t m_maxRooms  = 20;
    uint32_t m_corridorW = 2;
    float    m_density   = 0.5f;
};

class DungeonGenerator {
public:
    void setAlgorithm(DungeonGenerationAlgorithm a) { m_algorithm = a; }
    void setTheme(DungeonTheme t)                   { m_theme     = t; }
    void setState(DungeonGenerationState s)         { m_state     = s; }
    void setParams(const DungeonGeneratorParams& p) { m_params    = p; }
    void setGeneratedRoomCount(uint32_t v)          { m_generatedRooms = v; }
    void setEnsureConnected(bool v)                 { m_ensureConnected = v; }
    void setPlaceBossRoom(bool v)                   { m_placeBossRoom = v; }

    [[nodiscard]] DungeonGenerationAlgorithm algorithm()         const { return m_algorithm;       }
    [[nodiscard]] DungeonTheme               theme()            const { return m_theme;           }
    [[nodiscard]] DungeonGenerationState     state()            const { return m_state;           }
    [[nodiscard]] const DungeonGeneratorParams& params()        const { return m_params;          }
    [[nodiscard]] uint32_t                   generatedRoomCount()const{ return m_generatedRooms;  }
    [[nodiscard]] bool                       isEnsureConnected() const{ return m_ensureConnected; }
    [[nodiscard]] bool                       isPlaceBossRoom()   const{ return m_placeBossRoom;   }

    [[nodiscard]] bool isComplete() const { return m_state == DungeonGenerationState::Complete; }
    [[nodiscard]] bool isFailed()   const { return m_state == DungeonGenerationState::Failed;   }

private:
    DungeonGenerationAlgorithm m_algorithm        = DungeonGenerationAlgorithm::BSP;
    DungeonTheme               m_theme            = DungeonTheme::Cave;
    DungeonGenerationState     m_state            = DungeonGenerationState::Idle;
    DungeonGeneratorParams     m_params;
    uint32_t                   m_generatedRooms   = 0;
    bool                       m_ensureConnected  = true;
    bool                       m_placeBossRoom    = true;
};

} // namespace NF
