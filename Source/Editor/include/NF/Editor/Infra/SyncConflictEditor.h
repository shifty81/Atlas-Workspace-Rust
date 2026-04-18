#pragma once
// NF::Editor — sync conflict resolution editor
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

enum class ConflictResolution : uint8_t { LocalWins, RemoteWins, Merge, Manual, LastWrite };
inline const char* conflictResolutionName(ConflictResolution v) {
    switch (v) {
        case ConflictResolution::LocalWins:  return "LocalWins";
        case ConflictResolution::RemoteWins: return "RemoteWins";
        case ConflictResolution::Merge:      return "Merge";
        case ConflictResolution::Manual:     return "Manual";
        case ConflictResolution::LastWrite:  return "LastWrite";
    }
    return "Unknown";
}

enum class ConflictSeverity : uint8_t { Minor, Moderate, Major, Critical };
inline const char* conflictSeverityName(ConflictSeverity v) {
    switch (v) {
        case ConflictSeverity::Minor:    return "Minor";
        case ConflictSeverity::Moderate: return "Moderate";
        case ConflictSeverity::Major:    return "Major";
        case ConflictSeverity::Critical: return "Critical";
    }
    return "Unknown";
}

class SyncConflict {
public:
    explicit SyncConflict(uint32_t id, const std::string& name,
                          ConflictResolution resolution, ConflictSeverity severity)
        : m_id(id), m_name(name), m_resolution(resolution), m_severity(severity) {}

    void setIsResolved(bool v)        { m_isResolved      = v; }
    void setConflictTimeMs(uint32_t v){ m_conflictTimeMs  = v; }
    void setIsEnabled(bool v)         { m_isEnabled       = v; }

    [[nodiscard]] uint32_t             id()              const { return m_id;              }
    [[nodiscard]] const std::string&   name()            const { return m_name;            }
    [[nodiscard]] ConflictResolution   resolution()      const { return m_resolution;      }
    [[nodiscard]] ConflictSeverity     severity()        const { return m_severity;        }
    [[nodiscard]] bool                 isResolved()      const { return m_isResolved;      }
    [[nodiscard]] uint32_t             conflictTimeMs()  const { return m_conflictTimeMs;  }
    [[nodiscard]] bool                 isEnabled()       const { return m_isEnabled;       }

private:
    uint32_t           m_id;
    std::string        m_name;
    ConflictResolution m_resolution;
    ConflictSeverity   m_severity;
    bool               m_isResolved     = false;
    uint32_t           m_conflictTimeMs = 0u;
    bool               m_isEnabled      = true;
};

class SyncConflictEditor {
public:
    void setIsShowResolved(bool v)    { m_isShowResolved    = v; }
    void setIsGroupBySeverity(bool v) { m_isGroupBySeverity = v; }
    void setAutoResolvMinor(bool v)   { m_autoResolvMinor   = v; }

    bool addConflict(const SyncConflict& c) {
        for (auto& x : m_conflicts) if (x.id() == c.id()) return false;
        m_conflicts.push_back(c); return true;
    }
    bool removeConflict(uint32_t id) {
        auto it = std::find_if(m_conflicts.begin(), m_conflicts.end(),
            [&](const SyncConflict& c){ return c.id() == id; });
        if (it == m_conflicts.end()) return false;
        m_conflicts.erase(it); return true;
    }
    [[nodiscard]] SyncConflict* findConflict(uint32_t id) {
        for (auto& c : m_conflicts) if (c.id() == id) return &c;
        return nullptr;
    }

    [[nodiscard]] bool   isShowResolved()    const { return m_isShowResolved;    }
    [[nodiscard]] bool   isGroupBySeverity() const { return m_isGroupBySeverity; }
    [[nodiscard]] bool   autoResolvMinor()   const { return m_autoResolvMinor;   }
    [[nodiscard]] size_t conflictCount()     const { return m_conflicts.size();  }

    [[nodiscard]] size_t countByResolution(ConflictResolution r) const {
        size_t n = 0; for (auto& c : m_conflicts) if (c.resolution() == r) ++n; return n;
    }
    [[nodiscard]] size_t countBySeverity(ConflictSeverity s) const {
        size_t n = 0; for (auto& c : m_conflicts) if (c.severity() == s) ++n; return n;
    }
    [[nodiscard]] size_t countResolved() const {
        size_t n = 0; for (auto& c : m_conflicts) if (c.isResolved()) ++n; return n;
    }

private:
    std::vector<SyncConflict> m_conflicts;
    bool m_isShowResolved    = false;
    bool m_isGroupBySeverity = false;
    bool m_autoResolvMinor   = true;
};

} // namespace NF
