#pragma once
// NF::Editor — Editor toolbar
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

struct ToolbarItem {
    std::string name;
    std::string icon;
    std::string tooltip;
    std::function<void()> action;
    bool enabled = true;
    bool isSeparator = false;
};

class EditorToolbar {
public:
    void addItem(const std::string& name, const std::string& icon,
                 const std::string& tooltip, std::function<void()> action, bool enabled = true) {
        ToolbarItem item;
        item.name = name;
        item.icon = icon;
        item.tooltip = tooltip;
        item.action = std::move(action);
        item.enabled = enabled;
        item.isSeparator = false;
        m_items.push_back(std::move(item));
    }

    void addSeparator() {
        ToolbarItem sep;
        sep.isSeparator = true;
        m_items.push_back(std::move(sep));
    }

    [[nodiscard]] const std::vector<ToolbarItem>& items() const { return m_items; }
    [[nodiscard]] size_t itemCount() const { return m_items.size(); }

private:
    std::vector<ToolbarItem> m_items;
};

// ── Project Indexer ──────────────────────────────────────────────


} // namespace NF
