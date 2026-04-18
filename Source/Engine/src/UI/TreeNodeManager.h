#pragma once
#include "UIScreenGraph.h"
#include "UIEventRouter.h"
#include <cstdint>
#include <functional>

namespace atlas::ui {

using TreeNodeToggledCallback = std::function<void(uint32_t widgetId, bool expanded)>;

class TreeNodeManager {
public:
    void Init(UIScreen* screen);
    bool HandleEvent(const UIEvent& event);
    void SetTreeNodeToggledCallback(TreeNodeToggledCallback callback);

private:
    UIScreen* m_screen = nullptr;
    TreeNodeToggledCallback m_callback;
};

} // namespace atlas::ui
