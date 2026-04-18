#pragma once
// NF::Editor — Deployment target editor v1: deploy target configuration and push lifecycle authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Dtev1TargetType   : uint8_t { LocalDevice, RemoteDevice, CloudServer, CDN, QAFarm };
enum class Dtev1DeployState  : uint8_t { Idle, Connecting, Uploading, Verifying, Complete, Failed };
enum class Dtev1Environment  : uint8_t { Development, Staging, QA, Production };

inline const char* dtev1TargetTypeName(Dtev1TargetType t) {
    switch (t) {
        case Dtev1TargetType::LocalDevice:  return "LocalDevice";
        case Dtev1TargetType::RemoteDevice: return "RemoteDevice";
        case Dtev1TargetType::CloudServer:  return "CloudServer";
        case Dtev1TargetType::CDN:          return "CDN";
        case Dtev1TargetType::QAFarm:       return "QAFarm";
    }
    return "Unknown";
}

inline const char* dtev1DeployStateName(Dtev1DeployState s) {
    switch (s) {
        case Dtev1DeployState::Idle:        return "Idle";
        case Dtev1DeployState::Connecting:  return "Connecting";
        case Dtev1DeployState::Uploading:   return "Uploading";
        case Dtev1DeployState::Verifying:   return "Verifying";
        case Dtev1DeployState::Complete:    return "Complete";
        case Dtev1DeployState::Failed:      return "Failed";
    }
    return "Unknown";
}

inline const char* dtev1EnvironmentName(Dtev1Environment e) {
    switch (e) {
        case Dtev1Environment::Development: return "Development";
        case Dtev1Environment::Staging:     return "Staging";
        case Dtev1Environment::QA:          return "QA";
        case Dtev1Environment::Production:  return "Production";
    }
    return "Unknown";
}

struct Dtev1DeployTarget {
    uint64_t           id          = 0;
    std::string        name;
    std::string        address;
    Dtev1TargetType    targetType  = Dtev1TargetType::LocalDevice;
    Dtev1DeployState   state       = Dtev1DeployState::Idle;
    Dtev1Environment   environment = Dtev1Environment::Development;
    float              progress    = 0.f;
    bool               isEnabled   = true;
    bool               requiresVPN = false;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isComplete() const { return state == Dtev1DeployState::Complete; }
    [[nodiscard]] bool hasFailed()  const { return state == Dtev1DeployState::Failed; }
    [[nodiscard]] bool isActive()   const {
        return state == Dtev1DeployState::Connecting ||
               state == Dtev1DeployState::Uploading  ||
               state == Dtev1DeployState::Verifying;
    }
};

using Dtev1ChangeCallback = std::function<void(uint64_t)>;

class DeploymentTargetEditorV1 {
public:
    static constexpr size_t MAX_TARGETS = 64;

    bool addTarget(const Dtev1DeployTarget& target) {
        if (!target.isValid()) return false;
        for (const auto& t : m_targets) if (t.id == target.id) return false;
        if (m_targets.size() >= MAX_TARGETS) return false;
        m_targets.push_back(target);
        return true;
    }

    bool removeTarget(uint64_t id) {
        for (auto it = m_targets.begin(); it != m_targets.end(); ++it) {
            if (it->id == id) { m_targets.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Dtev1DeployTarget* findTarget(uint64_t id) {
        for (auto& t : m_targets) if (t.id == id) return &t;
        return nullptr;
    }

    bool setState(uint64_t id, Dtev1DeployState state) {
        auto* t = findTarget(id);
        if (!t) return false;
        t->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setProgress(uint64_t id, float progress) {
        if (progress < 0.f || progress > 1.f) return false;
        auto* t = findTarget(id);
        if (!t) return false;
        t->progress = progress;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setEnabled(uint64_t id, bool enabled) {
        auto* t = findTarget(id);
        if (!t) return false;
        t->isEnabled = enabled;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setRequiresVPN(uint64_t id, bool requiresVPN) {
        auto* t = findTarget(id);
        if (!t) return false;
        t->requiresVPN = requiresVPN;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t targetCount()    const { return m_targets.size(); }
    [[nodiscard]] size_t completeCount()  const {
        size_t c = 0; for (const auto& t : m_targets) if (t.isComplete()) ++c; return c;
    }
    [[nodiscard]] size_t failedCount()    const {
        size_t c = 0; for (const auto& t : m_targets) if (t.hasFailed())  ++c; return c;
    }
    [[nodiscard]] size_t activeCount()    const {
        size_t c = 0; for (const auto& t : m_targets) if (t.isActive())   ++c; return c;
    }
    [[nodiscard]] size_t enabledCount()   const {
        size_t c = 0; for (const auto& t : m_targets) if (t.isEnabled)    ++c; return c;
    }
    [[nodiscard]] size_t countByType(Dtev1TargetType type) const {
        size_t c = 0; for (const auto& t : m_targets) if (t.targetType == type) ++c; return c;
    }
    [[nodiscard]] size_t countByEnvironment(Dtev1Environment env) const {
        size_t c = 0; for (const auto& t : m_targets) if (t.environment == env) ++c; return c;
    }

    void setOnChange(Dtev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Dtev1DeployTarget> m_targets;
    Dtev1ChangeCallback            m_onChange;
};

} // namespace NF
