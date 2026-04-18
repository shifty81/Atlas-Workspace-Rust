#pragma once
// NF::Editor — AI blackboard editor v1: AI blackboard key-value entry management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Abbv1EntryType  : uint8_t { Bool, Int, Float, String, Vector, Object, Enum };
enum class Abbv1EntryScope : uint8_t { Local, Shared, Global, Persistent };

inline const char* abbv1EntryTypeName(Abbv1EntryType t) {
    switch (t) {
        case Abbv1EntryType::Bool:   return "Bool";
        case Abbv1EntryType::Int:    return "Int";
        case Abbv1EntryType::Float:  return "Float";
        case Abbv1EntryType::String: return "String";
        case Abbv1EntryType::Vector: return "Vector";
        case Abbv1EntryType::Object: return "Object";
        case Abbv1EntryType::Enum:   return "Enum";
    }
    return "Unknown";
}

inline const char* abbv1EntryScopeName(Abbv1EntryScope s) {
    switch (s) {
        case Abbv1EntryScope::Local:      return "Local";
        case Abbv1EntryScope::Shared:     return "Shared";
        case Abbv1EntryScope::Global:     return "Global";
        case Abbv1EntryScope::Persistent: return "Persistent";
    }
    return "Unknown";
}

struct Abbv1Entry {
    uint64_t        id    = 0;
    std::string     key;
    Abbv1EntryType  type  = Abbv1EntryType::Bool;
    Abbv1EntryScope scope = Abbv1EntryScope::Local;

    [[nodiscard]] bool isValid()    const { return id != 0 && !key.empty(); }
    [[nodiscard]] bool isShared()   const { return scope == Abbv1EntryScope::Shared; }
    [[nodiscard]] bool isGlobal()   const { return scope == Abbv1EntryScope::Global; }
};

using Abbv1ChangeCallback = std::function<void(uint64_t)>;

class AIBlackboardEditorV1 {
public:
    static constexpr size_t MAX_ENTRIES = 2048;

    bool addEntry(const Abbv1Entry& entry) {
        if (!entry.isValid()) return false;
        for (const auto& e : m_entries) if (e.id == entry.id) return false;
        if (m_entries.size() >= MAX_ENTRIES) return false;
        m_entries.push_back(entry);
        if (m_onChange) m_onChange(entry.id);
        return true;
    }

    bool removeEntry(uint64_t id) {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->id == id) { m_entries.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Abbv1Entry* findEntry(uint64_t id) {
        for (auto& e : m_entries) if (e.id == id) return &e;
        return nullptr;
    }

    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }

    [[nodiscard]] size_t sharedCount() const {
        size_t c = 0; for (const auto& e : m_entries) if (e.isShared()) ++c; return c;
    }
    [[nodiscard]] size_t globalCount() const {
        size_t c = 0; for (const auto& e : m_entries) if (e.isGlobal()) ++c; return c;
    }
    [[nodiscard]] size_t countByEntryType(Abbv1EntryType type) const {
        size_t c = 0; for (const auto& e : m_entries) if (e.type == type) ++c; return c;
    }
    [[nodiscard]] size_t countByScope(Abbv1EntryScope scope) const {
        size_t c = 0; for (const auto& e : m_entries) if (e.scope == scope) ++c; return c;
    }

    void setOnChange(Abbv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Abbv1Entry> m_entries;
    Abbv1ChangeCallback     m_onChange;
};

} // namespace NF
