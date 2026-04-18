#pragma once
// NF::Editor — online services panel
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

enum class OnlineServiceType : uint8_t {
    Authentication, Leaderboard, Achievements, Friends, Matchmaking,
    CloudStorage, PushNotification, InAppPurchase, Analytics
};

inline const char* onlineServiceTypeName(OnlineServiceType t) {
    switch (t) {
        case OnlineServiceType::Authentication:  return "Authentication";
        case OnlineServiceType::Leaderboard:     return "Leaderboard";
        case OnlineServiceType::Achievements:    return "Achievements";
        case OnlineServiceType::Friends:         return "Friends";
        case OnlineServiceType::Matchmaking:     return "Matchmaking";
        case OnlineServiceType::CloudStorage:    return "CloudStorage";
        case OnlineServiceType::PushNotification:return "PushNotification";
        case OnlineServiceType::InAppPurchase:   return "InAppPurchase";
        case OnlineServiceType::Analytics:       return "Analytics";
    }
    return "Unknown";
}

enum class OnlineServiceStatus : uint8_t {
    Unknown, Connecting, Connected, Degraded, Disconnected, Maintenance
};

inline const char* onlineServiceStatusName(OnlineServiceStatus s) {
    switch (s) {
        case OnlineServiceStatus::Unknown:      return "Unknown";
        case OnlineServiceStatus::Connecting:   return "Connecting";
        case OnlineServiceStatus::Connected:    return "Connected";
        case OnlineServiceStatus::Degraded:     return "Degraded";
        case OnlineServiceStatus::Disconnected: return "Disconnected";
        case OnlineServiceStatus::Maintenance:  return "Maintenance";
    }
    return "Unknown";
}

enum class OnlineServiceEnvironment : uint8_t {
    Development, Staging, Production, Sandbox
};

inline const char* onlineServiceEnvironmentName(OnlineServiceEnvironment e) {
    switch (e) {
        case OnlineServiceEnvironment::Development: return "Development";
        case OnlineServiceEnvironment::Staging:     return "Staging";
        case OnlineServiceEnvironment::Production:  return "Production";
        case OnlineServiceEnvironment::Sandbox:     return "Sandbox";
    }
    return "Unknown";
}

class OnlineService {
public:
    explicit OnlineService(uint32_t id, const std::string& name, OnlineServiceType type)
        : m_id(id), m_name(name), m_type(type) {}

    void setStatus(OnlineServiceStatus v)           { m_status      = v; }
    void setEnvironment(OnlineServiceEnvironment v) { m_environment = v; }
    void setIsEnabled(bool v)                       { m_isEnabled   = v; }
    void setLatencyMs(float v)                      { m_latencyMs   = v; }

    [[nodiscard]] uint32_t                  id()          const { return m_id;          }
    [[nodiscard]] const std::string&        name()        const { return m_name;        }
    [[nodiscard]] OnlineServiceType         type()        const { return m_type;        }
    [[nodiscard]] OnlineServiceStatus       status()      const { return m_status;      }
    [[nodiscard]] OnlineServiceEnvironment  environment() const { return m_environment; }
    [[nodiscard]] bool                      isEnabled()   const { return m_isEnabled;   }
    [[nodiscard]] float                     latencyMs()   const { return m_latencyMs;   }

private:
    uint32_t                  m_id;
    std::string               m_name;
    OnlineServiceType         m_type;
    OnlineServiceStatus       m_status      = OnlineServiceStatus::Unknown;
    OnlineServiceEnvironment  m_environment = OnlineServiceEnvironment::Development;
    bool                      m_isEnabled   = true;
    float                     m_latencyMs   = 0.0f;
};

class OnlineServicesPanel {
public:
    void setActiveEnvironment(OnlineServiceEnvironment v) { m_activeEnvironment = v; }
    void setShowOffline(bool v)                           { m_showOffline        = v; }
    void setAutoReconnect(bool v)                         { m_autoReconnect      = v; }

    bool addService(const OnlineService& s) {
        for (auto& x : m_services) if (x.id() == s.id()) return false;
        m_services.push_back(s); return true;
    }
    bool removeService(uint32_t id) {
        auto it = std::find_if(m_services.begin(), m_services.end(),
            [&](const OnlineService& s){ return s.id() == id; });
        if (it == m_services.end()) return false;
        m_services.erase(it); return true;
    }
    [[nodiscard]] OnlineService* findService(uint32_t id) {
        for (auto& s : m_services) if (s.id() == id) return &s;
        return nullptr;
    }

    [[nodiscard]] OnlineServiceEnvironment activeEnvironment() const { return m_activeEnvironment; }
    [[nodiscard]] bool                     isShowOffline()     const { return m_showOffline;        }
    [[nodiscard]] bool                     isAutoReconnect()   const { return m_autoReconnect;      }
    [[nodiscard]] size_t                   serviceCount()      const { return m_services.size();    }

    [[nodiscard]] size_t countByType(OnlineServiceType t) const {
        size_t n = 0; for (auto& s : m_services) if (s.type() == t) ++n; return n;
    }
    [[nodiscard]] size_t countByStatus(OnlineServiceStatus s) const {
        size_t n = 0; for (auto& sv : m_services) if (sv.status() == s) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& s : m_services) if (s.isEnabled()) ++n; return n;
    }

private:
    std::vector<OnlineService>  m_services;
    OnlineServiceEnvironment    m_activeEnvironment = OnlineServiceEnvironment::Development;
    bool                        m_showOffline       = true;
    bool                        m_autoReconnect     = true;
};

} // namespace NF
