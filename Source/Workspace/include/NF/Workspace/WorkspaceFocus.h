#pragma once
// NF::Workspace — Phase 21: Workspace Focus and Context Tracking
//
// Layered workspace focus management with history and observers:
//   FocusLayer   — focus priority layer enum
//   FocusTarget  — named focus target with layer classification
//   FocusRecord  — timestamped gain/lose focus event
//   FocusStack   — layered focus management with chronological history
//   FocusManager — workspace-scoped focus lifecycle with observers

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// FocusLayer — focus priority layer
// ═════════════════════════════════════════════════════════════════

enum class FocusLayer : uint8_t {
    Background = 0,
    Base       = 1,
    Overlay    = 2,
    Modal      = 3,
    Popup      = 4,
};

inline const char* focusLayerName(FocusLayer l) {
    switch (l) {
        case FocusLayer::Background: return "Background";
        case FocusLayer::Base:       return "Base";
        case FocusLayer::Overlay:    return "Overlay";
        case FocusLayer::Modal:      return "Modal";
        case FocusLayer::Popup:      return "Popup";
        default:                     return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// FocusTarget — named focus target with layer classification
// ═════════════════════════════════════════════════════════════════

struct FocusTarget {
    std::string id;
    std::string displayName;
    std::string panelId;
    std::string toolId;
    FocusLayer  layer = FocusLayer::Base;

    bool isValid() const { return !id.empty(); }

    bool operator==(const FocusTarget& o) const { return id == o.id; }
    bool operator!=(const FocusTarget& o) const { return id != o.id; }
};

// ═════════════════════════════════════════════════════════════════
// FocusRecord — timestamped gain/lose focus event
// ═════════════════════════════════════════════════════════════════

struct FocusRecord {
    FocusTarget target;
    uint64_t    timestamp = 0;
    bool        gained    = true; // true = focus gained, false = focus lost

    bool isValid() const { return target.isValid(); }
};

// ═════════════════════════════════════════════════════════════════
// FocusStack — layered focus management with chronological history
// ═════════════════════════════════════════════════════════════════

class FocusStack {
public:
    static constexpr int MAX_DEPTH   = 64;
    static constexpr int MAX_HISTORY = 256;

    bool push(const FocusTarget& target) {
        if (!target.isValid()) return false;
        if ((int)m_stack.size() >= MAX_DEPTH) return false;
        // Record lose event for current top
        if (!m_stack.empty())
            addHistory(m_stack.back(), false);
        m_stack.push_back(target);
        addHistory(target, true);
        return true;
    }

    bool pop() {
        if (m_stack.empty()) return false;
        addHistory(m_stack.back(), false);
        m_stack.pop_back();
        if (!m_stack.empty())
            addHistory(m_stack.back(), true);
        return true;
    }

    const FocusTarget* current() const {
        return m_stack.empty() ? nullptr : &m_stack.back();
    }

    int depth() const { return (int)m_stack.size(); }

    bool hasTarget(const std::string& id) const {
        return std::any_of(m_stack.begin(), m_stack.end(),
            [&](const FocusTarget& t) { return t.id == id; });
    }

    void clear() {
        m_stack.clear();
    }

    const std::vector<FocusRecord>& history() const { return m_history; }

    void clearHistory() { m_history.clear(); }

private:
    void addHistory(const FocusTarget& target, bool gained) {
        if ((int)m_history.size() >= MAX_HISTORY)
            m_history.erase(m_history.begin());
        m_history.push_back({target, m_clock++, gained});
    }

    std::vector<FocusTarget> m_stack;
    std::vector<FocusRecord> m_history;
    uint64_t                 m_clock = 0;
};

// ═════════════════════════════════════════════════════════════════
// FocusManager — workspace-scoped focus lifecycle with observers
// ═════════════════════════════════════════════════════════════════

class FocusManager {
public:
    using Observer = std::function<void(const FocusTarget&, bool gained)>;

    static constexpr int MAX_TARGETS   = 256;
    static constexpr int MAX_OBSERVERS = 16;

    bool registerTarget(const FocusTarget& target) {
        if (!target.isValid()) return false;
        if (isRegistered(target.id)) return false;
        if ((int)m_targets.size() >= MAX_TARGETS) return false;
        m_targets.push_back(target);
        return true;
    }

    bool unregisterTarget(const std::string& id) {
        auto it = std::find_if(m_targets.begin(), m_targets.end(),
            [&](const FocusTarget& t) { return t.id == id; });
        if (it == m_targets.end()) return false;
        m_targets.erase(it);
        return true;
    }

    bool isRegistered(const std::string& id) const {
        return findTarget(id) != nullptr;
    }

    const FocusTarget* findTarget(const std::string& id) const {
        for (auto& t : m_targets)
            if (t.id == id) return &t;
        return nullptr;
    }

    bool requestFocus(const std::string& id) {
        const FocusTarget* t = findTarget(id);
        if (!t) return false;
        bool ok = m_stack.push(*t);
        if (ok) notifyObservers(*t, true);
        return ok;
    }

    bool releaseFocus(const std::string& id) {
        auto* cur = m_stack.current();
        if (!cur || cur->id != id) return false;
        FocusTarget lost = *cur;
        bool ok = m_stack.pop();
        if (ok) notifyObservers(lost, false);
        return ok;
    }

    const FocusTarget* currentFocus() const { return m_stack.current(); }

    bool canFocus(const std::string& id) const {
        if (!isRegistered(id)) return false;
        auto* cur = m_stack.current();
        return !(cur && cur->id == id);
    }

    std::vector<std::string> allTargets() const {
        std::vector<std::string> ids;
        ids.reserve(m_targets.size());
        for (auto& t : m_targets) ids.push_back(t.id);
        return ids;
    }

    const FocusStack& stack() const { return m_stack; }

    void clear() {
        m_targets.clear();
        m_stack.clear();
    }

    uint32_t addObserver(Observer cb) {
        if (!cb || (int)m_observers.size() >= MAX_OBSERVERS) return 0;
        uint32_t id = ++m_nextObserverId;
        m_observers.push_back({id, std::move(cb)});
        return id;
    }

    void removeObserver(uint32_t id) {
        m_observers.erase(std::remove_if(m_observers.begin(), m_observers.end(),
            [id](const ObserverEntry& e) { return e.id == id; }), m_observers.end());
    }

    void clearObservers() { m_observers.clear(); }

private:
    struct ObserverEntry { uint32_t id; Observer cb; };

    void notifyObservers(const FocusTarget& target, bool gained) {
        for (auto& e : m_observers) e.cb(target, gained);
    }

    std::vector<FocusTarget>  m_targets;
    FocusStack                m_stack;
    uint32_t                  m_nextObserverId = 0;
    std::vector<ObserverEntry> m_observers;
};

} // namespace NF
