#pragma once
// NF::Workspace — SelectionSystem: multi-context selection with named sets and history
//
// Provides:
//   SelectionContextType  — enum for scope of selection (Scene/Asset/UI/Console/Code)
//   SelectionRecord       — lightweight handle (entity id + label + context type)
//   SelectionSet          — named container of SelectionRecords (add/remove/contains/clear/count/items)
//   SelectionHistory      — bounded ring of SelectionSets for back/forward navigation (MAX=32)
//   SelectionSystem       — owns multiple named SelectionSets, active context, and history
//
// All types live in namespace NF.

#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include <cstdint>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>

namespace NF {

// ── SelectionContextType ──────────────────────────────────────────────────────

enum class SelectionContextType : uint8_t {
    None    = 0,
    Scene   = 1,
    Asset   = 2,
    UI      = 3,
    Console = 4,
    Code    = 5
};

inline const char* selectionContextTypeName(SelectionContextType t) {
    switch (t) {
        case SelectionContextType::None:    return "None";
        case SelectionContextType::Scene:   return "Scene";
        case SelectionContextType::Asset:   return "Asset";
        case SelectionContextType::UI:      return "UI";
        case SelectionContextType::Console: return "Console";
        case SelectionContextType::Code:    return "Code";
        default:                            return "Unknown";
    }
}

// ── SelectionRecord ───────────────────────────────────────────────────────────

struct SelectionRecord {
    EntityID              id      = INVALID_ENTITY;
    std::string           label;
    SelectionContextType  context = SelectionContextType::None;

    [[nodiscard]] bool isValid()  const { return id != INVALID_ENTITY; }
    [[nodiscard]] bool operator==(const SelectionRecord& o) const { return id == o.id; }
    [[nodiscard]] bool operator!=(const SelectionRecord& o) const { return id != o.id; }
};

// ── SelectionSet ──────────────────────────────────────────────────────────────

class SelectionSet {
public:
    explicit SelectionSet(const std::string& name = "") : m_name(name) {}

    [[nodiscard]] const std::string& name() const { return m_name; }

    bool add(SelectionRecord r) {
        if (!r.isValid()) return false;
        for (auto& existing : m_records)
            if (existing.id == r.id) return false;
        m_records.push_back(std::move(r));
        ++m_version;
        return true;
    }

