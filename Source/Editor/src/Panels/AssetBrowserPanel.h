#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/assets/AssetRegistry.h"
#include "../../engine/ui/UIDrawList.h"
#include <string>
#include <vector>

namespace atlas::editor {

struct AssetBrowserEntry {
    std::string id;
    std::string path;
    std::string extension;
    uint64_t version = 1;
};

enum class AssetSortMode {
    ByName,
    ByType,
    ByPath
};

class AssetBrowserPanel : public EditorPanel {
public:
    explicit AssetBrowserPanel(asset::AssetRegistry& registry)
        : m_registry(registry) {}

    const char* Name() const override { return "Asset Browser"; }
    void Draw() override;

    void Refresh();

    void SetFilter(const std::string& filter) { m_filter = filter; }
    const std::string& Filter() const { return m_filter; }

    void SetSortMode(AssetSortMode mode) { m_sortMode = mode; }
    AssetSortMode SortMode() const { return m_sortMode; }

    void SelectAsset(const std::string& id) { m_selectedAsset = id; }
    const std::string& SelectedAsset() const { return m_selectedAsset; }

    const std::vector<AssetBrowserEntry>& Entries() const { return m_entries; }

    std::string Summary() const;

    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    asset::AssetRegistry& m_registry;
    std::vector<AssetBrowserEntry> m_entries;
    std::string m_filter;
    std::string m_selectedAsset;
    AssetSortMode m_sortMode = AssetSortMode::ByName;
    atlas::ui::UIDrawList m_drawList;
};

}
