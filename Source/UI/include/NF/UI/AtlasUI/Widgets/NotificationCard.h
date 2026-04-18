#pragma once

#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include <chrono>
#include <string>

namespace NF::UI::AtlasUI {

enum class NotificationLevel { Info, Warning, Error };

class NotificationCard final : public WidgetBase {
public:
    NotificationCard(std::string message, NotificationLevel level = NotificationLevel::Info);

    void setMessage(const std::string& msg) { m_message = msg; }
    void setLevel(NotificationLevel level) { m_level = level; }
    [[nodiscard]] bool isExpired() const { return m_expired; }
    void dismiss() { m_expired = true; m_visible = false; }

    void paint(IPaintContext& context) override;

private:
    std::string m_message;
    NotificationLevel m_level = NotificationLevel::Info;
    bool m_expired = false;
};

} // namespace NF::UI::AtlasUI
