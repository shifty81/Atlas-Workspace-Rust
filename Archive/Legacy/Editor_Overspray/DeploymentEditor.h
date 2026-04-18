#pragma once
// NF::Editor — deployment editor
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

enum class DeploymentEnv : uint8_t {
    Dev, Staging, QA, Production, Canary
};

inline const char* deploymentEnvName(DeploymentEnv e) {
    switch (e) {
        case DeploymentEnv::Dev:        return "Dev";
        case DeploymentEnv::Staging:    return "Staging";
        case DeploymentEnv::QA:         return "QA";
        case DeploymentEnv::Production: return "Production";
        case DeploymentEnv::Canary:     return "Canary";
    }
    return "Unknown";
}

enum class DeploymentStatus : uint8_t {
    NotDeployed, Queued, Deploying, Live, Rollback, Failed
};

inline const char* deploymentStatusName(DeploymentStatus s) {
    switch (s) {
        case DeploymentStatus::NotDeployed: return "NotDeployed";
        case DeploymentStatus::Queued:      return "Queued";
        case DeploymentStatus::Deploying:   return "Deploying";
        case DeploymentStatus::Live:        return "Live";
        case DeploymentStatus::Rollback:    return "Rollback";
        case DeploymentStatus::Failed:      return "Failed";
    }
    return "Unknown";
}

class DeployRecord {
public:
    explicit DeployRecord(uint32_t id, const std::string& name,
                          DeploymentEnv env, DeploymentStatus status)
        : m_id(id), m_name(name), m_env(env), m_status(status) {}

    void setStatus(DeploymentStatus v)          { m_status         = v; }
    void setBuildVersion(const std::string& v)  { m_buildVersion   = v; }
    void setIsAutoRollback(bool v)              { m_isAutoRollback = v; }

    [[nodiscard]] uint32_t           id()             const { return m_id;             }
    [[nodiscard]] const std::string& name()           const { return m_name;           }
    [[nodiscard]] DeploymentEnv      env()            const { return m_env;            }
    [[nodiscard]] DeploymentStatus   status()         const { return m_status;         }
    [[nodiscard]] const std::string& buildVersion()   const { return m_buildVersion;   }
    [[nodiscard]] bool               isAutoRollback() const { return m_isAutoRollback; }

private:
    uint32_t         m_id;
    std::string      m_name;
    DeploymentEnv    m_env;
    DeploymentStatus m_status;
    std::string      m_buildVersion   = "0.0.0";
    bool             m_isAutoRollback = false;
};

class DeploymentEditor {
public:
    void setIsShowFailed(bool v)   { m_isShowFailed  = v; }
    void setIsGroupByEnv(bool v)   { m_isGroupByEnv  = v; }
    void setRetryLimit(uint32_t v) { m_retryLimit    = v; }

    bool addRecord(const DeployRecord& r) {
        for (auto& x : m_records) if (x.id() == r.id()) return false;
        m_records.push_back(r); return true;
    }
    bool removeRecord(uint32_t id) {
        auto it = std::find_if(m_records.begin(), m_records.end(),
            [&](const DeployRecord& r){ return r.id() == id; });
        if (it == m_records.end()) return false;
        m_records.erase(it); return true;
    }
    [[nodiscard]] DeployRecord* findRecord(uint32_t id) {
        for (auto& r : m_records) if (r.id() == id) return &r;
        return nullptr;
    }

    [[nodiscard]] bool     isShowFailed() const { return m_isShowFailed; }
    [[nodiscard]] bool     isGroupByEnv() const { return m_isGroupByEnv; }
    [[nodiscard]] uint32_t retryLimit()   const { return m_retryLimit;   }
    [[nodiscard]] size_t   recordCount()  const { return m_records.size(); }

    [[nodiscard]] size_t countByEnv(DeploymentEnv e) const {
        size_t n = 0; for (auto& r : m_records) if (r.env() == e) ++n; return n;
    }
    [[nodiscard]] size_t countByStatus(DeploymentStatus s) const {
        size_t n = 0; for (auto& r : m_records) if (r.status() == s) ++n; return n;
    }
    [[nodiscard]] size_t countAutoRollback() const {
        size_t n = 0; for (auto& r : m_records) if (r.isAutoRollback()) ++n; return n;
    }

private:
    std::vector<DeployRecord> m_records;
    bool     m_isShowFailed = false;
    bool     m_isGroupByEnv = false;
    uint32_t m_retryLimit   = 3u;
};

} // namespace NF
