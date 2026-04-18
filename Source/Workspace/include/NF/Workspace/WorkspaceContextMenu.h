#pragma once
// NF::Workspace — Phase 26: Workspace Context Menu System
//
// Workspace-level context menu definition and lifecycle:
//   MenuItemKind    — Action / Separator / Submenu
//   ContextMenuItem — id + label + kind + enabled + shortcut + icon; isValid()
//   ContextMenu     — ordered list of items; add/remove/find; buildSubmenu
//   ContextMenuManager — named menu registry; open/close lifecycle; observers

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// MenuItemKind — type classification for a context menu entry
// ═════════════════════════════════════════════════════════════════

enum class MenuItemKind : uint8_t {
    Action,
    Separator,
    Submenu,
};

inline const char* menuItemKindName(MenuItemKind k) {
    switch (k) {
        case MenuItemKind::Action:    return "Action";
        case MenuItemKind::Separator: return "Separator";
        case MenuItemKind::Submenu:   return "Submenu";
        default:                      return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// ContextMenuItem — a single entry in a context menu
// ═════════════════════════════════════════════════════════════════

struct ContextMenuItem {
    std::string  id;
    std::string  label;
    MenuItemKind kind       = MenuItemKind::Action;
    bool         enabled    = true;
    std::string  shortcut;   // display-only hint (e.g. "Ctrl+Z")
    std::string  icon;       // optional icon id

    // Separator convenience constructor
    static ContextMenuItem separator(const std::string& id) {
        return {id, "", MenuItemKind::Separator, true, "", ""};
    }

    bool isValid() const {
        if (kind == MenuItemKind::Separator) return !id.empty();
        return !id.empty() && !label.empty();
    }

    bool operator==(const ContextMenuItem& o) const { return id == o.id; }
    bool operator!=(const ContextMenuItem& o) const { return id != o.id; }
};

// ═════════════════════════════════════════════════════════════════
// ContextMenu — ordered list of items with optional submenu trees
// ═════════════════════════════════════════════════════════════════

class ContextMenu {
public:
    static constexpr int MAX_ITEMS = 128;

    ContextMenu() = default;
    explicit ContextMenu(std::string menuId) : m_id(std::move(menuId)) {}

    const std::string& id() const { return m_id; }
    bool isValid() const { return !m_id.empty(); }

    // Item management ──────────────────────────────────────────

    bool addItem(const ContextMenuItem& item) {
        if (!item.isValid()) return false;
        if (findItem(item.id)) return false;
        if ((int)m_items.size() >= MAX_ITEMS) return false;
        m_items.push_back(item);
        return true;
    }

    bool removeItem(const std::string& id) {
        auto it = std::find_if(m_items.begin(), m_items.end(),
            [&](const ContextMenuItem& i) { return i.id == id; });
        if (it == m_items.end()) return false;
        m_items.erase(it);
        m_submenus.erase(
            std::remove_if(m_submenus.begin(), m_submenus.end(),
                [&](const ContextMenu& sub) { return sub.id() == id; }),
            m_submenus.end());
        return true;
    }

    bool updateItem(const ContextMenuItem& item) {
        for (auto& i : m_items) {
            if (i.id == item.id) { i = item; return true; }
        }
        return false;
    }

    ContextMenuItem* findItem(const std::string& id) {
        for (auto& i : m_items)
            if (i.id == id) return &i;
        return nullptr;
    }

    const ContextMenuItem* findItem(const std::string& id) const {
        for (auto& i : m_items)
            if (i.id == id) return &i;
        return nullptr;
    }

    bool contains(const std::string& id) const { return findItem(id) != nullptr; }
    int  itemCount() const { return (int)m_items.size(); }
    bool empty()     const { return m_items.empty(); }

    const std::vector<ContextMenuItem>& items() const { return m_items; }

    // Submenu attachment ───────────────────────────────────────

    bool attachSubmenu(ContextMenu submenu) {
        if (!submenu.isValid()) return false;
        // parent item must exist and be Submenu kind
        auto* item = findItem(submenu.id());
        if (!item || item->kind != MenuItemKind::Submenu) return false;
        // Remove old submenu if present
        m_submenus.erase(
            std::remove_if(m_submenus.begin(), m_submenus.end(),
                [&](const ContextMenu& s) { return s.id() == submenu.id(); }),
            m_submenus.end());
        m_submenus.push_back(std::move(submenu));
        return true;
    }

    const ContextMenu* findSubmenu(const std::string& id) const {
        for (auto& s : m_submenus)
            if (s.id() == id) return &s;
        return nullptr;
    }

    void clear() { m_items.clear(); m_submenus.clear(); }

private:
    std::string                 m_id;
    std::vector<ContextMenuItem> m_items;
    std::vector<ContextMenu>    m_submenus;
};

// ═════════════════════════════════════════════════════════════════
// ContextMenuManager — named menu registry with open/close lifecycle
// ═════════════════════════════════════════════════════════════════

class ContextMenuManager {
public:
    using ActionObserver = std::function<void(const std::string& menuId,
                                              const std::string& itemId)>;
    using LifecycleObserver = std::function<void(const std::string& menuId, bool opened)>;

    static constexpr int MAX_MENUS     = 64;
    static constexpr int MAX_OBSERVERS = 16;

    // Registry ─────────────────────────────────────────────────

    bool registerMenu(const ContextMenu& menu) {
        if (!menu.isValid()) return false;
        if (findMenu(menu.id())) return false;
        if ((int)m_menus.size() >= MAX_MENUS) return false;
        m_menus.push_back(menu);
        return true;
    }

    bool unregisterMenu(const std::string& id) {
        auto it = std::find_if(m_menus.begin(), m_menus.end(),
            [&](const ContextMenu& m) { return m.id() == id; });
        if (it == m_menus.end()) return false;
        if (m_openMenuId == id) closeMenu();
        m_menus.erase(it);
        return true;
    }

    bool isRegistered(const std::string& id) const { return findMenu(id) != nullptr; }

    const ContextMenu* findMenu(const std::string& id) const {
        for (auto& m : m_menus)
            if (m.id() == id) return &m;
        return nullptr;
    }

    ContextMenu* findMenu(const std::string& id) {
        for (auto& m : m_menus)
            if (m.id() == id) return &m;
        return nullptr;
    }

    std::vector<std::string> allMenuIds() const {
        std::vector<std::string> ids;
        ids.reserve(m_menus.size());
        for (auto& m : m_menus) ids.push_back(m.id());
        return ids;
    }

    // Open / close lifecycle ───────────────────────────────────

    bool openMenu(const std::string& id) {
        if (!findMenu(id)) return false;
        if (m_openMenuId == id) return false;
        if (!m_openMenuId.empty()) closeMenu();
        m_openMenuId = id;
        notifyLifecycle(id, true);
        return true;
    }

    bool closeMenu() {
        if (m_openMenuId.empty()) return false;
        std::string closing = m_openMenuId;
        m_openMenuId.clear();
        notifyLifecycle(closing, false);
        return true;
    }

    bool isOpen(const std::string& id) const { return m_openMenuId == id; }
    bool hasOpenMenu()                  const { return !m_openMenuId.empty(); }
    const std::string& openMenuId()     const { return m_openMenuId; }

    // Activate action ──────────────────────────────────────────

    bool activateItem(const std::string& menuId, const std::string& itemId) {
        const ContextMenu* menu = findMenu(menuId);
        if (!menu) return false;
        const ContextMenuItem* item = menu->findItem(itemId);
        if (!item || !item->enabled || item->kind != MenuItemKind::Action) return false;
        notifyAction(menuId, itemId);
        return true;
    }

    // Bulk ─────────────────────────────────────────────────────

    void clear() {
        m_openMenuId.clear();
        m_menus.clear();
    }

    // Observers ────────────────────────────────────────────────

    uint32_t addActionObserver(ActionObserver cb) {
        if (!cb || (int)m_actionObservers.size() >= MAX_OBSERVERS) return 0;
        uint32_t id = ++m_nextObserverId;
        m_actionObservers.push_back({id, std::move(cb)});
        return id;
    }

    uint32_t addLifecycleObserver(LifecycleObserver cb) {
        if (!cb || (int)m_lifecycleObservers.size() >= MAX_OBSERVERS) return 0;
        uint32_t id = ++m_nextObserverId;
        m_lifecycleObservers.push_back({id, std::move(cb)});
        return id;
    }

    void removeObserver(uint32_t id) {
        m_actionObservers.erase(
            std::remove_if(m_actionObservers.begin(), m_actionObservers.end(),
                [id](const ActionEntry& e) { return e.id == id; }),
            m_actionObservers.end());
        m_lifecycleObservers.erase(
            std::remove_if(m_lifecycleObservers.begin(), m_lifecycleObservers.end(),
                [id](const LifecycleEntry& e) { return e.id == id; }),
            m_lifecycleObservers.end());
    }

    void clearObservers() {
        m_actionObservers.clear();
        m_lifecycleObservers.clear();
    }

private:
    struct ActionEntry    { uint32_t id; ActionObserver    cb; };
    struct LifecycleEntry { uint32_t id; LifecycleObserver cb; };

    void notifyAction(const std::string& menuId, const std::string& itemId) {
        for (auto& e : m_actionObservers) e.cb(menuId, itemId);
    }
    void notifyLifecycle(const std::string& menuId, bool opened) {
        for (auto& e : m_lifecycleObservers) e.cb(menuId, opened);
    }

    std::vector<ContextMenu>    m_menus;
    std::string                 m_openMenuId;
    uint32_t                    m_nextObserverId = 0;
    std::vector<ActionEntry>    m_actionObservers;
    std::vector<LifecycleEntry> m_lifecycleObservers;
};

} // namespace NF
