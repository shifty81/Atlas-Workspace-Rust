#pragma once
// NF::Editor — Dock layout system
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>
#include "NF/Editor/EditorTheme.h"

namespace NF {

class DockLayout {
public:
    void addPanel(const std::string& name, DockSlot slot) {
        DockPanel panel;
        panel.name = name;
        panel.slot = slot;
        panel.visible = true;
        m_panels.push_back(std::move(panel));
        NF_LOG_INFO("Editor", "DockLayout: added panel '" + name + "'");
    }

    void removePanel(const std::string& name) {
        auto it = std::find_if(m_panels.begin(), m_panels.end(),
            [&](const DockPanel& p) { return p.name == name; });
        if (it != m_panels.end()) {
            m_panels.erase(it);
            NF_LOG_INFO("Editor", "DockLayout: removed panel '" + name + "'");
        }
    }

    [[nodiscard]] DockPanel* findPanel(const std::string& name) {
        auto it = std::find_if(m_panels.begin(), m_panels.end(),
            [&](const DockPanel& p) { return p.name == name; });
        return (it != m_panels.end()) ? &(*it) : nullptr;
    }

    void setPanelVisible(const std::string& name, bool visible) {
        if (auto* p = findPanel(name)) {
            p->visible = visible;
        }
    }

    void computeLayout(float width, float height, float toolbarHeight, float statusBarHeight) {
        float usableHeight = height - toolbarHeight - statusBarHeight;

        float leftWidth = 0.f;
        float rightWidth = 0.f;
        float topHeight = 0.f;
        float bottomHeight = 0.f;

        for (auto& p : m_panels) {
            if (!p.visible) continue;
            switch (p.slot) {
                case DockSlot::Left:   leftWidth   = m_leftWidth;   break;
                case DockSlot::Right:  rightWidth  = m_rightWidth;  break;
                case DockSlot::Top:    topHeight   = m_topHeight;   break;
                case DockSlot::Bottom: bottomHeight = m_bottomHeight; break;
                default: break;
            }
        }

        float centerX = leftWidth;
        float centerW = width - leftWidth - rightWidth;

        for (auto& p : m_panels) {
            if (!p.visible) continue;
            switch (p.slot) {
                case DockSlot::Left:
                    p.bounds = { 0.f, toolbarHeight, leftWidth, usableHeight };
                    break;
                case DockSlot::Right:
                    p.bounds = { width - rightWidth, toolbarHeight, rightWidth, usableHeight };
                    break;
                case DockSlot::Bottom:
                    p.bounds = { centerX, height - statusBarHeight - bottomHeight, centerW, bottomHeight };
                    break;
                case DockSlot::Top:
                    p.bounds = { centerX, toolbarHeight, centerW, topHeight };
                    break;
                case DockSlot::Center:
                    p.bounds = { centerX, toolbarHeight + topHeight, centerW,
                                 usableHeight - topHeight - bottomHeight };
                    break;
            }
        }
    }

    [[nodiscard]] size_t panelCount() const { return m_panels.size(); }
    [[nodiscard]] const std::vector<DockPanel>& panels() const { return m_panels; }

    static constexpr int   kDockSlotCount  = 5;
    static constexpr float kTabBarHeight   = 22.f;
    static constexpr float kMinPanelSize   = 50.f;

    // ── Splitter resizing ────────────────────────────────────────
    void beginResize(DockSlot slot, float mousePos) {
        m_resizingSlot = slot;
        m_resizeStart = mousePos;
        m_resizing = true;
    }

    void updateResize(float mousePos) {
        if (!m_resizing) return;
        float delta = mousePos - m_resizeStart;
        m_resizeStart = mousePos;
        switch (m_resizingSlot) {
            case DockSlot::Left:   m_leftWidth   = std::clamp(m_leftWidth + delta, kMinPanelSize, 600.f); break;
            case DockSlot::Right:  m_rightWidth  = std::clamp(m_rightWidth - delta, kMinPanelSize, 600.f); break;
            case DockSlot::Bottom: m_bottomHeight = std::clamp(m_bottomHeight - delta, kMinPanelSize, 500.f); break;
            case DockSlot::Top:    m_topHeight   = std::clamp(m_topHeight + delta, kMinPanelSize, 500.f); break;
            default: break;
        }
    }

    void endResize() { m_resizing = false; }
    [[nodiscard]] bool isResizing() const { return m_resizing; }
    [[nodiscard]] DockSlot resizingSlot() const { return m_resizingSlot; }

    // Reset to defaults
    void resetSizes() {
        m_leftWidth   = kDefaultLeftWidth;
        m_rightWidth  = kDefaultRightWidth;
        m_topHeight   = kDefaultTopHeight;
        m_bottomHeight = kDefaultBottomHeight;
    }