    bool remove(EntityID id) {
        for (auto it = m_records.begin(); it != m_records.end(); ++it) {
            if (it->id == id) {
                m_records.erase(it);
                ++m_version;
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] bool contains(EntityID id) const {
        for (auto& r : m_records) if (r.id == id) return true;
        return false;
    }

    [[nodiscard]] const SelectionRecord* find(EntityID id) const {
        for (auto& r : m_records) if (r.id == id) return &r;
        return nullptr;
    }

    void clear() {
        m_records.clear();
        ++m_version;
    }

    [[nodiscard]] size_t count()              const { return m_records.size(); }
    [[nodiscard]] bool   isEmpty()            const { return m_records.empty(); }
    [[nodiscard]] uint32_t version()          const { return m_version; }
    [[nodiscard]] const std::vector<SelectionRecord>& items() const { return m_records; }

    [[nodiscard]] size_t countByContext(SelectionContextType ctx) const {
        size_t n = 0;
        for (auto& r : m_records) if (r.context == ctx) ++n;
        return n;
    }

private:
    std::string                   m_name;
    std::vector<SelectionRecord>  m_records;
    uint32_t                      m_version = 0;
};

// ── SelectionHistory ──────────────────────────────────────────────────────────

class SelectionHistory {
public:
    static constexpr size_t MAX_HISTORY = 32;

    void push(SelectionSet snapshot) {
        // Truncate forward history on new push
        if (m_cursor < static_cast<int>(m_history.size()) - 1) {
            m_history.resize(static_cast<size_t>(m_cursor + 1));
        }
        if (m_history.size() >= MAX_HISTORY) {
            m_history.pop_front();
        }
        m_history.push_back(std::move(snapshot));
        m_cursor = static_cast<int>(m_history.size()) - 1;
    }

    bool back() {
        if (!canBack()) return false;
        --m_cursor;
        return true;
    }

    bool forward() {
        if (!canForward()) return false;
        ++m_cursor;
        return true;
    }

    [[nodiscard]] bool canBack()    const { return m_cursor > 0; }
    [[nodiscard]] bool canForward() const { return m_cursor < static_cast<int>(m_history.size()) - 1; }
    [[nodiscard]] bool hasHistory() const { return !m_history.empty(); }
    [[nodiscard]] size_t depth()    const { return m_history.size(); }

    [[nodiscard]] const SelectionSet* current() const {
        if (m_history.empty() || m_cursor < 0) return nullptr;
        return &m_history[static_cast<size_t>(m_cursor)];
    }

    void clear() {
        m_history.clear();
        m_cursor = -1;
    }

private:
    std::deque<SelectionSet>  m_history;
    int                        m_cursor = -1;
};

// ── SelectionSystem ───────────────────────────────────────────────────────────

class SelectionSystem {
public:
    static constexpr size_t MAX_SETS = 16;

    // ── Named selection sets ──

    bool createSet(const std::string& name) {
        if (name.empty()) return false;
        for (auto& s : m_sets) if (s.name() == name) return false;
        if (m_sets.size() >= MAX_SETS) return false;
        m_sets.emplace_back(name);
        return true;
    }

    bool removeSet(const std::string& name) {
        for (auto it = m_sets.begin(); it != m_sets.end(); ++it) {
            if (it->name() == name) {
                m_sets.erase(it);
                if (m_activeSet == name) m_activeSet.clear();
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] SelectionSet* findSet(const std::string& name) {
        for (auto& s : m_sets) if (s.name() == name) return &s;
        return nullptr;
    }

    [[nodiscard]] const SelectionSet* findSet(const std::string& name) const {
        for (auto& s : m_sets) if (s.name() == name) return &s;
        return nullptr;
    }

    [[nodiscard]] size_t setCount() const { return m_sets.size(); }

    // ── Active context ──

    void setActiveContext(SelectionContextType ctx) { m_activeContext = ctx; }
    [[nodiscard]] SelectionContextType activeContext() const { return m_activeContext; }

    bool setActiveSet(const std::string& name) {
        if (!findSet(name)) return false;
        m_activeSet = name;
        return true;
    }

    [[nodiscard]] const std::string& activeSetName() const { return m_activeSet; }

    [[nodiscard]] SelectionSet* activeSet() {
        if (m_activeSet.empty()) return nullptr;
        return findSet(m_activeSet);
    }

    // ── Convenience: operate on the active set ──

    bool select(SelectionRecord r) {
        SelectionSet* s = activeSet();
        if (!s) return false;
        r.context = m_activeContext;
        bool ok = s->add(r);
        if (ok) m_history.push(*s);
        return ok;
    }

    bool deselect(EntityID id) {
        SelectionSet* s = activeSet();
        if (!s) return false;
        bool ok = s->remove(id);
        if (ok) m_history.push(*s);
        return ok;
    }

    void clearActive() {
        SelectionSet* s = activeSet();
        if (!s) return;
        s->clear();
        m_history.push(*s);
    }

    [[nodiscard]] bool isSelected(EntityID id) const {
        const SelectionSet* s = (m_activeSet.empty()) ? nullptr : findSet(m_activeSet);
        if (!s) return false;
        return s->contains(id);
    }

    [[nodiscard]] size_t activeCount() const {
        const SelectionSet* s = (m_activeSet.empty()) ? nullptr : findSet(m_activeSet);
        return s ? s->count() : 0;
    }

    // ── History navigation ──

    [[nodiscard]] SelectionHistory& history() { return m_history; }
    [[nodiscard]] const SelectionHistory& history() const { return m_history; }

    void clearAll() {
        for (auto& s : m_sets) s.clear();
        m_history.clear();
    }

private:
    std::vector<SelectionSet>  m_sets;
    std::string                m_activeSet;
    SelectionContextType       m_activeContext = SelectionContextType::None;
    SelectionHistory           m_history;
};

} // namespace NF
