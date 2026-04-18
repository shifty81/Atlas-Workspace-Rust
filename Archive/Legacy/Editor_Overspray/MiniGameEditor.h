#pragma once
// NF::Editor — mini game editor
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

enum class MiniGameType : uint8_t {
    Racing, Shooting, Fishing, Cooking, Dancing, Memory, Crafting, Platformer
};

inline const char* miniGameTypeName(MiniGameType t) {
    switch (t) {
        case MiniGameType::Racing:     return "Racing";
        case MiniGameType::Shooting:   return "Shooting";
        case MiniGameType::Fishing:    return "Fishing";
        case MiniGameType::Cooking:    return "Cooking";
        case MiniGameType::Dancing:    return "Dancing";
        case MiniGameType::Memory:     return "Memory";
        case MiniGameType::Crafting:   return "Crafting";
        case MiniGameType::Platformer: return "Platformer";
    }
    return "Unknown";
}

enum class MiniGameDifficulty : uint8_t {
    VeryEasy, Easy, Normal, Hard, VeryHard
};

inline const char* miniGameDifficultyName(MiniGameDifficulty d) {
    switch (d) {
        case MiniGameDifficulty::VeryEasy: return "VeryEasy";
        case MiniGameDifficulty::Easy:     return "Easy";
        case MiniGameDifficulty::Normal:   return "Normal";
        case MiniGameDifficulty::Hard:     return "Hard";
        case MiniGameDifficulty::VeryHard: return "VeryHard";
    }
    return "Unknown";
}

enum class MiniGameTrigger : uint8_t {
    Manual, NPC, Zone, Item, Event, Timer
};

inline const char* miniGameTriggerName(MiniGameTrigger t) {
    switch (t) {
        case MiniGameTrigger::Manual: return "Manual";
        case MiniGameTrigger::NPC:    return "NPC";
        case MiniGameTrigger::Zone:   return "Zone";
        case MiniGameTrigger::Item:   return "Item";
        case MiniGameTrigger::Event:  return "Event";
        case MiniGameTrigger::Timer:  return "Timer";
    }
    return "Unknown";
}

class MiniGame {
public:
    explicit MiniGame(uint32_t id, const std::string& name, MiniGameType type)
        : m_id(id), m_name(name), m_type(type) {}

    void setDifficulty(MiniGameDifficulty v) { m_difficulty   = v; }
    void setTrigger(MiniGameTrigger v)       { m_trigger      = v; }
    void setTimeLimit(float v)               { m_timeLimit    = v; }
    void setRepeatable(bool v)               { m_isRepeatable = v; }
    void setOptional(bool v)                 { m_isOptional   = v; }

    [[nodiscard]] uint32_t            id()           const { return m_id;          }
    [[nodiscard]] const std::string&  name()         const { return m_name;        }
    [[nodiscard]] MiniGameType        type()         const { return m_type;        }
    [[nodiscard]] MiniGameDifficulty  difficulty()   const { return m_difficulty;  }
    [[nodiscard]] MiniGameTrigger     trigger()      const { return m_trigger;     }
    [[nodiscard]] float               timeLimit()    const { return m_timeLimit;   }
    [[nodiscard]] bool                isRepeatable() const { return m_isRepeatable;}
    [[nodiscard]] bool                isOptional()   const { return m_isOptional;  }

private:
    uint32_t         m_id;
    std::string      m_name;
    MiniGameType     m_type;
    MiniGameDifficulty m_difficulty   = MiniGameDifficulty::Normal;
    MiniGameTrigger    m_trigger      = MiniGameTrigger::Manual;
    float              m_timeLimit    = 60.0f;
    bool               m_isRepeatable = true;
    bool               m_isOptional   = true;
};

class MiniGameEditor {
public:
    void setShowBounds(bool v)       { m_showBounds   = v; }
    void setShowTriggers(bool v)     { m_showTriggers = v; }
    void setPreviewScale(float v)    { m_previewScale = v; }

    bool addGame(const MiniGame& g) {
        for (auto& e : m_games) if (e.id() == g.id()) return false;
        m_games.push_back(g); return true;
    }
    bool removeGame(uint32_t id) {
        auto it = std::find_if(m_games.begin(), m_games.end(),
            [&](const MiniGame& e){ return e.id() == id; });
        if (it == m_games.end()) return false;
        m_games.erase(it); return true;
    }
    [[nodiscard]] MiniGame* findGame(uint32_t id) {
        for (auto& e : m_games) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] bool   isShowBounds()   const { return m_showBounds;     }
    [[nodiscard]] bool   isShowTriggers() const { return m_showTriggers;   }
    [[nodiscard]] float  previewScale()   const { return m_previewScale;   }
    [[nodiscard]] size_t gameCount()      const { return m_games.size();   }

    [[nodiscard]] size_t countByType(MiniGameType t) const {
        size_t c = 0; for (auto& e : m_games) if (e.type() == t) ++c; return c;
    }
    [[nodiscard]] size_t countByDifficulty(MiniGameDifficulty d) const {
        size_t c = 0; for (auto& e : m_games) if (e.difficulty() == d) ++c; return c;
    }
    [[nodiscard]] size_t countRepeatable() const {
        size_t c = 0; for (auto& e : m_games) if (e.isRepeatable()) ++c; return c;
    }

private:
    std::vector<MiniGame> m_games;
    bool  m_showBounds   = true;
    bool  m_showTriggers = true;
    float m_previewScale = 1.0f;
};

} // namespace NF
