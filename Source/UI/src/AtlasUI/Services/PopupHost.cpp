#include "NF/UI/AtlasUI/Services/PopupHost.h"

namespace NF::UI::AtlasUI {

PopupHost& PopupHost::Get() {
    static PopupHost instance;
    return instance;
}

void PopupHost::openPopup(const NF::Rect& anchor, std::shared_ptr<IWidget> content) {
    m_anchor = anchor;
    m_content = std::move(content);
    m_open = true;
}

void PopupHost::closePopup() {
    m_content.reset();
    m_open = false;
}

void PopupHost::paint(IPaintContext& context) {
    if (!m_open || !m_content) return;
    m_content->paint(context);
}

bool PopupHost::handleInput(IInputContext& context) {
    if (!m_open || !m_content) return false;
    return m_content->handleInput(context);
}

} // namespace NF::UI::AtlasUI
