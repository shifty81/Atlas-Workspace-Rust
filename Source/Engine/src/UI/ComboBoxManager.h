#pragma once
#include "UIScreenGraph.h"
#include "UIEventRouter.h"
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace atlas::ui {

using ComboSelectionCallback = std::function<void(uint32_t comboId, int32_t selectedIndex)>;

class ComboBoxManager {
public:
    void Init(UIScreen* screen);
    bool HandleEvent(const UIEvent& event);

    void SetItems(uint32_t comboId, const std::vector<std::string>& items);
    const std::vector<std::string>& GetItems(uint32_t comboId) const;
    std::string GetSelectedText(uint32_t comboId) const;

    void SetSelectionCallback(ComboSelectionCallback callback);
    uint32_t GetOpenComboId() const;

private:
    UIScreen* m_screen = nullptr;
    std::unordered_map<uint32_t, std::vector<std::string>> m_items;
    uint32_t m_openComboId = 0;
    ComboSelectionCallback m_callback;
    static const std::vector<std::string> s_emptyItems;
};

} // namespace atlas::ui
