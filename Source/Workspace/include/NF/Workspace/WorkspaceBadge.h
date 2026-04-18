#pragma once
// NF::Workspace — Phase 27: Workspace Badge and Icon Registry
//
// Workspace-level badge overlay and icon management:
//   BadgeKind       — Info / Warning / Error / Success / Count / Custom
//   Badge           — id + kind + label + count + targetId; isValid()
//   IconEntry       — id + path/alias + size + category; isValid()
//   BadgeRegistry   — attach/detach badges to target ids; query per target
//   IconRegistry    — register/find icons by id or category; alias support

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// BadgeKind — semantic badge classification
// ═════════════════════════════════════════════════════════════════

enum class BadgeKind : uint8_t {
    Info,
    Warning,
    Error,
    Success,
    Count,   // numeric badge (e.g. notification count)
    Custom,
};

inline const char* badgeKindName(BadgeKind k) {
    switch (k) {
        case BadgeKind::Info:    return "Info";
        case BadgeKind::Warning: return "Warning";
        case BadgeKind::Error:   return "Error";
        case BadgeKind::Success: return "Success";
        case BadgeKind::Count:   return "Count";
        case BadgeKind::Custom:  return "Custom";
        default:                 return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// Badge — overlay indicator attached to a UI target
// ═════════════════════════════════════════════════════════════════

struct Badge {
    std::string id;
    std::string targetId;  // element/panel/tool this badge is attached to
    BadgeKind   kind      = BadgeKind::Info;
    std::string label;     // short label text (e.g. "!", "42")
    int         count     = 0;  // for BadgeKind::Count
    bool        visible   = true;

    bool isValid() const { return !id.empty() && !targetId.empty(); }

    bool operator==(const Badge& o) const { return id == o.id; }
    bool operator!=(const Badge& o) const { return id != o.id; }
};

// ═════════════════════════════════════════════════════════════════
// IconEntry — registered icon asset
// ═════════════════════════════════════════════════════════════════

struct IconEntry {
    std::string id;
    std::string path;      // asset path or resource name
    std::string alias;     // optional short alias
    std::string category;  // grouping (e.g. "editor", "ui", "toolbar")
    uint32_t    size = 16; // logical pixel size

    bool isValid() const { return !id.empty() && !path.empty(); }

    bool operator==(const IconEntry& o) const { return id == o.id; }
    bool operator!=(const IconEntry& o) const { return id != o.id; }
};

// ═════════════════════════════════════════════════════════════════
// BadgeRegistry — attach/detach badges, query per target
// ═════════════════════════════════════════════════════════════════

class BadgeRegistry {
public:
    using Observer = std::function<void(const Badge&, bool attached)>;

    static constexpr int MAX_BADGES    = 512;
    static constexpr int MAX_OBSERVERS = 16;

    bool attach(const Badge& badge) {
        if (!badge.isValid()) return false;
        if (findById(badge.id)) return false;
        if ((int)m_badges.size() >= MAX_BADGES) return false;
        m_badges.push_back(badge);
        notify(badge, true);
        return true;
    }

    bool detach(const std::string& id) {
        auto it = std::find_if(m_badges.begin(), m_badges.end(),
            [&](const Badge& b) { return b.id == id; });
        if (it == m_badges.end()) return false;
        Badge copy = *it;
        m_badges.erase(it);
        notify(copy, false);
        return true;
    }

    bool update(const Badge& badge) {
        for (auto& b : m_badges) {
            if (b.id == badge.id) {
                b = badge;
                notify(b, true);
                return true;
            }
        }
        return false;
    }

    bool isAttached(const std::string& id) const { return findById(id) != nullptr; }

    const Badge* findById(const std::string& id) const {
        for (auto& b : m_badges)
            if (b.id == id) return &b;
        return nullptr;
    }

    Badge* findById(const std::string& id) {
        for (auto& b : m_badges)
            if (b.id == id) return &b;
        return nullptr;
    }

    std::vector<const Badge*> findByTarget(const std::string& targetId) const {
        std::vector<const Badge*> result;
        for (auto& b : m_badges)
            if (b.targetId == targetId) result.push_back(&b);
        return result;
    }

    std::vector<const Badge*> findByKind(BadgeKind kind) const {
        std::vector<const Badge*> result;
        for (auto& b : m_badges)
            if (b.kind == kind) result.push_back(&b);
        return result;
    }

    bool setVisible(const std::string& id, bool visible) {
        Badge* b = findById(id);
        if (!b) return false;
        b->visible = visible;
        notify(*b, true);
        return true;
    }

    bool setCount(const std::string& id, int count) {
        Badge* b = findById(id);
        if (!b || b->kind != BadgeKind::Count) return false;
        b->count = count;
        notify(*b, true);
        return true;
    }

    int  totalCount() const { return (int)m_badges.size(); }
    bool empty()      const { return m_badges.empty(); }

    void clear() { m_badges.clear(); }

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

    void notify(const Badge& badge, bool attached) {
        for (auto& e : m_observers) e.cb(badge, attached);
    }

    std::vector<Badge>         m_badges;
    uint32_t                   m_nextObserverId = 0;
    std::vector<ObserverEntry> m_observers;
};

// ═════════════════════════════════════════════════════════════════
// IconRegistry — register/find icons by id, alias, or category
// ═════════════════════════════════════════════════════════════════

class IconRegistry {
public:
    static constexpr int MAX_ICONS = 1024;

    bool registerIcon(const IconEntry& icon) {
        if (!icon.isValid()) return false;
        if (findById(icon.id)) return false;
        if ((int)m_icons.size() >= MAX_ICONS) return false;
        m_icons.push_back(icon);
        return true;
    }

    bool unregisterIcon(const std::string& id) {
        auto it = std::find_if(m_icons.begin(), m_icons.end(),
            [&](const IconEntry& i) { return i.id == id; });
        if (it == m_icons.end()) return false;
        m_icons.erase(it);
        return true;
    }

    bool isRegistered(const std::string& id) const { return findById(id) != nullptr; }

    const IconEntry* findById(const std::string& id) const {
        for (auto& i : m_icons)
            if (i.id == id) return &i;
        return nullptr;
    }

    const IconEntry* findByAlias(const std::string& alias) const {
        for (auto& i : m_icons)
            if (!i.alias.empty() && i.alias == alias) return &i;
        return nullptr;
    }

    // Find by id first, then alias
    const IconEntry* find(const std::string& idOrAlias) const {
        const IconEntry* e = findById(idOrAlias);
        return e ? e : findByAlias(idOrAlias);
    }

    std::vector<const IconEntry*> findByCategory(const std::string& category) const {
        std::vector<const IconEntry*> result;
        for (auto& i : m_icons)
            if (i.category == category) result.push_back(&i);
        return result;
    }

    std::vector<std::string> allIds() const {
        std::vector<std::string> ids;
        ids.reserve(m_icons.size());
        for (auto& i : m_icons) ids.push_back(i.id);
        return ids;
    }

    int  count() const { return (int)m_icons.size(); }
    bool empty() const { return m_icons.empty(); }

    void clear() { m_icons.clear(); }

private:
    std::vector<IconEntry> m_icons;
};

} // namespace NF
