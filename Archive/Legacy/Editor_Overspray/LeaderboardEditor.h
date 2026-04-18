#pragma once
// NF::Editor — leaderboard editor
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

enum class LeaderboardScope : uint8_t {
    Global, Regional, Friends, Guild, Season, Weekly, Daily, Custom
};

inline const char* leaderboardScopeName(LeaderboardScope s) {
    switch (s) {
        case LeaderboardScope::Global:  return "Global";
        case LeaderboardScope::Regional: return "Regional";
        case LeaderboardScope::Friends: return "Friends";
        case LeaderboardScope::Guild:   return "Guild";
        case LeaderboardScope::Season:  return "Season";
        case LeaderboardScope::Weekly:  return "Weekly";
        case LeaderboardScope::Daily:   return "Daily";
        case LeaderboardScope::Custom:  return "Custom";
    }
    return "Unknown";
}

enum class LeaderboardSortOrder : uint8_t {
    Descending, Ascending
};

inline const char* leaderboardSortOrderName(LeaderboardSortOrder o) {
    switch (o) {
        case LeaderboardSortOrder::Descending: return "Descending";
        case LeaderboardSortOrder::Ascending:  return "Ascending";
    }
    return "Unknown";
}

enum class LeaderboardScoreType : uint8_t {
    Integer, Float, Time, Distance, Percentage
};

inline const char* leaderboardScoreTypeName(LeaderboardScoreType t) {
    switch (t) {
        case LeaderboardScoreType::Integer:    return "Integer";
        case LeaderboardScoreType::Float:      return "Float";
        case LeaderboardScoreType::Time:       return "Time";
        case LeaderboardScoreType::Distance:   return "Distance";
        case LeaderboardScoreType::Percentage: return "Percentage";
    }
    return "Unknown";
}

class LeaderboardDef {
public:
    explicit LeaderboardDef(uint32_t id, const std::string& name, LeaderboardScope scope)
        : m_id(id), m_name(name), m_scope(scope) {}

    void setSortOrder(LeaderboardSortOrder v)  { m_sortOrder  = v; }
    void setScoreType(LeaderboardScoreType v)  { m_scoreType  = v; }
    void setMaxEntries(uint32_t v)             { m_maxEntries = v; }
    void setIsPublic(bool v)                   { m_isPublic   = v; }
    void setResetPeriodDays(uint32_t v)        { m_resetPeriodDays = v; }

    [[nodiscard]] uint32_t              id()               const { return m_id;              }
    [[nodiscard]] const std::string&    name()             const { return m_name;            }
    [[nodiscard]] LeaderboardScope      scope()            const { return m_scope;           }
    [[nodiscard]] LeaderboardSortOrder  sortOrder()        const { return m_sortOrder;       }
    [[nodiscard]] LeaderboardScoreType  scoreType()        const { return m_scoreType;       }
    [[nodiscard]] uint32_t              maxEntries()       const { return m_maxEntries;      }
    [[nodiscard]] bool                  isPublic()         const { return m_isPublic;        }
    [[nodiscard]] uint32_t              resetPeriodDays()  const { return m_resetPeriodDays; }

private:
    uint32_t              m_id;
    std::string           m_name;
    LeaderboardScope      m_scope;
    LeaderboardSortOrder  m_sortOrder       = LeaderboardSortOrder::Descending;
    LeaderboardScoreType  m_scoreType       = LeaderboardScoreType::Integer;
    uint32_t              m_maxEntries      = 100u;
    bool                  m_isPublic        = true;
    uint32_t              m_resetPeriodDays = 0u;
};

class LeaderboardEditor {
public:
    void setShowPreview(bool v)     { m_showPreview = v; }
    void setShowRankBand(bool v)    { m_showRankBand = v; }
    void setPageSize(uint32_t v)    { m_pageSize = v; }

    bool addBoard(const LeaderboardDef& b) {
        for (auto& e : m_boards) if (e.id() == b.id()) return false;
        m_boards.push_back(b); return true;
    }
    bool removeBoard(uint32_t id) {
        auto it = std::find_if(m_boards.begin(), m_boards.end(),
            [&](const LeaderboardDef& e){ return e.id() == id; });
        if (it == m_boards.end()) return false;
        m_boards.erase(it); return true;
    }
    [[nodiscard]] LeaderboardDef* findBoard(uint32_t id) {
        for (auto& e : m_boards) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] bool     isShowPreview()  const { return m_showPreview;   }
    [[nodiscard]] bool     isShowRankBand() const { return m_showRankBand;  }
    [[nodiscard]] uint32_t pageSize()       const { return m_pageSize;      }
    [[nodiscard]] size_t   boardCount()     const { return m_boards.size(); }

    [[nodiscard]] size_t countByScope(LeaderboardScope s) const {
        size_t c = 0; for (auto& e : m_boards) if (e.scope() == s) ++c; return c;
    }
    [[nodiscard]] size_t countByScoreType(LeaderboardScoreType t) const {
        size_t c = 0; for (auto& e : m_boards) if (e.scoreType() == t) ++c; return c;
    }
    [[nodiscard]] size_t countPublic() const {
        size_t c = 0; for (auto& e : m_boards) if (e.isPublic()) ++c; return c;
    }

private:
    std::vector<LeaderboardDef> m_boards;
    bool     m_showPreview  = true;
    bool     m_showRankBand = true;
    uint32_t m_pageSize     = 25u;
};

} // namespace NF
