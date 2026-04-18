#include "AssetBrowserPanel.h"
#include <algorithm>
#include <sstream>
#include <filesystem>

namespace atlas::editor {

void AssetBrowserPanel::Draw() {
    Refresh();

    m_drawList.Clear();

    // Background
    m_drawList.DrawRect({0, 0, 600, 400}, {30, 30, 30, 255});

    // Title bar
    m_drawList.DrawRect({0, 0, 600, 24}, {50, 50, 50, 255});
    m_drawList.DrawText({4, 4, 300, 20}, "Asset Browser", {220, 220, 220, 255});

    // Summary line (count + filter + sort mode)
    int32_t y = 28;
    const char* sortLabel = "Name";
    switch (m_sortMode) {
        case AssetSortMode::ByType: sortLabel = "Type"; break;
        case AssetSortMode::ByPath: sortLabel = "Path"; break;
        default: break;
    }
    std::string info = "Assets: " + std::to_string(m_entries.size()) + "  Sort: " + sortLabel;
    if (!m_filter.empty()) {
        info += "  Filter: " + m_filter;
    }
    m_drawList.DrawText({4, y, 590, 16}, info, {160, 180, 200, 255});
    y += 20;

    if (m_entries.empty()) {
        m_drawList.DrawText({4, y, 590, 16}, "No assets found", {160, 160, 160, 255});
        return;
    }

    // Asset list
    for (const auto& entry : m_entries) {
        bool selected = (entry.id == m_selectedAsset);
        atlas::ui::UIColor bgColor = selected
            ? atlas::ui::UIColor{60, 80, 120, 255}
            : atlas::ui::UIColor{40, 40, 40, 255};
        m_drawList.DrawRect({0, y, 600, 20}, bgColor);

        std::string label = entry.id + "  " + entry.extension;
        m_drawList.DrawText({4, y + 2, 590, 16}, label, {200, 200, 200, 255});
        y += 22;
    }
}

void AssetBrowserPanel::Refresh() {
    m_entries.clear();

    auto allAssets = m_registry.GetAll();
    for (const auto& asset : allAssets) {
        AssetBrowserEntry entry;
        entry.id = asset.id;
        entry.path = asset.path;
        entry.version = asset.version;

        std::filesystem::path p(asset.path);
        entry.extension = p.extension().string();

        // Apply filter if set
        if (!m_filter.empty()) {
            bool matchesName = asset.id.find(m_filter) != std::string::npos;
            bool matchesExt = entry.extension.find(m_filter) != std::string::npos;
            if (!matchesName && !matchesExt) continue;
        }

        m_entries.push_back(std::move(entry));
    }

    // Sort based on current sort mode
    switch (m_sortMode) {
        case AssetSortMode::ByName:
            std::sort(m_entries.begin(), m_entries.end(),
                [](const AssetBrowserEntry& a, const AssetBrowserEntry& b) {
                    return a.id < b.id;
                });
            break;
        case AssetSortMode::ByType:
            std::sort(m_entries.begin(), m_entries.end(),
                [](const AssetBrowserEntry& a, const AssetBrowserEntry& b) {
                    return a.extension < b.extension;
                });
            break;
        case AssetSortMode::ByPath:
            std::sort(m_entries.begin(), m_entries.end(),
                [](const AssetBrowserEntry& a, const AssetBrowserEntry& b) {
                    return a.path < b.path;
                });
            break;
    }
}

std::string AssetBrowserPanel::Summary() const {
    std::ostringstream oss;
    oss << "Assets: " << m_entries.size();
    if (!m_filter.empty()) {
        oss << " (filter: " << m_filter << ")";
    }
    return oss.str();
}

}
