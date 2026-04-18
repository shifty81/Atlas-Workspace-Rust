#pragma once
// NF::Workspace — Phase 48: Workspace Activity Bar
//
// Activity bar data model — the vertically-stacked tool/action navigator
// on the workspace left edge.  The renderer queries this model each frame;
// the model never touches the UI directly.
//
//   ActivityItemKind   — Tool / Action / Separator
//   ActivityBarItem    — id + label + toolId|commandId + kind + enabled/pinned
//   ActivityBarSection — ordered, named collection (MAX_ITEMS=32)
//   ActivityBarManager — section registry (MAX_SECTIONS=8) + active tracking
//                        + observer callbacks (MAX_OBSERVERS=16)

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════════════
// ActivityItemKind — what clicking the item does
// ═════════════════════════════════════════════════════════════════════════

enum class ActivityItemKind : uint8_t {
    Tool,       // activates/deactivates a registered IHostedTool
    Action,     // fires a named command (commandId)
    Separator,  // visual gap — no interaction
};

inline const char* activityItemKindName(ActivityItemKind k) {
    switch (k) {
        case ActivityItemKind::Tool:      return "Tool";
        case ActivityItemKind::Action:    return "Action";
        case ActivityItemKind::Separator: return "Separator";
        default:                          return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════════════
// ActivityBarItem — a single entry in the activity bar
// ═════════════════════════════════════════════════════════════════════════

struct ActivityBarItem {
    std::string     id;          // unique item id
    std::string     label;       // display label
    std::string     iconKey;     // optional future icon id
    std::string     toolId;      // for Kind::Tool — toolId passed to ToolRegistry
    std::string     commandId;   // for Kind::Action — command to dispatch
    ActivityItemKind kind    = ActivityItemKind::Tool;
    bool             enabled = true;
    bool             pinned  = false;  // pinned items persist across tool switches

    [[nodiscard]] bool isValid() const {
        if (id.empty() || label.empty()) return false;
        if (kind == ActivityItemKind::Tool   && toolId.empty())    return false;
        if (kind == ActivityItemKind::Action && commandId.empty()) return false;
        return true;
    }

    [[nodiscard]] bool isTool()      const { return kind == ActivityItemKind::Tool; }
    [[nodiscard]] bool isAction()    const { return kind == ActivityItemKind::Action; }
    [[nodiscard]] bool isSeparator() const { return kind == ActivityItemKind::Separator; }

    bool operator==(const ActivityBarItem& o) const { return id == o.id; }
    bool operator!=(const ActivityBarItem& o) const { return id != o.id; }

    // Factory helpers
    static ActivityBarItem makeTool(const std::string& id,
                                    const std::string& label,
                                    const std::string& toolId) {
        ActivityBarItem item;
        item.id     = id;
        item.label  = label;
        item.toolId = toolId;
        item.kind   = ActivityItemKind::Tool;
        return item;
    }

    static ActivityBarItem makeAction(const std::string& id,
                                      const std::string& label,
                                      const std::string& commandId) {
        ActivityBarItem item;
        item.id        = id;
        item.label     = label;
        item.commandId = commandId;
        item.kind      = ActivityItemKind::Action;
        return item;
    }

    static ActivityBarItem makeSeparator(const std::string& id) {
        ActivityBarItem item;
        item.id    = id;
        item.label = "---";
        item.kind  = ActivityItemKind::Separator;
        return item;
    }
};

// ═════════════════════════════════════════════════════════════════════════
// ActivityBarSection — named, ordered collection of items
// ═════════════════════════════════════════════════════════════════════════

class ActivityBarSection {
public:
    static constexpr size_t MAX_ITEMS = 32;

    explicit ActivityBarSection(const std::string& name) : m_name(name) {}

    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] bool empty()  const { return m_items.empty(); }
    [[nodiscard]] size_t count() const { return m_items.size(); }
    [[nodiscard]] const std::vector<ActivityBarItem>& items() const { return m_items; }

    // Add an item to the end.  Returns false on duplicate id or capacity.
    bool addItem(const ActivityBarItem& item) {
        if (m_items.size() >= MAX_ITEMS) return false;
        if (!item.id.empty()) {
            for (const auto& it : m_items)
                if (it.id == item.id) return false;
        }
        m_items.push_back(item);
        return true;
    }

    // Remove item by id.  Returns false if not found.
    bool removeItem(const std::string& id) {
        auto it = std::find_if(m_items.begin(), m_items.end(),
                               [&](const ActivityBarItem& x){ return x.id == id; });
        if (it == m_items.end()) return false;
        m_items.erase(it);
        return true;
    }

    [[nodiscard]] const ActivityBarItem* findItem(const std::string& id) const {
        for (const auto& it : m_items)
            if (it.id == id) return &it;
        return nullptr;
    }

    [[nodiscard]] ActivityBarItem* findItemMut(const std::string& id) {
        for (auto& it : m_items)
            if (it.id == id) return &it;
        return nullptr;
    }

    [[nodiscard]] bool contains(const std::string& id) const {
        return findItem(id) != nullptr;
    }

    void clear() { m_items.clear(); }

private:
    std::string              m_name;
    std::vector<ActivityBarItem> m_items;
};

// ═════════════════════════════════════════════════════════════════════════
// ActivityBarManager — section registry with active-item tracking
// ═════════════════════════════════════════════════════════════════════════

using ActivityBarObserver = std::function<void(const std::string& /*itemId*/,
                                               bool             /*activated*/)>;

class ActivityBarManager {
public:
    static constexpr size_t MAX_SECTIONS  = 8;
    static constexpr size_t MAX_OBSERVERS = 16;

    // ── Section management ────────────────────────────────────────

    // Create a new named section.  Returns nullptr on duplicate name or capacity.
    ActivityBarSection* createSection(const std::string& name) {
        if (name.empty() || m_sections.size() >= MAX_SECTIONS) return nullptr;
        for (auto& s : m_sections)
            if (s.name() == name) return nullptr;
        m_sections.emplace_back(name);
        return &m_sections.back();
    }

    // Remove section by name.  Returns false if not found.
    bool removeSection(const std::string& name) {
        auto it = std::find_if(m_sections.begin(), m_sections.end(),
                               [&](const ActivityBarSection& s){ return s.name() == name; });
        if (it == m_sections.end()) return false;
        m_sections.erase(it);
        return true;
    }

    [[nodiscard]] ActivityBarSection* findSection(const std::string& name) {
        for (auto& s : m_sections)
            if (s.name() == name) return &s;
        return nullptr;
    }

    [[nodiscard]] const ActivityBarSection* findSection(const std::string& name) const {
        for (const auto& s : m_sections)
            if (s.name() == name) return &s;
        return nullptr;
    }

    [[nodiscard]] bool hasSection(const std::string& name) const {
        return findSection(name) != nullptr;
    }

    [[nodiscard]] size_t sectionCount() const { return m_sections.size(); }
    [[nodiscard]] bool empty() const { return m_sections.empty(); }

    [[nodiscard]] const std::vector<ActivityBarSection>& sections() const { return m_sections; }

    // ── Item convenience ──────────────────────────────────────────

    // Add an item to a named section (section must exist).
    bool addItem(const std::string& sectionName, const ActivityBarItem& item) {
        auto* sec = findSection(sectionName);
        if (!sec) return false;
        return sec->addItem(item);
    }

    // Remove item from the section it lives in.  Searches all sections.
    bool removeItem(const std::string& itemId) {
        for (auto& sec : m_sections)
            if (sec.removeItem(itemId)) return true;
        return false;
    }

    [[nodiscard]] const ActivityBarItem* findItem(const std::string& itemId) const {
        for (const auto& sec : m_sections) {
            const auto* it = sec.findItem(itemId);
            if (it) return it;
        }
        return nullptr;
    }

    // ── Active-item tracking ──────────────────────────────────────

    // Set the active item (by id).  Returns false if item not found or disabled.
    // Fires observers with activated=true for the new id, and activated=false for
    // the previously active id (if different).
    bool setActiveItem(const std::string& itemId) {
        const auto* item = findItem(itemId);
        if (!item) return false;
        if (!item->enabled) return false;

        std::string prev = m_activeItemId;
        m_activeItemId = itemId;
        if (prev != itemId) {
            if (!prev.empty()) fireObservers(prev, false);
            fireObservers(m_activeItemId, true);
        }
        return true;
    }

    // Clear the active item (returns to no-selection state).
    void clearActiveItem() {
        if (!m_activeItemId.empty()) {
            std::string prev = m_activeItemId;
            m_activeItemId.clear();
            fireObservers(prev, false);
        }
    }

    [[nodiscard]] const std::string& activeItemId() const { return m_activeItemId; }
    [[nodiscard]] bool hasActiveItem() const { return !m_activeItemId.empty(); }

    [[nodiscard]] bool isActive(const std::string& itemId) const {
        return !itemId.empty() && m_activeItemId == itemId;
    }

    // ── Enable / disable ──────────────────────────────────────────

    bool enableItem(const std::string& itemId, bool enabled) {
        for (auto& sec : m_sections) {
            auto* item = sec.findItemMut(itemId);
            if (item) { item->enabled = enabled; return true; }
        }
        return false;
    }

    // ── Observers ─────────────────────────────────────────────────

    bool addObserver(ActivityBarObserver obs) {
        if (m_observers.size() >= MAX_OBSERVERS) return false;
        m_observers.push_back(std::move(obs));
        return true;
    }

    void clearObservers() { m_observers.clear(); }

    // ── Clear ─────────────────────────────────────────────────────

    void clear() {
        m_sections.clear();
        m_activeItemId.clear();
        m_observers.clear();
    }

private:
    void fireObservers(const std::string& itemId, bool activated) {
        for (auto& obs : m_observers)
            obs(itemId, activated);
    }

    std::vector<ActivityBarSection>  m_sections;
    std::vector<ActivityBarObserver> m_observers;
    std::string                      m_activeItemId;
};

} // namespace NF
