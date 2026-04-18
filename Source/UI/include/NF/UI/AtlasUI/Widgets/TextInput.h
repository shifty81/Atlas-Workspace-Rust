#pragma once

#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include <functional>
#include <string>

namespace NF::UI::AtlasUI {

class TextInput final : public WidgetBase {
public:
    using ChangeHandler = std::function<void(const std::string&)>;

    explicit TextInput(std::string placeholder = {});

    void setText(const std::string& text);
    [[nodiscard]] const std::string& text() const { return m_text; }
    void setPlaceholder(const std::string& placeholder) { m_placeholder = placeholder; }
    void setOnChange(ChangeHandler handler) { m_onChange = std::move(handler); }

    void measure(ILayoutContext& context) override;
    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

private:
    std::string m_text;
    std::string m_placeholder;
    ChangeHandler m_onChange;
    bool m_focused = false;
    size_t m_cursor = 0;
};

} // namespace NF::UI::AtlasUI
