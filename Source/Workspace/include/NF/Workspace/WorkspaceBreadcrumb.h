#pragma once
// NF::Workspace — Phase 54: Workspace Breadcrumb Navigation
//
// A breadcrumb trail for navigating hierarchical workspace paths
// (e.g. Project → Scene → Entity → Component). UI-agnostic data model.
//
//   BreadcrumbItemKind  — Root / Category / Item / Leaf
//   BreadcrumbItem      — id + label + kind + iconKey + contextData; isValid()
//   BreadcrumbTrail     — ordered stack of items; push/pop/clear/truncate;
//                         current (top); contains/findById; MAX_DEPTH=32
//   BreadcrumbHistory   — circular history of recently-visited trails
//                         (MAX_HISTORY=16); push(trail)/back()/forward()/
//                         canBack()/canForward()
//   BreadcrumbManager   — owns one active trail + history; navigate(item) pushes
//                         and records; popTo(id) pops back to id; reset();
//                         observer callbacks on any change (MAX_OBSERVERS=16)

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════════════
// BreadcrumbItemKind
// ═════════════════════════════════════════════════════════════════════════

enum class BreadcrumbItemKind : uint8_t {
    Root,
    Category,
    Item,
    Leaf,
};

inline const char* breadcrumbItemKindName(BreadcrumbItemKind k) {
    switch (k) {
        case BreadcrumbItemKind::Root:     return "Root";
        case BreadcrumbItemKind::Category: return "Category";
        case BreadcrumbItemKind::Item:     return "Item";
        case BreadcrumbItemKind::Leaf:     return "Leaf";
        default:                           return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════════════
// BreadcrumbItem
// ═════════════════════════════════════════════════════════════════════════

struct BreadcrumbItem {
    std::string          id;
    std::string          label;
    BreadcrumbItemKind   kind        = BreadcrumbItemKind::Item;
    std::string          iconKey;
    std::string          contextData; // arbitrary opaque payload

    [[nodiscard]] bool isValid()  const { return !id.empty() && !label.empty(); }

    bool operator==(const BreadcrumbItem& o) const { return id == o.id; }
    bool operator!=(const BreadcrumbItem& o) const { return id != o.id; }
};

// ═════════════════════════════════════════════════════════════════════════
// BreadcrumbTrail — ordered hierarchy stack
// ═════════════════════════════════════════════════════════════════════════

class BreadcrumbTrail {
public:
    static constexpr size_t MAX_DEPTH = 32;

    [[nodiscard]] bool   empty()  const { return m_items.empty(); }
    [[nodiscard]] size_t depth()  const { return m_items.size(); }

    // Push item onto trail. Rejects invalid or at capacity.
    // Also rejects if an item with the same id is already in the trail
    // (use truncateTo(id) + push to navigate back).
    bool push(const BreadcrumbItem& item) {
        if (!item.isValid()) return false;
        if (m_items.size() >= MAX_DEPTH) return false;
        for (const auto& e : m_items)
            if (e.id == item.id) return false;
        m_items.push_back(item);
        return true;
    }

    // Pop the top item. Returns false if empty.
    bool pop() {
        if (m_items.empty()) return false;
        m_items.pop_back();
        return true;
    }

    // Return pointer to top item, nullptr if empty.
    [[nodiscard]] const BreadcrumbItem* current() const {
        return m_items.empty() ? nullptr : &m_items.back();
    }

    // Return pointer to root (front), nullptr if empty.
    [[nodiscard]] const BreadcrumbItem* root() const {
        return m_items.empty() ? nullptr : &m_items.front();
    }

    [[nodiscard]] const std::vector<BreadcrumbItem>& items() const { return m_items; }

    [[nodiscard]] bool contains(const std::string& id) const {
        return findById(id) != nullptr;
    }

    [[nodiscard]] const BreadcrumbItem* findById(const std::string& id) const {
        for (const auto& e : m_items)
            if (e.id == id) return &e;
        return nullptr;
    }

    // Truncate so that the item with `id` becomes the new top (pop all above it).
    // Returns false if id not found.
    bool truncateTo(const std::string& id) {
        for (size_t i = 0; i < m_items.size(); ++i) {
            if (m_items[i].id == id) {
                m_items.resize(i + 1);
                return true;
            }
        }
        return false;
    }

    void clear() { m_items.clear(); }

    bool operator==(const BreadcrumbTrail& o) const { return m_items == o.m_items; }
    bool operator!=(const BreadcrumbTrail& o) const { return !(*this == o); }

private:
    std::vector<BreadcrumbItem> m_items;
};

// ═════════════════════════════════════════════════════════════════════════
// BreadcrumbHistory — back/forward history of trails
// ═════════════════════════════════════════════════════════════════════════

class BreadcrumbHistory {
public:
    static constexpr size_t MAX_HISTORY = 16;

    // Push a trail snapshot. Clears any forward history.
    void push(const BreadcrumbTrail& trail) {
        // Evict oldest if at capacity
        if (m_entries.size() >= MAX_HISTORY)
            m_entries.erase(m_entries.begin());
        // Truncate forward history
        if (m_cursor < static_cast<int>(m_entries.size()))
            m_entries.resize(static_cast<size_t>(m_cursor));
        m_entries.push_back(trail);
        m_cursor = static_cast<int>(m_entries.size());
    }

    [[nodiscard]] bool canBack()    const { return m_cursor > 1; }
    [[nodiscard]] bool canForward() const {
        return m_cursor < static_cast<int>(m_entries.size());
    }

    // Move cursor back. Returns nullptr if can't go back.
    [[nodiscard]] const BreadcrumbTrail* back() {
        if (!canBack()) return nullptr;
        --m_cursor;
        return &m_entries[static_cast<size_t>(m_cursor - 1)];
    }

    // Move cursor forward. Returns nullptr if can't go forward.
    [[nodiscard]] const BreadcrumbTrail* forward() {
        if (!canForward()) return nullptr;
        const auto* t = &m_entries[static_cast<size_t>(m_cursor)];
        ++m_cursor;
        return t;
    }

    [[nodiscard]] const BreadcrumbTrail* current() const {
        if (m_cursor == 0 || m_entries.empty()) return nullptr;
        return &m_entries[static_cast<size_t>(m_cursor - 1)];
    }

    [[nodiscard]] size_t size()    const { return m_entries.size(); }
    [[nodiscard]] bool   empty()   const { return m_entries.empty(); }

    void clear() {
        m_entries.clear();
        m_cursor = 0;
    }

private:
    std::vector<BreadcrumbTrail> m_entries;
    int                          m_cursor = 0;
};

// ═════════════════════════════════════════════════════════════════════════
// BreadcrumbManager
// ═════════════════════════════════════════════════════════════════════════

using BreadcrumbObserver = std::function<void()>;

class BreadcrumbManager {
public:
    static constexpr size_t MAX_OBSERVERS = 16;

    // Navigate to a new item (append to trail + record in history).
    // Returns false if item invalid or trail is at max depth.
    bool navigate(const BreadcrumbItem& item) {
        if (!m_trail.push(item)) return false;
        m_history.push(m_trail);
        notify();
        return true;
    }

    // Pop back to the item with `id` in the active trail.
    // Returns false if id not found.
    bool popTo(const std::string& id) {
        if (!m_trail.truncateTo(id)) return false;
        m_history.push(m_trail);
        notify();
        return true;
    }

    // Pop one level.
    bool pop() {
        if (!m_trail.pop()) return false;
        m_history.push(m_trail);
        notify();
        return true;
    }

    // Navigate back in history (replaces active trail).
    bool back() {
        const auto* t = m_history.back();
        if (!t) return false;
        m_trail = *t;
        notify();
        return true;
    }

    // Navigate forward in history.
    bool forward() {
        const auto* t = m_history.forward();
        if (!t) return false;
        m_trail = *t;
        notify();
        return true;
    }

    [[nodiscard]] bool canBack()    const { return m_history.canBack(); }
    [[nodiscard]] bool canForward() const { return m_history.canForward(); }

    // Reset clears trail and history.
    void reset() {
        m_trail.clear();
        m_history.clear();
        notify();
    }

    [[nodiscard]] const BreadcrumbTrail&   trail()   const { return m_trail; }
    [[nodiscard]] const BreadcrumbHistory& history() const { return m_history; }

    // ── Observers ─────────────────────────────────────────────────

    bool addObserver(BreadcrumbObserver obs) {
        if (m_observers.size() >= MAX_OBSERVERS) return false;
        m_observers.push_back(std::move(obs));
        return true;
    }

    void clearObservers() { m_observers.clear(); }

private:
    void notify() {
        for (auto& obs : m_observers) obs();
    }

    BreadcrumbTrail                  m_trail;
    BreadcrumbHistory                m_history;
    std::vector<BreadcrumbObserver>  m_observers;
};

} // namespace NF
