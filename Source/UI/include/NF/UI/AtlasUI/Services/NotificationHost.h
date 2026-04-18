#pragma once

#include "NF/UI/AtlasUI/Widgets/NotificationCard.h"
#include <memory>
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

class NotificationHost {
public:
    static NotificationHost& Get();

    void post(const std::string& message,
              NotificationLevel level = NotificationLevel::Info);
    void dismissAll();
    void removeExpired();

    void paint(IPaintContext& context, NF::Rect viewport);

    [[nodiscard]] size_t count() const { return m_cards.size(); }

private:
    NotificationHost() = default;
    std::vector<std::shared_ptr<NotificationCard>> m_cards;
};

} // namespace NF::UI::AtlasUI
