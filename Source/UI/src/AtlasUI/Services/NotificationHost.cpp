#include "NF/UI/AtlasUI/Services/NotificationHost.h"
#include <algorithm>

namespace NF::UI::AtlasUI {

NotificationHost& NotificationHost::Get() {
    static NotificationHost instance;
    return instance;
}

void NotificationHost::post(const std::string& message, NotificationLevel level) {
    auto card = std::make_shared<NotificationCard>(message, level);
    m_cards.push_back(std::move(card));
}

void NotificationHost::dismissAll() {
    for (auto& card : m_cards) {
        card->dismiss();
    }
    removeExpired();
}

void NotificationHost::removeExpired() {
    auto it = std::remove_if(m_cards.begin(), m_cards.end(),
        [](const std::shared_ptr<NotificationCard>& card) {
            return card->isExpired();
        });
    m_cards.erase(it, m_cards.end());
}

void NotificationHost::paint(IPaintContext& context, NF::Rect viewport) {
    constexpr float kCardHeight = 48.f;
    constexpr float kCardWidth = 300.f;
    constexpr float kGap = 8.f;

    float y = viewport.y + viewport.h - kGap;
    float x = viewport.x + viewport.w - kCardWidth - kGap;

    for (auto& card : m_cards) {
        if (card->isExpired()) continue;
        y -= kCardHeight;
        card->arrange({x, y, kCardWidth, kCardHeight});
        card->paint(context);
        y -= kGap;
    }
}

} // namespace NF::UI::AtlasUI
