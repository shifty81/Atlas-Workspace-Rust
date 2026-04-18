#pragma once

#include "NF/UI/AtlasUI/Interfaces.h"
#include <string>

namespace NF::UI::AtlasUI {

class FocusService {
public:
    static FocusService& Get();

    void setFocus(IWidget* widget);
    void clearFocus();
    [[nodiscard]] IWidget* focusedWidget() const { return m_focused; }
    [[nodiscard]] bool hasFocus(const IWidget* widget) const { return m_focused == widget; }

    void pushScope(const std::string& scopeId);
    void popScope();
    [[nodiscard]] const std::string& currentScope() const { return m_currentScope; }

private:
    FocusService() = default;
    IWidget* m_focused = nullptr;
    std::string m_currentScope;
};

} // namespace NF::UI::AtlasUI
