#include "ComboBoxManager.h"
#include "UIConstants.h"

namespace atlas::ui {

const std::vector<std::string> ComboBoxManager::s_emptyItems;

static constexpr int32_t kItemHeight = 20;

void ComboBoxManager::Init(UIScreen* screen) {
    m_screen = screen;
    m_items.clear();
    m_openComboId = 0;
}

bool ComboBoxManager::HandleEvent(const UIEvent& event) {
    if (!m_screen) return false;

    if (event.type != UIEvent::Type::MouseDown || event.mouseButton != 0) {
        return false;
    }

    // If a combo is open, check if the click is on a dropdown item
    if (m_openComboId != 0) {
        const UIWidget* w = m_screen->GetWidget(m_openComboId);
        if (w) {
            auto itemIt = m_items.find(m_openComboId);
            if (itemIt != m_items.end() && !itemIt->second.empty()) {
                int32_t dropX = static_cast<int32_t>(w->x);
                int32_t dropY = static_cast<int32_t>(w->y + w->height);
                int32_t dropW = static_cast<int32_t>(w->width);
                int32_t dropH = static_cast<int32_t>(itemIt->second.size()) * kItemHeight;

                if (event.x >= dropX && event.x < dropX + dropW &&
                    event.y >= dropY && event.y < dropY + dropH) {
                    int32_t index = (event.y - dropY) / kItemHeight;
                    if (index >= 0 && index < static_cast<int32_t>(itemIt->second.size())) {
                        m_screen->SetSelectedIndex(m_openComboId, index);
                        m_screen->SetComboOpen(m_openComboId, false);
                        uint32_t comboId = m_openComboId;
                        m_openComboId = 0;
                        if (m_callback) {
                            m_callback(comboId, index);
                        }
                        return true;
                    }
                }
            }
        }
        // Click outside — close the combo
        m_screen->SetComboOpen(m_openComboId, false);
        m_openComboId = 0;
        return true;
    }

    // Check if click is on a ComboBox widget
    for (uint32_t i = 1; i < kMaxWidgetId; ++i) {
        const UIWidget* w = m_screen->GetWidget(i);
        if (!w || !w->visible) continue;
        if (w->type != UIWidgetType::ComboBox) continue;
        if (event.x >= w->x && event.x < w->x + w->width &&
            event.y >= w->y && event.y < w->y + w->height) {
            m_screen->SetComboOpen(i, true);
            m_openComboId = i;
            return true;
        }
    }

    return false;
}

void ComboBoxManager::SetItems(uint32_t comboId, const std::vector<std::string>& items) {
    m_items[comboId] = items;
}

const std::vector<std::string>& ComboBoxManager::GetItems(uint32_t comboId) const {
    auto it = m_items.find(comboId);
    if (it != m_items.end()) {
        return it->second;
    }
    return s_emptyItems;
}

std::string ComboBoxManager::GetSelectedText(uint32_t comboId) const {
    if (!m_screen) return "";
    int32_t idx = m_screen->GetSelectedIndex(comboId);
    if (idx < 0) return "";
    auto it = m_items.find(comboId);
    if (it == m_items.end()) return "";
    if (idx >= static_cast<int32_t>(it->second.size())) return "";
    return it->second[static_cast<size_t>(idx)];
}

void ComboBoxManager::SetSelectionCallback(ComboSelectionCallback callback) {
    m_callback = std::move(callback);
}

uint32_t ComboBoxManager::GetOpenComboId() const {
    return m_openComboId;
}

} // namespace atlas::ui
