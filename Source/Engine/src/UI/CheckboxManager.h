#pragma once
#include "UIScreenGraph.h"
#include "UIEventRouter.h"
#include <cstdint>
#include <functional>

namespace atlas::ui {

using CheckboxChangedCallback = std::function<void(uint32_t widgetId, bool checked)>;

class CheckboxManager {
public:
    void Init(UIScreen* screen);
    bool HandleEvent(const UIEvent& event);
    void SetCheckboxChangedCallback(CheckboxChangedCallback callback);

private:
    UIScreen* m_screen = nullptr;
    CheckboxChangedCallback m_callback;
};

} // namespace atlas::ui
