#include "SplitterManager.h"
#include "UIConstants.h"

namespace atlas::ui {

void SplitterManager::Init(UIScreen* screen) {
    m_screen = screen;
    m_draggingId = 0;
}

bool SplitterManager::HandleEvent(const UIEvent& event) {
    if (!m_screen) return false;

    if (event.type == UIEvent::Type::MouseDown && event.mouseButton == 0) {
        for (uint32_t i = 1; i < kMaxWidgetId; ++i) {
            const UIWidget* w = m_screen->GetWidget(i);
            if (!w || !w->visible) continue;
            if (w->type != UIWidgetType::Splitter) continue;
            // Use a slightly wider hit area for easier grabbing
            float hitX = w->x - 2.0f;
            float hitW = w->width + 4.0f;
            if (event.x >= hitX && event.x < hitX + hitW &&
                event.y >= w->y && event.y < w->y + w->height) {
                m_draggingId = i;
                return true;
            }
        }
    }

    if (event.type == UIEvent::Type::MouseMove && m_draggingId != 0) {
        UIWidget* w = m_screen->GetWidgetMutable(m_draggingId);
        if (w) {
            // Determine if this is a vertical or horizontal splitter
            // Vertical splitter: width < height (divides left/right)
            // Horizontal splitter: width >= height (divides top/bottom)
            if (w->width < w->height) {
                w->x = static_cast<float>(event.x);
            } else {
                w->y = static_cast<float>(event.y);
            }
            if (m_callback) {
                float position = (w->width < w->height)
                    ? static_cast<float>(event.x)
                    : static_cast<float>(event.y);
                m_callback(m_draggingId, position);
            }
        }
        return true;
    }

    if (event.type == UIEvent::Type::MouseUp && m_draggingId != 0) {
        m_draggingId = 0;
        return true;
    }

    return false;
}

void SplitterManager::SetSplitterMovedCallback(SplitterMovedCallback callback) {
    m_callback = std::move(callback);
}

uint32_t SplitterManager::GetDraggingId() const {
    return m_draggingId;
}

bool SplitterManager::IsDragging() const {
    return m_draggingId != 0;
}

} // namespace atlas::ui
