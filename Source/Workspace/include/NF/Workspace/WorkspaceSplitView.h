#pragma once
// NF::Workspace — Phase 33: Workspace Split View / Tab Groups
//
// Workspace split-view layout with panes, tab groups, and a view controller:
//   SplitOrientation — Horizontal / Vertical / None
//   TabEntry         — id + label + closeable; isValid()
//   TabGroup         — ordered tab list with active tab tracking
//   SplitPane        — leaf (TabGroup) or branch (two child panes + orientation)
//   SplitViewController — root pane management; add/remove/split/focus/close

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// SplitOrientation — direction of a split in the view
// ═════════════════════════════════════════════════════════════════

enum class SplitOrientation : uint8_t {
    None,       // leaf pane (not split)
    Horizontal, // side-by-side
    Vertical,   // stacked top/bottom
};

inline const char* splitOrientationName(SplitOrientation o) {
    switch (o) {
        case SplitOrientation::None:       return "None";
        case SplitOrientation::Horizontal: return "Horizontal";
        case SplitOrientation::Vertical:   return "Vertical";
        default:                           return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// TabEntry — one tab in a tab group
// ═════════════════════════════════════════════════════════════════

struct TabEntry {
    std::string id;
    std::string label;
    bool        closeable = true;

    bool isValid() const { return !id.empty() && !label.empty(); }
    bool operator==(const TabEntry& o) const { return id == o.id; }
};

// ═════════════════════════════════════════════════════════════════
// TabGroup — ordered tab list with an active tab
// ═════════════════════════════════════════════════════════════════

struct TabGroup {
    static constexpr int kMaxTabs = 64;

    std::string           groupId;
    std::vector<TabEntry> tabs;
    std::string           activeTabId;

    bool isValid() const { return !groupId.empty(); }

    bool addTab(const TabEntry& tab) {
        if (!tab.isValid()) return false;
        if (static_cast<int>(tabs.size()) >= kMaxTabs) return false;
        for (auto& t : tabs)
            if (t.id == tab.id) return false; // duplicate
        tabs.push_back(tab);
        if (activeTabId.empty()) activeTabId = tab.id;
        return true;
    }

    bool removeTab(const std::string& id) {
        auto it = std::find_if(tabs.begin(), tabs.end(),
            [&](const TabEntry& t){ return t.id == id; });
        if (it == tabs.end()) return false;
        bool wasActive = (id == activeTabId);
        tabs.erase(it);
        if (wasActive) {
            activeTabId = tabs.empty() ? std::string{} : tabs.front().id;
        }
        return true;
    }

    bool setActiveTab(const std::string& id) {
        for (auto& t : tabs)
            if (t.id == id) { activeTabId = id; return true; }
        return false;
    }

    bool hasTab(const std::string& id) const {
        return std::any_of(tabs.begin(), tabs.end(),
            [&](const TabEntry& t){ return t.id == id; });
    }

    int tabCount() const { return static_cast<int>(tabs.size()); }
    bool empty()   const { return tabs.empty(); }
};

// ═════════════════════════════════════════════════════════════════
// SplitPane — tree node: either a leaf (TabGroup) or a branch
// ═════════════════════════════════════════════════════════════════

struct SplitPane {
    std::string      id;
    SplitOrientation orientation = SplitOrientation::None; // None = leaf
    TabGroup         tabGroup;   // used only when leaf
    float            splitRatio  = 0.5f; // branch ratio [0..1]

    // child panes for branch nodes (owned via shared_ptr for simplicity)
    std::shared_ptr<SplitPane> first;
    std::shared_ptr<SplitPane> second;

    bool isLeaf()   const { return orientation == SplitOrientation::None; }
    bool isBranch() const { return !isLeaf(); }
    bool isValid()  const { return !id.empty(); }
};

// ═════════════════════════════════════════════════════════════════
// SplitViewController — manages the root pane tree
// ═════════════════════════════════════════════════════════════════

struct SplitViewController {
    using ObserverFn = std::function<void()>;

    SplitViewController() {
        // initialise with a single empty leaf root
        root_       = std::make_shared<SplitPane>();
        root_->id   = "root";
        activePaneId_ = "root";
    }

    // ── Pane access ──────────────────────────────────────────────

    SplitPane* root() const { return root_.get(); }

    SplitPane* findPane(const std::string& id) const {
        return findIn(root_.get(), id);
    }

    bool containsPane(const std::string& id) const {
        return findPane(id) != nullptr;
    }

    const std::string& activePaneId() const { return activePaneId_; }

    bool setActivePane(const std::string& id) {
        if (!containsPane(id)) return false;
        activePaneId_ = id;
        notify();
        return true;
    }

    // ── Tab operations on a leaf pane ────────────────────────────

    bool addTab(const std::string& paneId, const TabEntry& tab) {
        SplitPane* p = findPane(paneId);
        if (!p || !p->isLeaf()) return false;
        bool ok = p->tabGroup.addTab(tab);
        if (ok) notify();
        return ok;
    }

    bool removeTab(const std::string& paneId, const std::string& tabId) {
        SplitPane* p = findPane(paneId);
        if (!p || !p->isLeaf()) return false;
        bool ok = p->tabGroup.removeTab(tabId);
        if (ok) notify();
        return ok;
    }

    bool setActiveTab(const std::string& paneId, const std::string& tabId) {
        SplitPane* p = findPane(paneId);
        if (!p || !p->isLeaf()) return false;
        bool ok = p->tabGroup.setActiveTab(tabId);
        if (ok) notify();
        return ok;
    }

    // ── Split operations ─────────────────────────────────────────

    // Split a leaf pane: it becomes a branch with two leaf children.
    // The existing tab group moves into 'first'; 'secondId' is the new pane.
    bool splitPane(const std::string& paneId,
                   SplitOrientation orientation,
                   const std::string& secondId,
                   float ratio = 0.5f) {
        if (orientation == SplitOrientation::None) return false;
        SplitPane* p = findPane(paneId);
        if (!p || !p->isLeaf()) return false;
        if (containsPane(secondId)) return false;

        // create first child (clone current leaf)
        auto firstChild = std::make_shared<SplitPane>();
        firstChild->id        = paneId + "_first";
        firstChild->tabGroup  = p->tabGroup;

        // create second child (empty leaf)
        auto secondChild = std::make_shared<SplitPane>();
        secondChild->id       = secondId;
        secondChild->tabGroup.groupId = secondId;

        // transform p into branch
        p->orientation = orientation;
        p->splitRatio  = ratio;
        p->tabGroup    = TabGroup{}; // clear (branch has no direct tabs)
        p->first       = firstChild;
        p->second      = secondChild;

        notify();
        return true;
    }

    // Collapse a branch pane: keep only the 'first' child (simple close-second)
    bool collapsePane(const std::string& paneId) {
        SplitPane* p = findPane(paneId);
        if (!p || !p->isBranch()) return false;

        // restore leaf state from first child
        auto firstChild  = p->first;
        p->orientation   = SplitOrientation::None;
        p->tabGroup      = firstChild->tabGroup;
        p->first.reset();
        p->second.reset();

        if (activePaneId_ != paneId) activePaneId_ = paneId;
        notify();
        return true;
    }

    // ── Observers ────────────────────────────────────────────────

    bool addObserver(ObserverFn fn) {
        if (static_cast<int>(observers_.size()) >= kMaxObservers) return false;
        observers_.push_back(std::move(fn));
        return true;
    }

    void clearObservers() { observers_.clear(); }

private:
    static constexpr int kMaxObservers = 16;

    std::shared_ptr<SplitPane>  root_;
    std::string                 activePaneId_;
    std::vector<ObserverFn>     observers_;

    SplitPane* findIn(SplitPane* p, const std::string& id) const {
        if (!p) return nullptr;
        if (p->id == id) return p;
        if (p->first) {
            if (auto* r = findIn(p->first.get(), id)) return r;
        }
        if (p->second) {
            if (auto* r = findIn(p->second.get(), id)) return r;
        }
        return nullptr;
    }

    void notify() {
        for (auto& obs : observers_)
            if (obs) obs();
    }
};

} // namespace NF
