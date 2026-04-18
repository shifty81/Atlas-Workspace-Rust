#pragma once
// NF::Editor — Workspace layout + manager
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

namespace NF {

enum class LayoutPanelType : uint8_t {
    Viewport    = 0,
    Inspector   = 1,
    Hierarchy   = 2,
    ContentBrowser = 3,
    Console     = 4,
    Profiler    = 5,
    Timeline    = 6,
    Custom      = 7,
};

inline const char* layoutPanelTypeName(LayoutPanelType t) {
    switch (t) {
        case LayoutPanelType::Viewport:       return "Viewport";
        case LayoutPanelType::Inspector:      return "Inspector";
        case LayoutPanelType::Hierarchy:      return "Hierarchy";
        case LayoutPanelType::ContentBrowser: return "ContentBrowser";
        case LayoutPanelType::Console:        return "Console";
        case LayoutPanelType::Profiler:       return "Profiler";
        case LayoutPanelType::Timeline:       return "Timeline";
        case LayoutPanelType::Custom:         return "Custom";
        default:                              return "Unknown";
    }
}

enum class LayoutDockZone : uint8_t {
    Left   = 0,
    Right  = 1,
    Top    = 2,
    Bottom = 3,
};

inline const char* layoutDockZoneName(LayoutDockZone z) {
    switch (z) {
        case LayoutDockZone::Left:   return "Left";
        case LayoutDockZone::Right:  return "Right";
        case LayoutDockZone::Top:    return "Top";
        case LayoutDockZone::Bottom: return "Bottom";
        default:                     return "Unknown";
    }
}

struct LayoutPanel {
    std::string     id;
    std::string     title;
    LayoutPanelType type     = LayoutPanelType::Custom;
    LayoutDockZone  dockZone = LayoutDockZone::Left;
    float           width    = 0.f;
    float           height   = 0.f;
    bool            visible  = true;
    bool            pinned   = false;

    void show()    { visible = true;  }
    void hide()    { visible = false; }
    void pin()     { pinned = true;   }
    void unpin()   { pinned = false;  }

    [[nodiscard]] bool isVisible() const { return visible; }
    [[nodiscard]] bool isPinned()  const { return pinned;  }
    [[nodiscard]] bool hasSize()   const { return width > 0.f && height > 0.f; }
};

struct LayoutSplit {
    std::string firstPanelId;
    std::string secondPanelId;
    bool        isHorizontal = true;
    float       ratio        = 0.5f;

    [[nodiscard]] bool isValid() const {
        return !firstPanelId.empty() && !secondPanelId.empty() && ratio > 0.f && ratio < 1.f;
    }
    void flipOrientation() { isHorizontal = !isHorizontal; }
};

class WorkspaceLayout {
public:
    explicit WorkspaceLayout(std::string name) : m_name(std::move(name)) {}

    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] size_t             panelCount()  const { return m_panels.size();  }
    [[nodiscard]] size_t             splitCount()  const { return m_splits.size();  }

    bool addPanel(const LayoutPanel& p) {
        for (auto& existing : m_panels) if (existing.id == p.id) return false;
        m_panels.push_back(p);
        return true;
    }

    bool removePanel(const std::string& id) {
        for (auto it = m_panels.begin(); it != m_panels.end(); ++it) {
            if (it->id == id) { m_panels.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] LayoutPanel* findPanel(const std::string& id) {
        for (auto& p : m_panels) if (p.id == id) return &p;
        return nullptr;
    }

    bool addSplit(const LayoutSplit& s) {
        if (!s.isValid()) return false;
        m_splits.push_back(s);
        return true;
    }

    [[nodiscard]] size_t visiblePanelCount() const {
        size_t c = 0;
        for (auto& p : m_panels) if (p.isVisible()) c++;
        return c;
    }

    [[nodiscard]] size_t pinnedPanelCount() const {
        size_t c = 0;
        for (auto& p : m_panels) if (p.isPinned()) c++;
        return c;
    }

    void showAll() { for (auto& p : m_panels) p.show(); }
    void hideAll() { for (auto& p : m_panels) p.hide(); }

private:
    std::string              m_name;
    std::vector<LayoutPanel> m_panels;
    std::vector<LayoutSplit> m_splits;
};

class WorkspaceLayoutManager {
public:
    static constexpr size_t MAX_LAYOUTS = 32;

    WorkspaceLayout* createLayout(const std::string& name) {
        if (m_layouts.size() >= MAX_LAYOUTS) return nullptr;
        for (auto& l : m_layouts) if (l.name() == name) return nullptr;
        m_layouts.emplace_back(name);
        return &m_layouts.back();
    }

    bool removeLayout(const std::string& name) {
        for (auto it = m_layouts.begin(); it != m_layouts.end(); ++it) {
            if (it->name() == name) {
                if (m_activeLayout == name) m_activeLayout.clear();
                m_layouts.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] WorkspaceLayout* findLayout(const std::string& name) {
        for (auto& l : m_layouts) if (l.name() == name) return &l;
        return nullptr;
    }

    bool setActive(const std::string& name) {
        if (!findLayout(name)) return false;
        m_activeLayout = name;
        return true;
    }

    [[nodiscard]] WorkspaceLayout* activeLayout() {
        return findLayout(m_activeLayout);
    }

    [[nodiscard]] const std::string& activeName()  const { return m_activeLayout; }
    [[nodiscard]] size_t             layoutCount() const { return m_layouts.size(); }
    [[nodiscard]] bool               hasActive()   const { return !m_activeLayout.empty(); }

private:
    std::vector<WorkspaceLayout> m_layouts;
    std::string                  m_activeLayout;
};

// ============================================================
// S23 — Shortcut Manager
// ============================================================


} // namespace NF
