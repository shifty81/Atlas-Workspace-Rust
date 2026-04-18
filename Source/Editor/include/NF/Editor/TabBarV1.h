#pragma once
// NF::Editor — Tab bar v1: tab model with ordering, pinning, close/add, active tracking
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Tab State ─────────────────────────────────────────────────────

enum class TabState : uint8_t {
    Normal,
    Modified,   // unsaved changes
    Loading,
    Error,
    Disabled,
};

inline const char* tabStateName(TabState s) {
    switch (s) {
        case TabState::Normal:   return "Normal";
        case TabState::Modified: return "Modified";
        case TabState::Loading:  return "Loading";
        case TabState::Error:    return "Error";
        case TabState::Disabled: return "Disabled";
    }
    return "Unknown";
}

// ── Tab Descriptor ────────────────────────────────────────────────

struct TabDescriptor {
    uint32_t    id      = 0;
    std::string label;
    std::string iconId;
    std::string tooltip;
    TabState    state   = TabState::Normal;
    bool        pinned  = false;
    bool        closable = true;
    bool        visible  = true;

    [[nodiscard]] bool isValid()    const { return id != 0 && !label.empty(); }
    [[nodiscard]] bool isModified() const { return state == TabState::Modified; }
    [[nodiscard]] bool isDisabled() const { return state == TabState::Disabled; }
};

// ── Tab Bar Events ────────────────────────────────────────────────

using TabActivateCallback = std::function<void(uint32_t tabId)>;
using TabCloseCallback    = std::function<bool(uint32_t tabId)>;  // return false to cancel
using TabAddCallback      = std::function<void()>;

// ── Tab Bar V1 ────────────────────────────────────────────────────

class TabBarV1 {
public:
    static constexpr size_t MAX_TABS = 64;

    bool addTab(const TabDescriptor& tab, int insertBefore = -1) {
        if (!tab.isValid()) return false;
        if (m_tabs.size() >= MAX_TABS) return false;
        for (const auto& t : m_tabs) if (t.id == tab.id) return false;

        if (insertBefore >= 0 && static_cast<size_t>(insertBefore) < m_tabs.size()) {
            // Insert before pinned tabs if the new tab is not pinned
            m_tabs.insert(m_tabs.begin() + insertBefore, tab);
        } else {
            // Pinned tabs go to the front
            if (tab.pinned) {
                auto insertPos = pinnedCount();
                m_tabs.insert(m_tabs.begin() + static_cast<ptrdiff_t>(insertPos), tab);
            } else {
                m_tabs.push_back(tab);
            }
        }

        if (m_activeTabId == 0) m_activeTabId = tab.id;
        return true;
    }

    bool removeTab(uint32_t id) {
        auto* t = findTabMut(id);
        if (!t) return false;
        if (t->pinned) return false;  // pinned tabs cannot be closed by default

        bool wasActive = (m_activeTabId == id);
        m_tabs.erase(std::remove_if(m_tabs.begin(), m_tabs.end(),
            [id](const TabDescriptor& tab) { return tab.id == id; }), m_tabs.end());

        if (wasActive && !m_tabs.empty()) {
            m_activeTabId = m_tabs[0].id;
        } else if (m_tabs.empty()) {
            m_activeTabId = 0;
        }
        return true;
    }

    bool forceRemoveTab(uint32_t id) {
        // Remove even pinned tabs
        auto* t = findTabMut(id);
        if (!t) return false;
        t->pinned = false;
        return removeTab(id);
    }

    bool activateTab(uint32_t id) {
        const auto* t = findTab(id);
        if (!t || !t->visible || t->isDisabled()) return false;
        uint32_t prev = m_activeTabId;
        m_activeTabId = id;
        ++m_activationCount;
        if (m_onActivate && prev != id) m_onActivate(id);
        return true;
    }

    bool closeTab(uint32_t id) {
        const auto* t = findTab(id);
        if (!t || !t->closable) return false;
        if (m_onClose) {
            bool allow = m_onClose(id);
            if (!allow) return false;
        }
        return removeTab(id);
    }

    bool pinTab(uint32_t id, bool pin) {
        auto* t = findTabMut(id);
        if (!t) return false;
        t->pinned = pin;
        // Re-sort: pinned tabs to front
        if (pin) {
            TabDescriptor saved = *t;
            m_tabs.erase(std::remove_if(m_tabs.begin(), m_tabs.end(),
                [id](const TabDescriptor& tab) { return tab.id == id; }), m_tabs.end());
            size_t pos = pinnedCount();
            m_tabs.insert(m_tabs.begin() + static_cast<ptrdiff_t>(pos), saved);
        }
        return true;
    }

