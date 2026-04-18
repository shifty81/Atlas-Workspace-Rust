#pragma once
// NF::Editor — tournament bracket management editor
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

enum class TournamentFormat : uint8_t { SingleElim, DoubleElim, RoundRobin, Swiss, FFA };
inline const char* tournamentFormatName(TournamentFormat v) {
    switch (v) {
        case TournamentFormat::SingleElim:  return "SingleElim";
        case TournamentFormat::DoubleElim:  return "DoubleElim";
        case TournamentFormat::RoundRobin:  return "RoundRobin";
        case TournamentFormat::Swiss:       return "Swiss";
        case TournamentFormat::FFA:         return "FFA";
    }
    return "Unknown";
}

enum class TournamentStatus : uint8_t { Draft, Registration, Active, Completed, Cancelled };
inline const char* tournamentStatusName(TournamentStatus v) {
    switch (v) {
        case TournamentStatus::Draft:        return "Draft";
        case TournamentStatus::Registration: return "Registration";
        case TournamentStatus::Active:       return "Active";
        case TournamentStatus::Completed:    return "Completed";
        case TournamentStatus::Cancelled:    return "Cancelled";
    }
    return "Unknown";
}

class TournamentEntry {
public:
    explicit TournamentEntry(uint32_t id, const std::string& name, TournamentFormat format)
        : m_id(id), m_name(name), m_format(format) {}

    void setStatus(TournamentStatus v)     { m_status          = v; }
    void setMaxParticipants(uint32_t v)    { m_maxParticipants  = v; }
    void setIsPrized(bool v)               { m_isPrized        = v; }
    void setIsEnabled(bool v)              { m_isEnabled       = v; }

    [[nodiscard]] uint32_t           id()              const { return m_id;              }
    [[nodiscard]] const std::string& name()            const { return m_name;            }
    [[nodiscard]] TournamentFormat   format()          const { return m_format;          }
    [[nodiscard]] TournamentStatus   status()          const { return m_status;          }
    [[nodiscard]] uint32_t           maxParticipants() const { return m_maxParticipants;  }
    [[nodiscard]] bool               isPrized()        const { return m_isPrized;        }
    [[nodiscard]] bool               isEnabled()       const { return m_isEnabled;       }

private:
    uint32_t         m_id;
    std::string      m_name;
    TournamentFormat m_format;
    TournamentStatus m_status          = TournamentStatus::Draft;
    uint32_t         m_maxParticipants  = 16u;
    bool             m_isPrized        = false;
    bool             m_isEnabled       = true;
};

class TournamentEditor {
public:
    void setIsShowCompleted(bool v)          { m_isShowCompleted         = v; }
    void setIsGroupByFormat(bool v)          { m_isGroupByFormat         = v; }
    void setDefaultMaxParticipants(uint32_t v) { m_defaultMaxParticipants = v; }

    bool addTournament(const TournamentEntry& e) {
        for (auto& x : m_tournaments) if (x.id() == e.id()) return false;
        m_tournaments.push_back(e); return true;
    }
    bool removeTournament(uint32_t id) {
        auto it = std::find_if(m_tournaments.begin(), m_tournaments.end(),
            [&](const TournamentEntry& e){ return e.id() == id; });
        if (it == m_tournaments.end()) return false;
        m_tournaments.erase(it); return true;
    }
    [[nodiscard]] TournamentEntry* findTournament(uint32_t id) {
        for (auto& e : m_tournaments) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] bool     isShowCompleted()        const { return m_isShowCompleted;         }
    [[nodiscard]] bool     isGroupByFormat()        const { return m_isGroupByFormat;         }
    [[nodiscard]] uint32_t defaultMaxParticipants() const { return m_defaultMaxParticipants;  }
    [[nodiscard]] size_t   tournamentCount()        const { return m_tournaments.size();      }

    [[nodiscard]] size_t countByFormat(TournamentFormat f) const {
        size_t n = 0; for (auto& e : m_tournaments) if (e.format() == f) ++n; return n;
    }
    [[nodiscard]] size_t countByStatus(TournamentStatus s) const {
        size_t n = 0; for (auto& e : m_tournaments) if (e.status() == s) ++n; return n;
    }
    [[nodiscard]] size_t countPrized() const {
        size_t n = 0; for (auto& e : m_tournaments) if (e.isPrized()) ++n; return n;
    }

private:
    std::vector<TournamentEntry> m_tournaments;
    bool     m_isShowCompleted        = false;
    bool     m_isGroupByFormat        = false;
    uint32_t m_defaultMaxParticipants = 32u;
};

} // namespace NF
