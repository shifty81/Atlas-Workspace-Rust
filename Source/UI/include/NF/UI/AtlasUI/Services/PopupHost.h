#pragma once

#include "NF/UI/AtlasUI/Interfaces.h"
#include <memory>

namespace NF::UI::AtlasUI {

class PopupHost : public IPopupHost {
public:
    static PopupHost& Get();

    void openPopup(const NF::Rect& anchor, std::shared_ptr<IWidget> content) override;
    void closePopup() override;
    [[nodiscard]] bool isPopupOpen() const override { return m_open; }

    void paint(IPaintContext& context);
    bool handleInput(IInputContext& context);

    [[nodiscard]] const NF::Rect& anchor() const { return m_anchor; }

private:
    PopupHost() = default;
    NF::Rect m_anchor{};
    std::shared_ptr<IWidget> m_content;
    bool m_open = false;
};

} // namespace NF::UI::AtlasUI
