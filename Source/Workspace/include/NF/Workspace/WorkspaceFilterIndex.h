#pragma once
// NF::Workspace — Phase 30: Workspace Filter and Search Index
//
// Workspace-level searchable item index with tag and field filters:
//   IndexedItemKind  — Asset / Panel / Tool / Node / Command / Custom
//   IndexedItem      — id + kind + label + tags + fields; isValid()
//   FilterQuery      — text + kind filter + tag set + field predicates
//   WorkspaceFilterIndex — item registry with query evaluation and observers

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// IndexedItemKind — category of a searchable workspace item
// ═════════════════════════════════════════════════════════════════

enum class IndexedItemKind : uint8_t {
    Asset,
    Panel,
    Tool,
    Node,
    Command,
    Custom,
};

inline const char* indexedItemKindName(IndexedItemKind k) {
    switch (k) {
        case IndexedItemKind::Asset:   return "Asset";
        case IndexedItemKind::Panel:   return "Panel";
        case IndexedItemKind::Tool:    return "Tool";
        case IndexedItemKind::Node:    return "Node";
        case IndexedItemKind::Command: return "Command";
        case IndexedItemKind::Custom:  return "Custom";
        default:                       return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// IndexedItem — a workspace item in the search index
// ═════════════════════════════════════════════════════════════════

struct IndexedItem {
    std::string                             id;
    IndexedItemKind                         kind   = IndexedItemKind::Custom;
    std::string                             label;
    std::vector<std::string>                tags;
    std::unordered_map<std::string,std::string> fields; // arbitrary metadata

    bool isValid() const { return !id.empty() && !label.empty(); }

    bool hasTag(const std::string& tag) const {
        return std::find(tags.begin(), tags.end(), tag) != tags.end();
    }

    bool hasField(const std::string& key) const {
        return fields.count(key) > 0;
    }

    const std::string& fieldValue(const std::string& key) const {
        static const std::string empty;
        auto it = fields.find(key);
        return it != fields.end() ? it->second : empty;
    }

    bool operator==(const IndexedItem& o) const { return id == o.id; }
    bool operator!=(const IndexedItem& o) const { return id != o.id; }
};

// ═════════════════════════════════════════════════════════════════
// FilterQuery — describes a search/filter operation
// ═════════════════════════════════════════════════════════════════

struct FilterQuery {
    std::string              text;        // substring match on label (case-insensitive)
    bool                     filterKind = false;
    IndexedItemKind          kind       = IndexedItemKind::Custom;
    std::vector<std::string> requiredTags;   // item must have ALL of these
    std::vector<std::string> requiredFields; // item must have ALL of these field keys

    bool matchesItem(const IndexedItem& item) const {
        // Text match on label
        if (!text.empty()) {
            std::string lowerLabel = item.label;
            std::string lowerText  = text;
            std::transform(lowerLabel.begin(), lowerLabel.end(), lowerLabel.begin(), ::tolower);
            std::transform(lowerText.begin(),  lowerText.end(),  lowerText.begin(),  ::tolower);
            if (lowerLabel.find(lowerText) == std::string::npos) return false;
        }
        // Kind filter
        if (filterKind && item.kind != kind) return false;
        // Required tags
        for (auto& tag : requiredTags)
            if (!item.hasTag(tag)) return false;
        // Required fields
        for (auto& field : requiredFields)
            if (!item.hasField(field)) return false;
        return true;
    }
};

// ═════════════════════════════════════════════════════════════════
// WorkspaceFilterIndex — item registry with query execution
// ═════════════════════════════════════════════════════════════════

class WorkspaceFilterIndex {
public:
    using Observer = std::function<void(const IndexedItem&, bool added)>;

    static constexpr int MAX_ITEMS     = 4096;
    static constexpr int MAX_OBSERVERS = 16;

    // Registry ─────────────────────────────────────────────────

    bool addItem(const IndexedItem& item) {
        if (!item.isValid()) return false;
        if (findById(item.id)) return false;
        if ((int)m_items.size() >= MAX_ITEMS) return false;
        m_items.push_back(item);
        notify(m_items.back(), true);
        return true;
    }

    bool removeItem(const std::string& id) {
        auto it = findIt(id);
        if (it == m_items.end()) return false;
        IndexedItem copy = *it;
        m_items.erase(it);
        notify(copy, false);
        return true;
    }

    bool updateItem(const IndexedItem& item) {
        auto it = findIt(item.id);
        if (it == m_items.end()) return false;
        *it = item;
        notify(*it, true);
        return true;
    }

    bool isIndexed(const std::string& id) const { return findById(id) != nullptr; }

    const IndexedItem* findById(const std::string& id) const {
        for (auto& i : m_items)
            if (i.id == id) return &i;
        return nullptr;
    }

    IndexedItem* findById(const std::string& id) {
        for (auto& i : m_items)
            if (i.id == id) return &i;
        return nullptr;
    }

    // Querying ─────────────────────────────────────────────────

    std::vector<const IndexedItem*> query(const FilterQuery& q) const {
        std::vector<const IndexedItem*> result;
        for (auto& i : m_items)
            if (q.matchesItem(i)) result.push_back(&i);
        return result;
    }

    std::vector<const IndexedItem*> findByKind(IndexedItemKind kind) const {
        std::vector<const IndexedItem*> result;
        for (auto& i : m_items)
            if (i.kind == kind) result.push_back(&i);
        return result;
    }

    std::vector<const IndexedItem*> findByTag(const std::string& tag) const {
        std::vector<const IndexedItem*> result;
        for (auto& i : m_items)
            if (i.hasTag(tag)) result.push_back(&i);
        return result;
    }

    std::vector<std::string> allIds() const {
        std::vector<std::string> ids;
        ids.reserve(m_items.size());
        for (auto& i : m_items) ids.push_back(i.id);
        return ids;
    }

    int  count() const { return (int)m_items.size(); }
    bool empty() const { return m_items.empty(); }

    void clear() { m_items.clear(); }

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

    using ItemIt = std::vector<IndexedItem>::iterator;

    ItemIt findIt(const std::string& id) {
        return std::find_if(m_items.begin(), m_items.end(),
            [&](const IndexedItem& i) { return i.id == id; });
    }

    void notify(const IndexedItem& item, bool added) {
        for (auto& e : m_observers) e.cb(item, added);
    }

    std::vector<IndexedItem>   m_items;
    uint32_t                   m_nextObserverId = 0;
    std::vector<ObserverEntry> m_observers;
};

} // namespace NF
