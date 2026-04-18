#pragma once
#include "UIScreenGraph.h"
#include "UIEventRouter.h"
#include <cstdint>
#include <functional>

namespace atlas::ui {

using SliderChangedCallback = std::function<void(uint32_t widgetId, float value)>;

class SliderManager {
public:
    void Init(UIScreen* screen);
    bool HandleEvent(const UIEvent& event);
    void SetSliderChangedCallback(SliderChangedCallback callback);
    uint32_t GetDraggingId() const;
    bool IsDragging() const;

private:
    UIScreen* m_screen = nullptr;
    uint32_t m_draggingId = 0;
    SliderChangedCallback m_callback;

    float ComputeValueFromMouse(const UIWidget* widget, int32_t mouseX) const;
};

} // namespace atlas::ui
