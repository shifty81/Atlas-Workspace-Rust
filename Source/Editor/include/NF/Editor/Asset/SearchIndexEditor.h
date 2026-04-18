#pragma once
// NF::Editor — search index configuration editor
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

enum class IndexType : uint8_t { FullText, Keyword, Numeric, Spatial, Vector };
inline const char* indexTypeName(IndexType v) {
    switch (v) {
        case IndexType::FullText: return "FullText";
        case IndexType::Keyword:  return "Keyword";
        case IndexType::Numeric:  return "Numeric";
        case IndexType::Spatial:  return "Spatial";
        case IndexType::Vector:   return "Vector";
    }
    return "Unknown";
}

enum class IndexStatus : uint8_t { Idle, Building, Ready, Stale, Error };
inline const char* indexStatusName(IndexStatus v) {
    switch (v) {
        case IndexStatus::Idle:     return "Idle";
        case IndexStatus::Building: return "Building";
        case IndexStatus::Ready:    return "Ready";
        case IndexStatus::Stale:    return "Stale";
        case IndexStatus::Error:    return "Error";
    }
    return "Unknown";
}

class SearchIndex {
public:
    explicit SearchIndex(uint32_t id, const std::string& name, IndexType type)
        : m_id(id), m_name(name), m_type(type) {}

    void setStatus(IndexStatus v)     { m_status        = v; }
    void setEntryCount(uint32_t v)    { m_entryCount     = v; }
    void setIsAutoRebuild(bool v)     { m_isAutoRebuild  = v; }
    void setIsEnabled(bool v)         { m_isEnabled      = v; }

    [[nodiscard]] uint32_t           id()            const { return m_id;            }
    [[nodiscard]] const std::string& name()          const { return m_name;          }
    [[nodiscard]] IndexType          type()          const { return m_type;          }
    [[nodiscard]] IndexStatus        status()        const { return m_status;        }
    [[nodiscard]] uint32_t           entryCount()    const { return m_entryCount;     }
    [[nodiscard]] bool               isAutoRebuild() const { return m_isAutoRebuild;  }
    [[nodiscard]] bool               isEnabled()     const { return m_isEnabled;     }

private:
    uint32_t    m_id;
    std::string m_name;
    IndexType   m_type;
    IndexStatus m_status        = IndexStatus::Idle;
    uint32_t    m_entryCount     = 0u;
    bool        m_isAutoRebuild  = true;
    bool        m_isEnabled      = true;
};

class SearchIndexEditor {
public:
    void setIsShowDisabled(bool v)       { m_isShowDisabled      = v; }
    void setIsGroupByType(bool v)        { m_isGroupByType       = v; }
    void setRebuildIntervalMin(uint32_t v) { m_rebuildIntervalMin = v; }

    bool addIndex(const SearchIndex& i) {
        for (auto& x : m_indices) if (x.id() == i.id()) return false;
        m_indices.push_back(i); return true;
    }
    bool removeIndex(uint32_t id) {
        auto it = std::find_if(m_indices.begin(), m_indices.end(),
            [&](const SearchIndex& i){ return i.id() == id; });
        if (it == m_indices.end()) return false;
        m_indices.erase(it); return true;
    }
    [[nodiscard]] SearchIndex* findIndex(uint32_t id) {
        for (auto& i : m_indices) if (i.id() == id) return &i;
        return nullptr;
    }

    [[nodiscard]] bool     isShowDisabled()      const { return m_isShowDisabled;      }
    [[nodiscard]] bool     isGroupByType()       const { return m_isGroupByType;       }
    [[nodiscard]] uint32_t rebuildIntervalMin()  const { return m_rebuildIntervalMin;  }
    [[nodiscard]] size_t   indexCount()          const { return m_indices.size();      }

    [[nodiscard]] size_t countByType(IndexType t) const {
        size_t n = 0; for (auto& i : m_indices) if (i.type() == t) ++n; return n;
    }
    [[nodiscard]] size_t countByStatus(IndexStatus s) const {
        size_t n = 0; for (auto& i : m_indices) if (i.status() == s) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& i : m_indices) if (i.isEnabled()) ++n; return n;
    }

private:
    std::vector<SearchIndex> m_indices;
    bool     m_isShowDisabled     = false;
    bool     m_isGroupByType      = false;
    uint32_t m_rebuildIntervalMin = 60u;
};

} // namespace NF
