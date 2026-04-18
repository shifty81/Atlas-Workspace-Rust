#include "NF/UI/AtlasUI/Services/FocusService.h"

namespace NF::UI::AtlasUI {

FocusService& FocusService::Get() {
    static FocusService instance;
    return instance;
}

void FocusService::setFocus(IWidget* widget) {
    m_focused = widget;
}

void FocusService::clearFocus() {
    m_focused = nullptr;
}

void FocusService::pushScope(const std::string& scopeId) {
    m_currentScope = scopeId;
}

void FocusService::popScope() {
    m_currentScope.clear();
}

} // namespace NF::UI::AtlasUI
