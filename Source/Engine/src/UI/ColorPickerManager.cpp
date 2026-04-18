#include "ColorPickerManager.h"
#include "UIConstants.h"

namespace atlas::ui {

void ColorPickerManager::Init(UIScreen* screen) {
    m_screen = screen;
    m_openPickerId = 0;
}

bool ColorPickerManager::HandleEvent(const UIEvent& event) {
    if (!m_screen) return false;

    if (event.type != UIEvent::Type::MouseDown || event.mouseButton != 0) {
        return false;
    }

    // If a picker popup is open, check if click is inside the popup area
    if (m_openPickerId != 0) {
        const UIWidget* w = m_screen->GetWidget(m_openPickerId);
        if (w) {
            // Popup area: positioned below the swatch, 100x80 pixels
            int32_t popX = static_cast<int32_t>(w->x);
            int32_t popY = static_cast<int32_t>(w->y + w->height);
            int32_t popW = 100;
            int32_t popH = 80;

            if (event.x >= popX && event.x < popX + popW &&
                event.y >= popY && event.y < popY + popH) {
                // Map click position to a color
                float normX = static_cast<float>(event.x - popX) / static_cast<float>(popW);
                float normY = static_cast<float>(event.y - popY) / static_cast<float>(popH);
                uint8_t r = static_cast<uint8_t>(normX * 255.0f);
                uint8_t g = static_cast<uint8_t>((1.0f - normY) * 255.0f);
                uint8_t b = 128;
                uint8_t a = 255;
                m_screen->SetColor(m_openPickerId, r, g, b, a);
                if (m_callback) {
                    m_callback(m_openPickerId, r, g, b, a);
                }
                m_openPickerId = 0;
                return true;
            }
        }
        // Click outside — close the picker
        m_openPickerId = 0;
        return true;
    }

    // Check if click is on a ColorPicker swatch
    for (uint32_t i = 1; i < kMaxWidgetId; ++i) {
        const UIWidget* w = m_screen->GetWidget(i);
        if (!w || !w->visible) continue;
        if (w->type != UIWidgetType::ColorPicker) continue;
        if (event.x >= w->x && event.x < w->x + w->width &&
            event.y >= w->y && event.y < w->y + w->height) {
            m_openPickerId = i;
            return true;
        }
    }

    return false;
}

void ColorPickerManager::SetColorChangedCallback(ColorChangedCallback callback) {
    m_callback = std::move(callback);
}

void ColorPickerManager::SetColor(uint32_t widgetId, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (!m_screen) return;
    m_screen->SetColor(widgetId, r, g, b, a);
    if (m_callback) {
        m_callback(widgetId, r, g, b, a);
    }
}

uint32_t ColorPickerManager::GetOpenPickerId() const {
    return m_openPickerId;
}

bool ColorPickerManager::IsOpen() const {
    return m_openPickerId != 0;
}

} // namespace atlas::ui
