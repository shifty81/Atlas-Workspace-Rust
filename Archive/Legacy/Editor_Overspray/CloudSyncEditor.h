#pragma once
// NF::Editor — cloud sync editor
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

enum class CloudSyncState : uint8_t {
    Idle, Uploading, Downloading, Synced, Conflict, Error, Offline
};

inline const char* cloudSyncStateName(CloudSyncState s) {
    switch (s) {
        case CloudSyncState::Idle:        return "Idle";
        case CloudSyncState::Uploading:   return "Uploading";
        case CloudSyncState::Downloading: return "Downloading";
        case CloudSyncState::Synced:      return "Synced";
        case CloudSyncState::Conflict:    return "Conflict";
        case CloudSyncState::Error:       return "Error";
        case CloudSyncState::Offline:     return "Offline";
    }
    return "Unknown";
}

enum class CloudSyncConflictPolicy : uint8_t {
    KeepLocal, KeepRemote, KeepNewest, KeepOldest, Manual
};

inline const char* cloudSyncConflictPolicyName(CloudSyncConflictPolicy p) {
    switch (p) {
        case CloudSyncConflictPolicy::KeepLocal:   return "KeepLocal";
        case CloudSyncConflictPolicy::KeepRemote:  return "KeepRemote";
        case CloudSyncConflictPolicy::KeepNewest:  return "KeepNewest";
        case CloudSyncConflictPolicy::KeepOldest:  return "KeepOldest";
        case CloudSyncConflictPolicy::Manual:      return "Manual";
    }
    return "Unknown";
}

enum class CloudSyncDataType : uint8_t {
    SaveGame, Settings, Achievements, Statistics, Profile, Custom
};

inline const char* cloudSyncDataTypeName(CloudSyncDataType t) {
    switch (t) {
        case CloudSyncDataType::SaveGame:     return "SaveGame";
        case CloudSyncDataType::Settings:     return "Settings";
        case CloudSyncDataType::Achievements: return "Achievements";
        case CloudSyncDataType::Statistics:   return "Statistics";
        case CloudSyncDataType::Profile:      return "Profile";
        case CloudSyncDataType::Custom:       return "Custom";
    }
    return "Unknown";
}

class CloudSyncSlot {
public:
    explicit CloudSyncSlot(uint32_t id, const std::string& name, CloudSyncDataType dataType)
        : m_id(id), m_name(name), m_dataType(dataType) {}

    void setState(CloudSyncState v)               { m_state          = v; }
    void setConflictPolicy(CloudSyncConflictPolicy v) { m_conflictPolicy = v; }
    void setIsEnabled(bool v)                     { m_isEnabled      = v; }
    void setSizeBytes(uint64_t v)                 { m_sizeBytes      = v; }

    [[nodiscard]] uint32_t                 id()             const { return m_id;             }
    [[nodiscard]] const std::string&       name()           const { return m_name;           }
    [[nodiscard]] CloudSyncDataType        dataType()       const { return m_dataType;       }
    [[nodiscard]] CloudSyncState           state()          const { return m_state;          }
    [[nodiscard]] CloudSyncConflictPolicy  conflictPolicy() const { return m_conflictPolicy; }
    [[nodiscard]] bool                     isEnabled()      const { return m_isEnabled;      }
    [[nodiscard]] uint64_t                 sizeBytes()      const { return m_sizeBytes;      }

private:
    uint32_t               m_id;
    std::string            m_name;
    CloudSyncDataType      m_dataType;
    CloudSyncState         m_state          = CloudSyncState::Idle;
    CloudSyncConflictPolicy m_conflictPolicy = CloudSyncConflictPolicy::KeepNewest;
    bool                   m_isEnabled      = true;
    uint64_t               m_sizeBytes      = 0u;
};

class CloudSyncEditor {
public:
    void setAutoSyncEnabled(bool v)      { m_autoSyncEnabled  = v; }
    void setSyncIntervalSec(float v)     { m_syncIntervalSec  = v; }
    void setDefaultConflictPolicy(CloudSyncConflictPolicy v) { m_defaultConflictPolicy = v; }

    bool addSlot(const CloudSyncSlot& s) {
        for (auto& x : m_slots) if (x.id() == s.id()) return false;
        m_slots.push_back(s); return true;
    }
    bool removeSlot(uint32_t id) {
        auto it = std::find_if(m_slots.begin(), m_slots.end(),
            [&](const CloudSyncSlot& s){ return s.id() == id; });
        if (it == m_slots.end()) return false;
        m_slots.erase(it); return true;
    }
    [[nodiscard]] CloudSyncSlot* findSlot(uint32_t id) {
        for (auto& s : m_slots) if (s.id() == id) return &s;
        return nullptr;
    }

    [[nodiscard]] bool                   isAutoSyncEnabled()     const { return m_autoSyncEnabled;     }
    [[nodiscard]] float                  syncIntervalSec()       const { return m_syncIntervalSec;     }
    [[nodiscard]] CloudSyncConflictPolicy defaultConflictPolicy() const { return m_defaultConflictPolicy; }
    [[nodiscard]] size_t                 slotCount()             const { return m_slots.size();        }

    [[nodiscard]] size_t countByState(CloudSyncState s) const {
        size_t n = 0; for (auto& sl : m_slots) if (sl.state() == s) ++n; return n;
    }
    [[nodiscard]] size_t countByDataType(CloudSyncDataType t) const {
        size_t n = 0; for (auto& sl : m_slots) if (sl.dataType() == t) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& sl : m_slots) if (sl.isEnabled()) ++n; return n;
    }

private:
    std::vector<CloudSyncSlot>  m_slots;
    bool                        m_autoSyncEnabled      = true;
    float                       m_syncIntervalSec      = 300.0f;
    CloudSyncConflictPolicy     m_defaultConflictPolicy = CloudSyncConflictPolicy::KeepNewest;
};

} // namespace NF
