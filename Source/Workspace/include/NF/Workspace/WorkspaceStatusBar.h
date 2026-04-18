#pragma once
// NF::Workspace — Phase 25: Workspace Status Bar System
//
// Workspace-level status bar item management:
//   StatusBarSide    — Left / Center / Right placement
//   StatusBarItem    — id + label + tooltip + icon + priority + enabled; isValid()
//   StatusBarSection — ordered collection of items on one side; insert/remove/sorted
//   StatusBarManager — three-section registry with observers on item change

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// StatusBarSide — placement on the status bar
// ═════════════════════════════════════════════════════════════════

enum class StatusBarSide : uint8_t {
    Left,
    Center,
    Right,
};

inline const char* statusBarSideName(StatusBarSide s) {
    switch (s) {
        case StatusBarSide::Left:   return "Left";
        case StatusBarSide::Center: return "Center";
        case StatusBarSide::Right:  return "Right";
        default:                    return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// StatusBarItem — a single status bar widget
// ═════════════════════════════════════════════════════════════════

struct StatusBarItem {
    std::string id;
    std::string label;
    std::string tooltip;
    std::string icon;        // optional icon id
    int         priority = 0;  // lower = further left / top
    bool        enabled  = true;

    bool isValid() const { return !id.empty(); }

    bool operator==(const StatusBarItem& o) const { return id == o.id; }
    bool operator!=(const StatusBarItem& o) const { return id != o.id; }
};

// ═════════════════════════════════════════════════════════════════
// StatusBarSection — ordered, priority-sorted item collection
// ═════════════════════════════════════════════════════════════════

class StatusBarSection {
public:
    static constexpr int MAX_ITEMS = 64;

    bool add(const StatusBarItem& item) {
        if (!item.isValid()) return false;
        if (find(item.id)) return false;
        if ((int)m_items.size() >= MAX_ITEMS) return false;
        m_items.push_back(item);
        sort();
        return true;
    }

    bool remove(const std::string& id) {
        auto it = std::find_if(m_items.begin(), m_items.end(),
            [&](const StatusBarItem& i) { return i.id == id; });
        if (it == m_items.end()) return false;
        m_items.erase(it);
        return true;
    }

    bool update(const StatusBarItem& item) {
        for (auto& i : m_items) {
            if (i.id == item.id) {
                i = item;
                sort();
                return true;
            }
        }
        return false;
    }

    StatusBarItem* find(const std::string& id) {
        for (auto& i : m_items)
            if (i.id == id) return &i;
        return nullptr;
    }

    const StatusBarItem* find(const std::string& id) const {
        for (auto& i : m_items)
            if (i.id == id) return &i;
        return nullptr;
    }

    bool contains(const std::string& id) const { return find(id) != nullptr; }
    int  count()                          const { return (int)m_items.size(); }
    bool empty()                          const { return m_items.empty(); }

    const std::vector<StatusBarItem>& items() const { return m_items; }

    void clear() { m_items.clear(); }

private:
    void sort() {
        std::stable_sort(m_items.begin(), m_items.end(),
            [](const StatusBarItem& a, const StatusBarItem& b) {
                return a.priority < b.priority;
            });
    }

    std::vector<StatusBarItem> m_items;
};

// ═════════════════════════════════════════════════════════════════
// StatusBarManager — three-section registry with observers
// ═════════════════════════════════════════════════════════════════

class StatusBarManager {
public:
    using Observer = std::function<void(const StatusBarItem&, StatusBarSide, bool added)>;

    static constexpr int MAX_OBSERVERS = 16;

    // Item management ──────────────────────────────────────────

    bool addItem(const StatusBarItem& item, StatusBarSide side) {
        bool ok = section(side).add(item);
        if (ok) notify(item, side, true);
        return ok;
    }

    bool removeItem(const std::string& id, StatusBarSide side) {
        const StatusBarItem* item = section(side).find(id);
        if (!item) return false;
        StatusBarItem copy = *item;
        bool ok = section(side).remove(id);
        if (ok) notify(copy, side, false);
        return ok;
    }

    bool updateItem(const StatusBarItem& item, StatusBarSide side) {
        bool ok = section(side).update(item);
        if (ok) notify(item, side, true);
        return ok;
    }

    bool contains(const std::string& id, StatusBarSide side) const {
        return section(side).contains(id);
    }

    const StatusBarItem* findItem(const std::string& id, StatusBarSide side) const {
        return section(side).find(id);
    }

    const StatusBarSection& sectionOf(StatusBarSide side) const { return section(side); }

    // Enable / disable ─────────────────────────────────────────

    bool enableItem(const std::string& id, StatusBarSide side) {
        StatusBarItem* item = section(side).find(id);
        if (!item) return false;
        item->enabled = true;
        notify(*item, side, true);
        return true;
    }

    bool disableItem(const std::string& id, StatusBarSide side) {
        StatusBarItem* item = section(side).find(id);
        if (!item) return false;
        item->enabled = false;
        notify(*item, side, true);
        return true;
    }

    // Bulk ─────────────────────────────────────────────────────

    void clear() {
        m_left.clear();
        m_center.clear();
        m_right.clear();
    }

    // Observers ────────────────────────────────────────────────

    uint32_t addObserver(Observer cb) {
        if (!cb || (int)m_observers.size() >= MAX_OBSERVERS) return 0;
        uint32_t id = ++m_nextObserverId;
        m_observers.push_back({id, std::move(cb)});
        return id;
    }

    void removeObserver(uint32_t id) {
        m_observers.erase(
            std::remove_if(m_observers.begin(), m_observers.end(),
                [id](const ObserverEntry& e) { return e.id == id; }),
            m_observers.end());
    }

    void clearObservers() { m_observers.clear(); }

private:
    struct ObserverEntry { uint32_t id; Observer cb; };

    StatusBarSection& section(StatusBarSide s) {
        if (s == StatusBarSide::Left)   return m_left;
        if (s == StatusBarSide::Center) return m_center;
        return m_right;
    }
    const StatusBarSection& section(StatusBarSide s) const {
        if (s == StatusBarSide::Left)   return m_left;
        if (s == StatusBarSide::Center) return m_center;
        return m_right;
    }

    void notify(const StatusBarItem& item, StatusBarSide side, bool added) {
        for (auto& e : m_observers) e.cb(item, side, added);
    }

    StatusBarSection          m_left, m_center, m_right;
    uint32_t                  m_nextObserverId = 0;
    std::vector<ObserverEntry> m_observers;
};

} // namespace NF
