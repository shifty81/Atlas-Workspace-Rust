#pragma once
// NF::Editor — cloud storage bucket management editor
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

enum class StorageTier : uint8_t { Hot, Warm, Cold, Archive, Custom };
inline const char* storageTierName(StorageTier v) {
    switch (v) {
        case StorageTier::Hot:     return "Hot";
        case StorageTier::Warm:    return "Warm";
        case StorageTier::Cold:    return "Cold";
        case StorageTier::Archive: return "Archive";
        case StorageTier::Custom:  return "Custom";
    }
    return "Unknown";
}

enum class StorageAccess : uint8_t { Public, Private, Authenticated, Restricted };
inline const char* storageAccessName(StorageAccess v) {
    switch (v) {
        case StorageAccess::Public:        return "Public";
        case StorageAccess::Private:       return "Private";
        case StorageAccess::Authenticated: return "Authenticated";
        case StorageAccess::Restricted:    return "Restricted";
    }
    return "Unknown";
}

class StorageBucket {
public:
    explicit StorageBucket(uint32_t id, const std::string& name, StorageTier tier, StorageAccess access)
        : m_id(id), m_name(name), m_tier(tier), m_access(access) {}

    void setCapacityMB(uint32_t v)  { m_capacityMB  = v; }
    void setIsVersioned(bool v)     { m_isVersioned  = v; }
    void setIsEnabled(bool v)       { m_isEnabled    = v; }

    [[nodiscard]] uint32_t           id()          const { return m_id;          }
    [[nodiscard]] const std::string& name()        const { return m_name;        }
    [[nodiscard]] StorageTier        tier()        const { return m_tier;        }
    [[nodiscard]] StorageAccess      access()      const { return m_access;      }
    [[nodiscard]] uint32_t           capacityMB()  const { return m_capacityMB;  }
    [[nodiscard]] bool               isVersioned() const { return m_isVersioned; }
    [[nodiscard]] bool               isEnabled()   const { return m_isEnabled;   }

private:
    uint32_t      m_id;
    std::string   m_name;
    StorageTier   m_tier;
    StorageAccess m_access;
    uint32_t      m_capacityMB  = 1024u;
    bool          m_isVersioned = false;
    bool          m_isEnabled   = true;
};

class CloudStorageEditor {
public:
    void setIsShowDisabled(bool v)      { m_isShowDisabled    = v; }
    void setIsGroupByTier(bool v)       { m_isGroupByTier     = v; }
    void setDefaultCapacityMB(uint32_t v) { m_defaultCapacityMB = v; }

    bool addBucket(const StorageBucket& b) {
        for (auto& x : m_buckets) if (x.id() == b.id()) return false;
        m_buckets.push_back(b); return true;
    }
    bool removeBucket(uint32_t id) {
        auto it = std::find_if(m_buckets.begin(), m_buckets.end(),
            [&](const StorageBucket& b){ return b.id() == id; });
        if (it == m_buckets.end()) return false;
        m_buckets.erase(it); return true;
    }
    [[nodiscard]] StorageBucket* findBucket(uint32_t id) {
        for (auto& b : m_buckets) if (b.id() == id) return &b;
        return nullptr;
    }

    [[nodiscard]] bool     isShowDisabled()    const { return m_isShowDisabled;    }
    [[nodiscard]] bool     isGroupByTier()     const { return m_isGroupByTier;     }
    [[nodiscard]] uint32_t defaultCapacityMB() const { return m_defaultCapacityMB; }
    [[nodiscard]] size_t   bucketCount()       const { return m_buckets.size();    }

    [[nodiscard]] size_t countByTier(StorageTier t) const {
        size_t n = 0; for (auto& b : m_buckets) if (b.tier() == t) ++n; return n;
    }
    [[nodiscard]] size_t countByAccess(StorageAccess a) const {
        size_t n = 0; for (auto& b : m_buckets) if (b.access() == a) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& b : m_buckets) if (b.isEnabled()) ++n; return n;
    }

private:
    std::vector<StorageBucket> m_buckets;
    bool     m_isShowDisabled    = false;
    bool     m_isGroupByTier     = false;
    uint32_t m_defaultCapacityMB = 512u;
};

} // namespace NF
