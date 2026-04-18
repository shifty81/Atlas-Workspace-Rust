#include "TooltipManager.h"
#include "UIConstants.h"

namespace atlas::ui {

void TooltipManager::Init(UIScreen* screen) {
    m_screen = screen;
    m_tooltips.clear();
    m_hoveredOwner = 0;
    m_hoverTime = 0.0f;
    m_activeTooltip = 0;
}

void TooltipManager::SetTooltip(uint32_t ownerWidgetId, uint32_t tooltipWidgetId) {
    m_tooltips[ownerWidgetId] = tooltipWidgetId;
    // Ensure tooltip starts hidden
    if (m_screen) {
        m_screen->SetVisible(tooltipWidgetId, false);
    }
}

void TooltipManager::RemoveTooltip(uint32_t ownerWidgetId) {
    auto it = m_tooltips.find(ownerWidgetId);
    if (it != m_tooltips.end()) {
        if (m_screen) {
            m_screen->SetVisible(it->second, false);
        }
        if (m_activeTooltip == it->second) {
            m_activeTooltip = 0;
        }
        m_tooltips.erase(it);
    }
}

uint32_t TooltipManager::GetTooltip(uint32_t ownerWidgetId) const {
    auto it = m_tooltips.find(ownerWidgetId);
    return (it != m_tooltips.end()) ? it->second : 0;
}

void TooltipManager::SetDelay(float seconds) {
    m_delay = seconds;
}

float TooltipManager::GetDelay() const {
    return m_delay;
}

bool TooltipManager::IsPointInWidget(const UIWidget* widget, int32_t x, int32_t y) const {
    if (!widget) return false;
    return x >= widget->x && x < widget->x + widget->width &&
           y >= widget->y && y < widget->y + widget->height;
}

void TooltipManager::Update(int32_t mouseX, int32_t mouseY, float deltaTime) {
    if (!m_screen) return;

    // Find which owner widget the mouse is over
    uint32_t newHoveredOwner = 0;
    for (const auto& [ownerId, tipId] : m_tooltips) {
        const UIWidget* owner = m_screen->GetWidget(ownerId);
        if (!owner || !owner->visible) continue;
        if (IsPointInWidget(owner, mouseX, mouseY)) {
            newHoveredOwner = ownerId;
            break;
        }
    }

    if (newHoveredOwner != m_hoveredOwner) {
        // Mouse moved to a different owner or left all owners
        // Hide the previous tooltip
        if (m_activeTooltip != 0) {
            m_screen->SetVisible(m_activeTooltip, false);
            m_activeTooltip = 0;
        }
        m_hoveredOwner = newHoveredOwner;
        m_hoverTime = 0.0f;
    }

    if (m_hoveredOwner == 0) return;

    m_hoverTime += deltaTime;

    if (m_hoverTime >= m_delay && m_activeTooltip == 0) {
        // Show the tooltip
        auto it = m_tooltips.find(m_hoveredOwner);
        if (it != m_tooltips.end()) {
            m_activeTooltip = it->second;
            m_screen->SetVisible(m_activeTooltip, true);

            // Position tooltip near the owner widget
            const UIWidget* owner = m_screen->GetWidget(m_hoveredOwner);
            UIWidget* tip = m_screen->GetWidgetMutable(m_activeTooltip);
            if (owner && tip) {
                tip->x = owner->x;
                tip->y = owner->y + owner->height + 2.0f;
            }
        }
    }
}

void TooltipManager::HideAll() {
    if (!m_screen) return;
    for (const auto& [ownerId, tipId] : m_tooltips) {
        m_screen->SetVisible(tipId, false);
    }
    m_activeTooltip = 0;
    m_hoveredOwner = 0;
    m_hoverTime = 0.0f;
}

uint32_t TooltipManager::GetActiveTooltip() const {
    return m_activeTooltip;
}

} // namespace atlas::ui
