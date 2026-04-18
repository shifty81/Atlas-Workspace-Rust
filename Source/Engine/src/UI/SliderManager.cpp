#include "SliderManager.h"
#include "UIConstants.h"
#include <algorithm>

namespace atlas::ui {

void SliderManager::Init(UIScreen* screen) {
    m_screen = screen;
    m_draggingId = 0;
}

bool SliderManager::HandleEvent(const UIEvent& event) {
    if (!m_screen) return false;

    if (event.type == UIEvent::Type::MouseDown && event.mouseButton == 0) {
        for (uint32_t i = 1; i < kMaxWidgetId; ++i) {
            const UIWidget* w = m_screen->GetWidget(i);
            if (!w || !w->visible) continue;
            if (w->type != UIWidgetType::Slider) continue;
            if (event.x >= w->x && event.x < w->x + w->width &&
                event.y >= w->y && event.y < w->y + w->height) {
                m_draggingId = i;
                float newValue = ComputeValueFromMouse(w, event.x);
                m_screen->SetValue(i, newValue);
                if (m_callback) {
                    m_callback(i, newValue);
                }
                return true;
            }
        }
    }

    if (event.type == UIEvent::Type::MouseMove && m_draggingId != 0) {
        const UIWidget* w = m_screen->GetWidget(m_draggingId);
        if (w) {
            float newValue = ComputeValueFromMouse(w, event.x);
            m_screen->SetValue(m_draggingId, newValue);
            if (m_callback) {
                m_callback(m_draggingId, newValue);
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

void SliderManager::SetSliderChangedCallback(SliderChangedCallback callback) {
    m_callback = std::move(callback);
}

uint32_t SliderManager::GetDraggingId() const {
    return m_draggingId;
}

bool SliderManager::IsDragging() const {
    return m_draggingId != 0;
}

float SliderManager::ComputeValueFromMouse(const UIWidget* widget, int32_t mouseX) const {
    if (!widget || widget->width <= 0.0f) return 0.0f;
    float relative = static_cast<float>(mouseX) - widget->x;
    float normalized = relative / widget->width;
    return std::clamp(normalized, 0.0f, 1.0f);
}

} // namespace atlas::ui