    bool setTabLabel(uint32_t id, const std::string& label) {
        auto* t = findTabMut(id);
        if (!t || label.empty()) return false;
        t->label = label;
        return true;
    }

    bool setTabState(uint32_t id, TabState state) {
        auto* t = findTabMut(id);
        if (!t) return false;
        t->state = state;
        return true;
    }

    bool setTabVisible(uint32_t id, bool visible) {
        auto* t = findTabMut(id);
        if (!t) return false;
        t->visible = visible;
        if (!visible && m_activeTabId == id) {
            // Activate first visible
            for (const auto& tab : m_tabs) {
                if (tab.visible && tab.id != id) { activateTab(tab.id); break; }
            }
        }
        return true;
    }

    bool reorder(uint32_t id, size_t newIndex) {
        auto it = std::find_if(m_tabs.begin(), m_tabs.end(),
            [id](const TabDescriptor& t) { return t.id == id; });
        if (it == m_tabs.end()) return false;
        if (it->pinned) return false;  // pinned tabs cannot be reordered out of pinned zone

        size_t pinned = pinnedCount();
        if (newIndex < pinned) newIndex = pinned;
        if (newIndex >= m_tabs.size()) newIndex = m_tabs.size() - 1;

        TabDescriptor saved = *it;
        m_tabs.erase(it);
        m_tabs.insert(m_tabs.begin() + static_cast<ptrdiff_t>(newIndex), saved);
        return true;
    }

    [[nodiscard]] const TabDescriptor* findTab(uint32_t id) const {
        for (const auto& t : m_tabs) if (t.id == id) return &t;
        return nullptr;
    }

    [[nodiscard]] uint32_t nextTab() const {
        if (m_tabs.empty()) return 0;
        auto it = std::find_if(m_tabs.begin(), m_tabs.end(),
            [this](const TabDescriptor& t) { return t.id == m_activeTabId; });
        if (it == m_tabs.end()) return m_tabs.front().id;
        ++it;
        while (it != m_tabs.end()) { if (it->visible && !it->isDisabled()) return it->id; ++it; }
        return m_activeTabId;
    }

    [[nodiscard]] uint32_t prevTab() const {
        if (m_tabs.empty()) return 0;
        auto it = std::find_if(m_tabs.rbegin(), m_tabs.rend(),
            [this](const TabDescriptor& t) { return t.id == m_activeTabId; });
        if (it == m_tabs.rend()) return m_tabs.back().id;
        ++it;
        while (it != m_tabs.rend()) { if (it->visible && !it->isDisabled()) return it->id; ++it; }
        return m_activeTabId;
    }

    void setOnActivate(TabActivateCallback cb) { m_onActivate = std::move(cb); }
    void setOnClose(TabCloseCallback cb)       { m_onClose    = std::move(cb); }
    void setOnAdd(TabAddCallback cb)           { m_onAdd      = std::move(cb); }

    void triggerAdd() { if (m_onAdd) m_onAdd(); }

    [[nodiscard]] uint32_t activeTabId()     const { return m_activeTabId;       }
    [[nodiscard]] size_t   tabCount()        const { return m_tabs.size();        }
    [[nodiscard]] size_t   activationCount() const { return m_activationCount;    }
    [[nodiscard]] const std::vector<TabDescriptor>& tabs() const { return m_tabs; }

    [[nodiscard]] size_t pinnedCount() const {
        size_t n = 0;
        for (const auto& t : m_tabs) if (t.pinned) ++n;
        return n;
    }
    [[nodiscard]] size_t modifiedCount() const {
        size_t n = 0;
        for (const auto& t : m_tabs) if (t.isModified()) ++n;
        return n;
    }

private:
    TabDescriptor* findTabMut(uint32_t id) {
        for (auto& t : m_tabs) if (t.id == id) return &t;
        return nullptr;
    }

    std::vector<TabDescriptor> m_tabs;
    TabActivateCallback        m_onActivate;
    TabCloseCallback           m_onClose;
    TabAddCallback             m_onAdd;
    uint32_t                   m_activeTabId     = 0;
    size_t                     m_activationCount = 0;
};

} // namespace NF
