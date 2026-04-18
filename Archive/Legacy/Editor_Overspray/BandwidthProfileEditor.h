#pragma once
// NF::Editor — bandwidth profile configuration management
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
#include <string>
#include <vector>
#include <cstdint>

namespace NF {

enum class BwProfileTier : uint8_t { Low, Medium, High, Ultra, Unlimited };
inline const char* bwProfileTierName(BwProfileTier v) {
    switch (v) {
        case BwProfileTier::Low:       return "Low";
        case BwProfileTier::Medium:    return "Medium";
        case BwProfileTier::High:      return "High";
        case BwProfileTier::Ultra:     return "Ultra";
        case BwProfileTier::Unlimited: return "Unlimited";
    }
    return "Unknown";
}

enum class BwProfileDirection : uint8_t { Upload, Download, Bidirectional };
inline const char* bwProfileDirectionName(BwProfileDirection v) {
    switch (v) {
        case BwProfileDirection::Upload:        return "Upload";
        case BwProfileDirection::Download:      return "Download";
        case BwProfileDirection::Bidirectional: return "Bidirectional";
    }
    return "Unknown";
}

class BandwidthProfile {
public:
    explicit BandwidthProfile(uint32_t id, const std::string& name,
                               BwProfileTier tier, BwProfileDirection direction)
        : m_id(id), m_name(name), m_tier(tier), m_direction(direction) {}

    void setLimitKbps(uint32_t v)  { m_limitKbps  = v; }
    void setBurstKbps(uint32_t v)  { m_burstKbps  = v; }
    void setIsEnabled(bool v)      { m_isEnabled   = v; }

    [[nodiscard]] uint32_t              id()        const { return m_id;        }
    [[nodiscard]] const std::string&    name()      const { return m_name;      }
    [[nodiscard]] BwProfileTier         tier()      const { return m_tier;      }
    [[nodiscard]] BwProfileDirection    direction() const { return m_direction; }
    [[nodiscard]] uint32_t              limitKbps() const { return m_limitKbps; }
    [[nodiscard]] uint32_t              burstKbps() const { return m_burstKbps; }
    [[nodiscard]] bool                  isEnabled() const { return m_isEnabled; }

private:
    uint32_t           m_id;
    std::string        m_name;
    BwProfileTier      m_tier;
    BwProfileDirection m_direction;
    uint32_t           m_limitKbps  = 1024u;
    uint32_t           m_burstKbps  = 2048u;
    bool               m_isEnabled  = true;
};

class BandwidthProfileEditor {
public:
    void setIsShowDisabled(bool v)       { m_isShowDisabled    = v; }
    void setIsGroupByTier(bool v)        { m_isGroupByTier     = v; }
    void setDefaultLimitKbps(uint32_t v) { m_defaultLimitKbps  = v; }

    bool addProfile(const BandwidthProfile& p) {
        for (auto& x : m_profiles) if (x.id() == p.id()) return false;
        m_profiles.push_back(p); return true;
    }
    bool removeProfile(uint32_t id) {
        auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
            [&](const BandwidthProfile& p){ return p.id() == id; });
        if (it == m_profiles.end()) return false;
        m_profiles.erase(it); return true;
    }
    [[nodiscard]] BandwidthProfile* findProfile(uint32_t id) {
        for (auto& p : m_profiles) if (p.id() == id) return &p;
        return nullptr;
    }

    [[nodiscard]] bool     isShowDisabled()   const { return m_isShowDisabled;   }
    [[nodiscard]] bool     isGroupByTier()    const { return m_isGroupByTier;    }
    [[nodiscard]] uint32_t defaultLimitKbps() const { return m_defaultLimitKbps; }
    [[nodiscard]] size_t   profileCount()     const { return m_profiles.size();  }

    [[nodiscard]] size_t countByTier(BwProfileTier t) const {
        size_t n = 0; for (auto& x : m_profiles) if (x.tier() == t) ++n; return n;
    }
    [[nodiscard]] size_t countByDirection(BwProfileDirection d) const {
        size_t n = 0; for (auto& x : m_profiles) if (x.direction() == d) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& x : m_profiles) if (x.isEnabled()) ++n; return n;
    }

private:
    std::vector<BandwidthProfile> m_profiles;
    bool     m_isShowDisabled   = false;
    bool     m_isGroupByTier    = true;
    uint32_t m_defaultLimitKbps = 512u;
};

} // namespace NF
