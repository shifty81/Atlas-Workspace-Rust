#pragma once
// NF::Editor — asset bundle editor
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

enum class BundleType : uint8_t {
    Core, Optional, DLC, Patch, Streaming, Language
};

inline const char* bundleTypeName(BundleType t) {
    switch (t) {
        case BundleType::Core:      return "Core";
        case BundleType::Optional:  return "Optional";
        case BundleType::DLC:       return "DLC";
        case BundleType::Patch:     return "Patch";
        case BundleType::Streaming: return "Streaming";
        case BundleType::Language:  return "Language";
    }
    return "Unknown";
}

enum class BundleState : uint8_t {
    Pending, Building, Ready, Stale, Failed
};

inline const char* bundleStateName(BundleState s) {
    switch (s) {
        case BundleState::Pending:  return "Pending";
        case BundleState::Building: return "Building";
        case BundleState::Ready:    return "Ready";
        case BundleState::Stale:    return "Stale";
        case BundleState::Failed:   return "Failed";
    }
    return "Unknown";
}

class AssetBundleEntry {
public:
    explicit AssetBundleEntry(uint32_t id, const std::string& name,
                              BundleType bundleType, BundleState state)
        : m_id(id), m_name(name), m_bundleType(bundleType), m_state(state) {}

    void setState(BundleState v)      { m_state        = v; }
    void setSizeKB(uint32_t v)        { m_sizeKB       = v; }
    void setIsCompressed(bool v)      { m_isCompressed = v; }

    [[nodiscard]] uint32_t           id()           const { return m_id;           }
    [[nodiscard]] const std::string& name()         const { return m_name;         }
    [[nodiscard]] BundleType         bundleType()   const { return m_bundleType;   }
    [[nodiscard]] BundleState        state()        const { return m_state;        }
    [[nodiscard]] uint32_t           sizeKB()       const { return m_sizeKB;       }
    [[nodiscard]] bool               isCompressed() const { return m_isCompressed; }

private:
    uint32_t    m_id;
    std::string m_name;
    BundleType  m_bundleType;
    BundleState m_state;
    uint32_t    m_sizeKB       = 0u;
    bool        m_isCompressed = true;
};

class AssetBundleEditor {
public:
    void setIsShowStale(bool v)          { m_isShowStale       = v; }
    void setIsGroupByType(bool v)        { m_isGroupByType     = v; }
    void setCompressionLevel(uint32_t v) { m_compressionLevel  = v; }

    bool addBundle(const AssetBundleEntry& b) {
        for (auto& x : m_bundles) if (x.id() == b.id()) return false;
        m_bundles.push_back(b); return true;
    }
    bool removeBundle(uint32_t id) {
        auto it = std::find_if(m_bundles.begin(), m_bundles.end(),
            [&](const AssetBundleEntry& b){ return b.id() == id; });
        if (it == m_bundles.end()) return false;
        m_bundles.erase(it); return true;
    }
    [[nodiscard]] AssetBundleEntry* findBundle(uint32_t id) {
        for (auto& b : m_bundles) if (b.id() == id) return &b;
        return nullptr;
    }

    [[nodiscard]] bool     isShowStale()       const { return m_isShowStale;      }
    [[nodiscard]] bool     isGroupByType()     const { return m_isGroupByType;    }
    [[nodiscard]] uint32_t compressionLevel()  const { return m_compressionLevel; }
    [[nodiscard]] size_t   bundleCount()       const { return m_bundles.size();   }

    [[nodiscard]] size_t countByType(BundleType t) const {
        size_t n = 0; for (auto& b : m_bundles) if (b.bundleType() == t) ++n; return n;
    }
    [[nodiscard]] size_t countByState(BundleState s) const {
        size_t n = 0; for (auto& b : m_bundles) if (b.state() == s) ++n; return n;
    }
    [[nodiscard]] size_t countCompressed() const {
        size_t n = 0; for (auto& b : m_bundles) if (b.isCompressed()) ++n; return n;
    }

private:
    std::vector<AssetBundleEntry> m_bundles;
    bool     m_isShowStale      = false;
    bool     m_isGroupByType    = false;
    uint32_t m_compressionLevel = 6u;
};

} // namespace NF
