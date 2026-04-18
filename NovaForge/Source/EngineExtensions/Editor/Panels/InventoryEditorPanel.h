#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/ui/UIDrawList.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace atlas::editor {

struct InventoryItem {
    uint64_t id = 0;
    std::string name;
    std::string category; // "weapon", "armor", "consumable", "material", "quest"
    int32_t stackSize = 1;
    int32_t maxStack = 99;
    float weight = 0.0f;
    std::unordered_map<std::string, std::string> properties;
};

class InventoryEditorPanel : public EditorPanel {
public:
    const char* Name() const override { return "Inventory Editor"; }
    void Draw() override;

    uint64_t AddItem(const InventoryItem& item);
    bool RemoveItem(uint64_t id);
    const InventoryItem* GetItem(uint64_t id) const;
    std::vector<InventoryItem> ListItems() const;
    size_t ItemCount() const;

    void SelectItem(uint64_t id);
    uint64_t SelectedItemID() const;

    void SetFilterCategory(const std::string& category);
    std::string FilterCategory() const;
    std::vector<InventoryItem> FilteredItems() const;

    void SortByName();
    void SortByCategory();

    std::string ExportInventory() const;
    bool ImportInventory(const std::string& json);

    void Clear();

    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    std::vector<InventoryItem> m_items;
    uint64_t m_nextID = 1;
    uint64_t m_selectedID = 0;
    std::string m_filterCategory;
    atlas::ui::UIDrawList m_drawList;
};

} // namespace atlas::editor
