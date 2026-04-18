#pragma once
// NF::Editor — daily quest management editor
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

enum class DailyQuestDifficulty : uint8_t { Trivial, Easy, Medium, Hard, Epic };
inline const char* dailyQuestDifficultyName(DailyQuestDifficulty v) {
    switch (v) {
        case DailyQuestDifficulty::Trivial: return "Trivial";
        case DailyQuestDifficulty::Easy:    return "Easy";
        case DailyQuestDifficulty::Medium:  return "Medium";
        case DailyQuestDifficulty::Hard:    return "Hard";
        case DailyQuestDifficulty::Epic:    return "Epic";
    }
    return "Unknown";
}

enum class DailyQuestStatus : uint8_t { Locked, Available, Active, Completed, Expired };
inline const char* dailyQuestStatusName(DailyQuestStatus v) {
    switch (v) {
        case DailyQuestStatus::Locked:    return "Locked";
        case DailyQuestStatus::Available: return "Available";
        case DailyQuestStatus::Active:    return "Active";
        case DailyQuestStatus::Completed: return "Completed";
        case DailyQuestStatus::Expired:   return "Expired";
    }
    return "Unknown";
}

class DailyQuestEntry {
public:
    explicit DailyQuestEntry(uint32_t id, const std::string& name, DailyQuestDifficulty difficulty)
        : m_id(id), m_name(name), m_difficulty(difficulty) {}

    void setStatus(DailyQuestStatus v) { m_status    = v; }
    void setXpReward(uint32_t v)       { m_xpReward  = v; }
    void setIsPinned(bool v)           { m_isPinned  = v; }
    void setIsEnabled(bool v)          { m_isEnabled = v; }

    [[nodiscard]] uint32_t              id()         const { return m_id;         }
    [[nodiscard]] const std::string&    name()       const { return m_name;       }
    [[nodiscard]] DailyQuestDifficulty  difficulty() const { return m_difficulty; }
    [[nodiscard]] DailyQuestStatus      status()     const { return m_status;     }
    [[nodiscard]] uint32_t              xpReward()   const { return m_xpReward;   }
    [[nodiscard]] bool                  isPinned()   const { return m_isPinned;   }
    [[nodiscard]] bool                  isEnabled()  const { return m_isEnabled;  }

private:
    uint32_t             m_id;
    std::string          m_name;
    DailyQuestDifficulty m_difficulty;
    DailyQuestStatus m_status    = DailyQuestStatus::Available;
    uint32_t         m_xpReward  = 100u;
    bool             m_isPinned  = false;
    bool             m_isEnabled = true;
};

class DailyQuestEditor {
public:
    void setIsShowExpired(bool v)        { m_isShowExpired       = v; }
    void setIsGroupByDifficulty(bool v)  { m_isGroupByDifficulty = v; }
    void setRefreshHours(uint32_t v)     { m_refreshHours        = v; }

    bool addQuest(const DailyQuestEntry& q) {
        for (auto& x : m_quests) if (x.id() == q.id()) return false;
        m_quests.push_back(q); return true;
    }
    bool removeQuest(uint32_t id) {
        auto it = std::find_if(m_quests.begin(), m_quests.end(),
            [&](const DailyQuestEntry& q){ return q.id() == id; });
        if (it == m_quests.end()) return false;
        m_quests.erase(it); return true;
    }
    [[nodiscard]] DailyQuestEntry* findQuest(uint32_t id) {
        for (auto& q : m_quests) if (q.id() == id) return &q;
        return nullptr;
    }

    [[nodiscard]] bool     isShowExpired()        const { return m_isShowExpired;       }
    [[nodiscard]] bool     isGroupByDifficulty()  const { return m_isGroupByDifficulty; }
    [[nodiscard]] uint32_t refreshHours()         const { return m_refreshHours;        }
    [[nodiscard]] size_t   questCount()           const { return m_quests.size();       }

    [[nodiscard]] size_t countByDifficulty(DailyQuestDifficulty d) const {
        size_t n = 0; for (auto& q : m_quests) if (q.difficulty() == d) ++n; return n;
    }
    [[nodiscard]] size_t countByStatus(DailyQuestStatus s) const {
        size_t n = 0; for (auto& q : m_quests) if (q.status() == s) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& q : m_quests) if (q.isEnabled()) ++n; return n;
    }

private:
    std::vector<DailyQuestEntry> m_quests;
    bool     m_isShowExpired       = false;
    bool     m_isGroupByDifficulty = true;
    uint32_t m_refreshHours        = 24u;
};

} // namespace NF