    // ── Tab groups ───────────────────────────────────────────────
    void addTab(const std::string& panelName, DockSlot slot) {
        m_tabGroups[static_cast<int>(slot)].push_back(panelName);
    }

    [[nodiscard]] std::string activeTab(DockSlot slot) const {
        int idx = static_cast<int>(slot);
        auto it = m_activeTabIndex.find(idx);
        int tabIdx = (it != m_activeTabIndex.end()) ? it->second : 0;
        if (idx < kDockSlotCount && tabIdx < static_cast<int>(m_tabGroups[idx].size())) {
            return m_tabGroups[idx][static_cast<size_t>(tabIdx)];
        }
        return {};
    }

    void selectTab(const std::string& panelName) {
        for (int s = 0; s < kDockSlotCount; ++s) {
            for (size_t i = 0; i < m_tabGroups[s].size(); ++i) {
                if (m_tabGroups[s][i] == panelName) {
                    m_activeTabIndex[s] = static_cast<int>(i);
                    return;
                }
            }
        }
    }

    [[nodiscard]] const std::vector<std::string>& tabGroup(DockSlot slot) const {
        return m_tabGroups[static_cast<int>(slot)];
    }

    /// Returns true if the panel is not in any tab group, or is the currently active tab.
    [[nodiscard]] bool isPanelActive(const std::string& name) const {
        for (int s = 0; s < kDockSlotCount; ++s) {
            for (size_t i = 0; i < m_tabGroups[s].size(); ++i) {
                if (m_tabGroups[s][i] == name) {
                    auto it = m_activeTabIndex.find(s);
                    int activeIdx = (it != m_activeTabIndex.end()) ? it->second : 0;
                    return static_cast<int>(i) == activeIdx;
                }
            }
        }
        return true; // not in any tab group → always active
    }

    // Accessors for current sizes
    [[nodiscard]] float leftWidth()    const { return m_leftWidth; }
    [[nodiscard]] float rightWidth()   const { return m_rightWidth; }
    [[nodiscard]] float topHeight()    const { return m_topHeight; }
    [[nodiscard]] float bottomHeight() const { return m_bottomHeight; }

    // Setters for restoring persisted sizes
    void setLeftWidth(float w)    { m_leftWidth    = std::clamp(w, kMinPanelSize, 600.f); }
    void setRightWidth(float w)   { m_rightWidth   = std::clamp(w, kMinPanelSize, 600.f); }
    void setTopHeight(float h)    { m_topHeight    = std::clamp(h, kMinPanelSize, 500.f); }
    void setBottomHeight(float h) { m_bottomHeight = std::clamp(h, kMinPanelSize, 500.f); }

    static constexpr float kTabCharWidth = 8.f;   ///< pixels per character in tab labels
    static constexpr float kTabPadding   = 16.f;  ///< total horizontal padding per tab

    /// Width of a tab label button for the given panel name.
    [[nodiscard]] static float tabLabelWidth(const std::string& name) {
        return static_cast<float>(name.size()) * kTabCharWidth + kTabPadding;
    }

    /// Returns panel bounds adjusted to sit below the tab bar when the panel is in a tab group.
    /// If the panel is not in a tab group the bounds are returned unchanged.
    [[nodiscard]] Rect adjustedBoundsForPanel(const DockPanel& dp) const {
        if (tabGroup(dp.slot).empty()) return dp.bounds;
        Rect b = dp.bounds;
        b.y += kTabBarHeight;
        b.h  = std::max(0.f, b.h - kTabBarHeight);
        return b;
    }
    static constexpr float kDefaultLeftWidth   = 250.f;
    static constexpr float kDefaultRightWidth  = 300.f;
    static constexpr float kDefaultTopHeight   = 200.f;
    static constexpr float kDefaultBottomHeight = 200.f;

private:
    std::vector<DockPanel> m_panels;
    float m_leftWidth    = kDefaultLeftWidth;
    float m_rightWidth   = kDefaultRightWidth;
    float m_topHeight    = kDefaultTopHeight;
    float m_bottomHeight = kDefaultBottomHeight;
    bool  m_resizing     = false;
    DockSlot m_resizingSlot = DockSlot::Center;
    float m_resizeStart  = 0.f;
    std::vector<std::string> m_tabGroups[kDockSlotCount];  // indexed by DockSlot
    std::unordered_map<int, int> m_activeTabIndex;
};

// ── EditorPanel (abstract) ───────────────────────────────────────
// DEPRECATED: Legacy panel base class. New panels should derive from
// NF::UI::AtlasUI::PanelBase instead. See Source/UI/include/NF/UI/AtlasUI/Panels/
// for the canonical AtlasUI panel implementations.
// Migration phases U1-U7 provide AtlasUI equivalents for each panel below.


} // namespace NF
