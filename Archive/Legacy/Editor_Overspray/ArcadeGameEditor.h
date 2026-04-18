#pragma once
// NF::Editor — arcade game editor
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

enum class ArcadeGameGenre : uint8_t {
    Shooter, Platformer, Puzzle, Fighting, Racing, Sports, RPG, Survival
};

inline const char* arcadeGameGenreName(ArcadeGameGenre g) {
    switch (g) {
        case ArcadeGameGenre::Shooter:    return "Shooter";
        case ArcadeGameGenre::Platformer: return "Platformer";
        case ArcadeGameGenre::Puzzle:     return "Puzzle";
        case ArcadeGameGenre::Fighting:   return "Fighting";
        case ArcadeGameGenre::Racing:     return "Racing";
        case ArcadeGameGenre::Sports:     return "Sports";
        case ArcadeGameGenre::RPG:        return "RPG";
        case ArcadeGameGenre::Survival:   return "Survival";
    }
    return "Unknown";
}

enum class ArcadeControlScheme : uint8_t {
    Keyboard, Gamepad, Touch, Motion, Combined
};

inline const char* arcadeControlSchemeName(ArcadeControlScheme c) {
    switch (c) {
        case ArcadeControlScheme::Keyboard: return "Keyboard";
        case ArcadeControlScheme::Gamepad:  return "Gamepad";
        case ArcadeControlScheme::Touch:    return "Touch";
        case ArcadeControlScheme::Motion:   return "Motion";
        case ArcadeControlScheme::Combined: return "Combined";
    }
    return "Unknown";
}

enum class ArcadeScoreMode : uint8_t {
    Points, Time, Combo, Survival
};

inline const char* arcadeScoreModeName(ArcadeScoreMode m) {
    switch (m) {
        case ArcadeScoreMode::Points:   return "Points";
        case ArcadeScoreMode::Time:     return "Time";
        case ArcadeScoreMode::Combo:    return "Combo";
        case ArcadeScoreMode::Survival: return "Survival";
    }
    return "Unknown";
}

class ArcadeConfig {
public:
    explicit ArcadeConfig(uint32_t id, const std::string& name, ArcadeGameGenre genre)
        : m_id(id), m_name(name), m_genre(genre) {}

    void setControlScheme(ArcadeControlScheme v) { m_controlScheme      = v; }
    void setScoreMode(ArcadeScoreMode v)         { m_scoreMode          = v; }
    void setLives(uint32_t v)                    { m_lives              = v; }
    void setHighScoreEnabled(bool v)             { m_isHighScoreEnabled = v; }
    void setTwoPlayer(bool v)                    { m_isTwoPlayer        = v; }

    [[nodiscard]] uint32_t            id()                  const { return m_id;                 }
    [[nodiscard]] const std::string&  name()                const { return m_name;               }
    [[nodiscard]] ArcadeGameGenre     genre()               const { return m_genre;              }
    [[nodiscard]] ArcadeControlScheme controlScheme()       const { return m_controlScheme;      }
    [[nodiscard]] ArcadeScoreMode     scoreMode()           const { return m_scoreMode;          }
    [[nodiscard]] uint32_t            lives()               const { return m_lives;              }
    [[nodiscard]] bool                isHighScoreEnabled()  const { return m_isHighScoreEnabled; }
    [[nodiscard]] bool                isTwoPlayer()         const { return m_isTwoPlayer;        }

private:
    uint32_t           m_id;
    std::string        m_name;
    ArcadeGameGenre    m_genre;
    ArcadeControlScheme m_controlScheme      = ArcadeControlScheme::Gamepad;
    ArcadeScoreMode    m_scoreMode           = ArcadeScoreMode::Points;
    uint32_t           m_lives              = 3u;
    bool               m_isHighScoreEnabled = true;
    bool               m_isTwoPlayer        = false;
};

class ArcadeGameEditor {
public:
    void setShowHitboxes(bool v)       { m_showHitboxes    = v; }
    void setShowSpawnPoints(bool v)    { m_showSpawnPoints = v; }
    void setSimulationSpeed(float v)   { m_simulationSpeed = v; }

    bool addConfig(const ArcadeConfig& c) {
        for (auto& e : m_configs) if (e.id() == c.id()) return false;
        m_configs.push_back(c); return true;
    }
    bool removeConfig(uint32_t id) {
        auto it = std::find_if(m_configs.begin(), m_configs.end(),
            [&](const ArcadeConfig& e){ return e.id() == id; });
        if (it == m_configs.end()) return false;
        m_configs.erase(it); return true;
    }
    [[nodiscard]] ArcadeConfig* findConfig(uint32_t id) {
        for (auto& e : m_configs) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] bool   isShowHitboxes()    const { return m_showHitboxes;    }
    [[nodiscard]] bool   isShowSpawnPoints() const { return m_showSpawnPoints; }
    [[nodiscard]] float  simulationSpeed()   const { return m_simulationSpeed; }
    [[nodiscard]] size_t configCount()       const { return m_configs.size();  }

    [[nodiscard]] size_t countByGenre(ArcadeGameGenre g) const {
        size_t c = 0; for (auto& e : m_configs) if (e.genre() == g) ++c; return c;
    }
    [[nodiscard]] size_t countByControlScheme(ArcadeControlScheme cs) const {
        size_t c = 0; for (auto& e : m_configs) if (e.controlScheme() == cs) ++c; return c;
    }
    [[nodiscard]] size_t countTwoPlayer() const {
        size_t c = 0; for (auto& e : m_configs) if (e.isTwoPlayer()) ++c; return c;
    }

private:
    std::vector<ArcadeConfig> m_configs;
    bool  m_showHitboxes    = false;
    bool  m_showSpawnPoints = true;
    float m_simulationSpeed = 1.0f;
};

} // namespace NF
