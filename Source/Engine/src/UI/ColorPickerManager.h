#pragma once
#include "UIScreenGraph.h"
#include "UIEventRouter.h"
#include <cstdint>
#include <functional>

namespace atlas::ui {

using ColorChangedCallback = std::function<void(uint32_t widgetId, uint8_t r, uint8_t g, uint8_t b, uint8_t a)>;

class ColorPickerManager {
public:
    void Init(UIScreen* screen);
    bool HandleEvent(const UIEvent& event);
    void SetColorChangedCallback(ColorChangedCallback callback);

    void SetColor(uint32_t widgetId, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    uint32_t GetOpenPickerId() const;
    bool IsOpen() const;

private:
    UIScreen* m_screen = nullptr;
    uint32_t m_openPickerId = 0;
    ColorChangedCallback m_callback;
};

} // namespace atlas::ui
