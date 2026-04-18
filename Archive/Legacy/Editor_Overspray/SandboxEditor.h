#pragma once
// NF::Editor — sandbox environment editor
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

enum class SandboxMode : uint8_t { Isolated, Networked, Headless, FullStack, Custom };
inline const char* sandboxModeName(SandboxMode v) {
    switch (v) {
        case SandboxMode::Isolated:   return "Isolated";
        case SandboxMode::Networked:  return "Networked";
        case SandboxMode::Headless:   return "Headless";
        case SandboxMode::FullStack:  return "FullStack";
        case SandboxMode::Custom:     return "Custom";
    }
    return "Unknown";
}

enum class SandboxState : uint8_t { Inactive, Launching, Active, Paused, Crashed, Destroyed };
inline const char* sandboxStateName(SandboxState v) {
    switch (v) {
        case SandboxState::Inactive:   return "Inactive";
        case SandboxState::Launching:  return "Launching";
        case SandboxState::Active:     return "Active";
        case SandboxState::Paused:     return "Paused";
        case SandboxState::Crashed:    return "Crashed";
        case SandboxState::Destroyed:  return "Destroyed";
    }
    return "Unknown";
}

class SandboxEntry {
public:
    explicit SandboxEntry(uint32_t id, const std::string& name, SandboxMode mode)
        : m_id(id), m_name(name), m_mode(mode) {}

    void setState(SandboxState v)    { m_state        = v; }
    void setIsRecordable(bool v)     { m_isRecordable = v; }
    void setTimeoutSec(float v)      { m_timeoutSec   = v; }
    void setIsEnabled(bool v)        { m_isEnabled    = v; }

    [[nodiscard]] uint32_t           id()           const { return m_id;           }
    [[nodiscard]] const std::string& name()         const { return m_name;         }
    [[nodiscard]] SandboxMode        mode()         const { return m_mode;         }
    [[nodiscard]] SandboxState       state()        const { return m_state;        }
    [[nodiscard]] bool               isRecordable() const { return m_isRecordable; }
    [[nodiscard]] float              timeoutSec()   const { return m_timeoutSec;   }
    [[nodiscard]] bool               isEnabled()    const { return m_isEnabled;    }

private:
    uint32_t    m_id;
    std::string m_name;
    SandboxMode m_mode;
    SandboxState m_state        = SandboxState::Inactive;
    bool         m_isRecordable = false;
    float        m_timeoutSec   = 30.0f;
    bool         m_isEnabled    = true;
};

class SandboxEditor {
public:
    void setIsShowInactive(bool v)     { m_isShowInactive = v; }
    void setIsGroupByMode(bool v)      { m_isGroupByMode  = v; }
    void setMaxConcurrent(uint32_t v)  { m_maxConcurrent  = v; }

    bool addEntry(const SandboxEntry& e) {
        for (auto& x : m_entries) if (x.id() == e.id()) return false;
        m_entries.push_back(e); return true;
    }
    bool removeEntry(uint32_t id) {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
            [&](const SandboxEntry& e){ return e.id() == id; });
        if (it == m_entries.end()) return false;
        m_entries.erase(it); return true;
    }
    [[nodiscard]] SandboxEntry* findEntry(uint32_t id) {
        for (auto& e : m_entries) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] bool     isShowInactive() const { return m_isShowInactive; }
    [[nodiscard]] bool     isGroupByMode()  const { return m_isGroupByMode;  }
    [[nodiscard]] uint32_t maxConcurrent()  const { return m_maxConcurrent;  }
    [[nodiscard]] size_t   entryCount()     const { return m_entries.size(); }

    [[nodiscard]] size_t countByMode(SandboxMode m) const {
        size_t n = 0; for (auto& e : m_entries) if (e.mode() == m) ++n; return n;
    }
    [[nodiscard]] size_t countByState(SandboxState s) const {
        size_t n = 0; for (auto& e : m_entries) if (e.state() == s) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& e : m_entries) if (e.isEnabled()) ++n; return n;
    }

private:
    std::vector<SandboxEntry> m_entries;
    bool     m_isShowInactive = true;
    bool     m_isGroupByMode  = false;
    uint32_t m_maxConcurrent  = 4u;
};

} // namespace NF
