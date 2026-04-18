#pragma once
#include "UIScreenGraph.h"
#include "UIEventRouter.h"
#include <cstdint>
#include <functional>

namespace atlas::ui {

using SplitterMovedCallback = std::function<void(uint32_t widgetId, float position)>;

class SplitterManager {
public:
    void Init(UIScreen* screen);
    bool HandleEvent(const UIEvent& event);
    void SetSplitterMovedCallback(SplitterMovedCallback callback);
    uint32_t GetDraggingId() const;
    bool IsDragging() const;

private:
    UIScreen* m_screen = nullptr;
    uint32_t m_draggingId = 0;
    SplitterMovedCallback m_callback;
};

} // namespace atlas::ui
