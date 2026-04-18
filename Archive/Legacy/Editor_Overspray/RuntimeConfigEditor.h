#pragma once
// NF::Editor — runtime config editor
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

enum class RtConfigScope : uint8_t {
    Global, Session, User, Project, Platform
};

inline const char* rtConfigScopeName(RtConfigScope s) {
    switch (s) {
        case RtConfigScope::Global:   return "Global";
        case RtConfigScope::Session:  return "Session";
        case RtConfigScope::User:     return "User";
        case RtConfigScope::Project:  return "Project";
        case RtConfigScope::Platform: return "Platform";
    }
    return "Unknown";
}

enum class RtConfigValueType : uint8_t {
    Bool, Int, Float, String, Json
};

inline const char* rtConfigValueTypeName(RtConfigValueType t) {
    switch (t) {
        case RtConfigValueType::Bool:   return "Bool";
        case RtConfigValueType::Int:    return "Int";
        case RtConfigValueType::Float:  return "Float";
        case RtConfigValueType::String: return "String";
        case RtConfigValueType::Json:   return "Json";
    }
    return "Unknown";
}

class RuntimeConfigEntry {
public:
    explicit RuntimeConfigEntry(uint32_t id, const std::string& key,
                                RtConfigScope scope, RtConfigValueType valueType)
        : m_id(id), m_key(key), m_scope(scope), m_valueType(valueType) {}

    void setIsOverridable(bool v) { m_isOverridable = v; }

    [[nodiscard]] uint32_t           id()           const { return m_id;           }
    [[nodiscard]] const std::string& key()          const { return m_key;          }
    [[nodiscard]] RtConfigScope      scope()        const { return m_scope;        }
    [[nodiscard]] RtConfigValueType  valueType()    const { return m_valueType;    }
    [[nodiscard]] bool               isOverridable() const { return m_isOverridable; }

private:
    uint32_t        m_id;
    std::string     m_key;
    RtConfigScope   m_scope;
    RtConfigValueType m_valueType;
    bool            m_isOverridable = false;
};

class RuntimeConfigEditor {
public:
    void setIsShowOverridesOnly(bool v)       { m_isShowOverridesOnly = v; }
    void setIsGroupByScope(bool v)            { m_isGroupByScope      = v; }
    void setSearchQuery(const std::string& v) { m_searchQuery         = v; }

    bool addEntry(const RuntimeConfigEntry& e) {
        for (auto& x : m_entries) if (x.id() == e.id()) return false;
        m_entries.push_back(e); return true;
    }
    bool removeEntry(uint32_t id) {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
            [&](const RuntimeConfigEntry& e){ return e.id() == id; });
        if (it == m_entries.end()) return false;
        m_entries.erase(it); return true;
    }
    [[nodiscard]] RuntimeConfigEntry* findEntry(uint32_t id) {
        for (auto& e : m_entries) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] bool              isShowOverridesOnly() const { return m_isShowOverridesOnly; }
    [[nodiscard]] bool              isGroupByScope()      const { return m_isGroupByScope;      }
    [[nodiscard]] const std::string& searchQuery()        const { return m_searchQuery;         }
    [[nodiscard]] size_t            entryCount()          const { return m_entries.size();       }

    [[nodiscard]] size_t countByScope(RtConfigScope s) const {
        size_t n = 0; for (auto& e : m_entries) if (e.scope() == s) ++n; return n;
    }
    [[nodiscard]] size_t countByType(RtConfigValueType t) const {
        size_t n = 0; for (auto& e : m_entries) if (e.valueType() == t) ++n; return n;
    }
    [[nodiscard]] size_t countOverridable() const {
        size_t n = 0; for (auto& e : m_entries) if (e.isOverridable()) ++n; return n;
    }

private:
    std::vector<RuntimeConfigEntry> m_entries;
    bool        m_isShowOverridesOnly = false;
    bool        m_isGroupByScope      = false;
    std::string m_searchQuery;
};

} // namespace NF
