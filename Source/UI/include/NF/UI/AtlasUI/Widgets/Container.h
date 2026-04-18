#pragma once

#include "NF/UI/AtlasUI/WidgetBase.h"
#include <memory>
#include <vector>

namespace NF::UI::AtlasUI {

class Container : public WidgetBase {
public:
    void addChild(std::shared_ptr<IWidget> child) {
        if (child) {
            m_children.push_back(std::move(child));
        }
    }

    [[nodiscard]] const std::vector<std::shared_ptr<IWidget>>& children() const { return m_children; }
    [[nodiscard]] std::vector<std::shared_ptr<IWidget>>& children() { return m_children; }

    bool handleInput(IInputContext& context) override {
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
            if (*it && (*it)->isVisible() && (*it)->handleInput(context)) {
                return true;
            }
        }
        return false;
    }

protected:
    std::vector<std::shared_ptr<IWidget>> m_children;
};

} // namespace NF::UI::AtlasUI
