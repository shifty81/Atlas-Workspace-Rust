#include "InventoryEditorPanel.h"
#include <sstream>
#include <algorithm>

namespace atlas::editor {

void InventoryEditorPanel::Draw() {
    m_drawList.Clear();

    // Background
    m_drawList.DrawRect({0, 0, 600, 400}, {30, 30, 30, 255});

    // Title bar with filter info
    m_drawList.DrawRect({0, 0, 600, 24}, {50, 50, 50, 255});
    std::string title = "Inventory Editor";
    if (!m_filterCategory.empty()) title += " [" + m_filterCategory + "]";
    m_drawList.DrawText({4, 4, 300, 20}, title, {220, 220, 220, 255});

    // Item list (apply filter)
    auto items = FilteredItems();
    int32_t y = 28;
    for (const auto& item : items) {
        bool selected = (item.id == m_selectedID);
        atlas::ui::UIColor bgColor = selected
            ? atlas::ui::UIColor{60, 80, 120, 255}
            : atlas::ui::UIColor{40, 40, 40, 255};
        m_drawList.DrawRect({0, y, 600, 20}, bgColor);

        std::ostringstream line;
        line << item.name << " (" << item.category << ") "
             << item.stackSize << "/" << item.maxStack;
        m_drawList.DrawText({4, y + 2, 590, 16}, line.str(), {200, 200, 200, 255});
        y += 22;
    }
}

uint64_t InventoryEditorPanel::AddItem(const InventoryItem& item) {
    InventoryItem i = item;
    i.id = m_nextID++;
    m_items.push_back(i);
    return i.id;
}

bool InventoryEditorPanel::RemoveItem(uint64_t id) {
    auto it = std::find_if(m_items.begin(), m_items.end(),
                           [id](const InventoryItem& i) { return i.id == id; });
    if (it != m_items.end()) {
        if (m_selectedID == id) m_selectedID = 0;
        m_items.erase(it);
        return true;
    }
    return false;
}

const InventoryItem* InventoryEditorPanel::GetItem(uint64_t id) const {
    for (auto& i : m_items) {
        if (i.id == id) return &i;
    }
    return nullptr;
}

std::vector<InventoryItem> InventoryEditorPanel::ListItems() const {
    return m_items;
}

size_t InventoryEditorPanel::ItemCount() const {
    return m_items.size();
}

void InventoryEditorPanel::SelectItem(uint64_t id) {
    m_selectedID = id;
}

uint64_t InventoryEditorPanel::SelectedItemID() const {
    return m_selectedID;
}

void InventoryEditorPanel::SetFilterCategory(const std::string& category) {
    m_filterCategory = category;
}

std::string InventoryEditorPanel::FilterCategory() const {
    return m_filterCategory;
}

std::vector<InventoryItem> InventoryEditorPanel::FilteredItems() const {
    if (m_filterCategory.empty()) return m_items;
    std::vector<InventoryItem> result;
    for (auto& i : m_items) {
        if (i.category == m_filterCategory) result.push_back(i);
    }
    return result;
}

void InventoryEditorPanel::SortByName() {
    std::sort(m_items.begin(), m_items.end(),
              [](const InventoryItem& a, const InventoryItem& b) {
                  return a.name < b.name;
              });
}

void InventoryEditorPanel::SortByCategory() {
    std::sort(m_items.begin(), m_items.end(),
              [](const InventoryItem& a, const InventoryItem& b) {
                  return a.category < b.category;
              });
}

std::string InventoryEditorPanel::ExportInventory() const {
    std::ostringstream ss;
    ss << "{\"items\":[";
    bool first = true;
    for (auto& i : m_items) {
        if (!first) ss << ",";
        first = false;
        ss << "{\"id\":" << i.id
           << ",\"name\":\"" << i.name << "\""
           << ",\"category\":\"" << i.category << "\""
           << ",\"stackSize\":" << i.stackSize
           << ",\"maxStack\":" << i.maxStack
           << ",\"weight\":" << i.weight << "}";
    }
    ss << "]}";
    return ss.str();
}

bool InventoryEditorPanel::ImportInventory(const std::string& json) {
    if (json.empty()) return false;
    if (json.find("\"items\"") == std::string::npos) return false;
    return true;
}

void InventoryEditorPanel::Clear() {
    m_items.clear();
    m_nextID = 1;
    m_selectedID = 0;
    m_filterCategory.clear();
}

} // namespace atlas::editor
