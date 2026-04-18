#pragma once
// NF::Editor — feature flag editor
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

enum class FlagState : uint8_t {
    Disabled, Enabled, RollingOut, Deprecated
};

inline const char* featureFlagStateName(FlagState s) {
    switch (s) {
        case FlagState::Disabled:   return "Disabled";
        case FlagState::Enabled:    return "Enabled";
        case FlagState::RollingOut: return "RollingOut";
        case FlagState::Deprecated: return "Deprecated";
    }
    return "Unknown";
}

enum class FlagTarget : uint8_t {
    All, Internal, Beta, Canary, Custom
};

inline const char* featureFlagTargetName(FlagTarget t) {
    switch (t) {
        case FlagTarget::All:      return "All";
        case FlagTarget::Internal: return "Internal";
        case FlagTarget::Beta:     return "Beta";
        case FlagTarget::Canary:   return "Canary";
        case FlagTarget::Custom:   return "Custom";
    }
    return "Unknown";
}

class FeatureFlagEntry {
public:
    explicit FeatureFlagEntry(uint32_t id, const std::string& name,
                              FlagState state, FlagTarget target)
        : m_id(id), m_name(name), m_state(state), m_target(target) {}

    void setState(FlagState v)        { m_state         = v; }
    void setTarget(FlagTarget v)      { m_target        = v; }
    void setRolloutPercent(float v)   { m_rolloutPercent = v; }
    void setIsLogged(bool v)          { m_isLogged      = v; }

    [[nodiscard]] uint32_t           id()             const { return m_id;             }
    [[nodiscard]] const std::string& name()           const { return m_name;           }
    [[nodiscard]] FlagState          state()          const { return m_state;          }
    [[nodiscard]] FlagTarget         target()         const { return m_target;         }
    [[nodiscard]] float              rolloutPercent() const { return m_rolloutPercent; }
    [[nodiscard]] bool               isLogged()       const { return m_isLogged;       }

private:
    uint32_t    m_id;
    std::string m_name;
    FlagState   m_state;
    FlagTarget  m_target;
    float       m_rolloutPercent = 0.0f;
    bool        m_isLogged       = true;
};

class FeatureFlagEditor {
public:
    void setIsShowDeprecated(bool v)  { m_isShowDeprecated = v; }
    void setIsGroupByTarget(bool v)   { m_isGroupByTarget  = v; }
    void setRolloutThreshold(float v) { m_rolloutThreshold = v; }

    bool addFlag(const FeatureFlagEntry& f) {
        for (auto& x : m_flags) if (x.id() == f.id()) return false;
        m_flags.push_back(f); return true;
    }
    bool removeFlag(uint32_t id) {
        auto it = std::find_if(m_flags.begin(), m_flags.end(),
            [&](const FeatureFlagEntry& f){ return f.id() == id; });
        if (it == m_flags.end()) return false;
        m_flags.erase(it); return true;
    }
    [[nodiscard]] FeatureFlagEntry* findFlag(uint32_t id) {
        for (auto& f : m_flags) if (f.id() == id) return &f;
        return nullptr;
    }

    [[nodiscard]] bool  isShowDeprecated() const { return m_isShowDeprecated; }
    [[nodiscard]] bool  isGroupByTarget()  const { return m_isGroupByTarget;  }
    [[nodiscard]] float rolloutThreshold() const { return m_rolloutThreshold; }
    [[nodiscard]] size_t flagCount()       const { return m_flags.size();     }

    [[nodiscard]] size_t countByState(FlagState s) const {
        size_t n = 0; for (auto& f : m_flags) if (f.state() == s) ++n; return n;
    }
    [[nodiscard]] size_t countByTarget(FlagTarget t) const {
        size_t n = 0; for (auto& f : m_flags) if (f.target() == t) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& f : m_flags) if (f.state() == FlagState::Enabled) ++n; return n;
    }

private:
    std::vector<FeatureFlagEntry> m_flags;
    bool  m_isShowDeprecated = false;
    bool  m_isGroupByTarget  = false;
    float m_rolloutThreshold = 50.0f;
};

} // namespace NF
