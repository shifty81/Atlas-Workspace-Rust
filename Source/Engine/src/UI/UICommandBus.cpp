#include "UICommandBus.h"

namespace atlas::ui {

void UICommandBus::Enqueue(const UICommand& cmd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pending.push_back(cmd);
}

void UICommandBus::Enqueue(UICommand&& cmd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pending.push_back(std::move(cmd));
}

void UICommandBus::Drain(std::vector<UICommand>& out) {
    std::lock_guard<std::mutex> lock(m_mutex);
    out.insert(out.end(), std::make_move_iterator(m_pending.begin()),
               std::make_move_iterator(m_pending.end()));
    m_pending.clear();
}

size_t UICommandBus::PendingCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_pending.size();
}

void UICommandBus::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pending.clear();
}

void UICommandBus::RegisterHandler(UICommandType type, Handler handler) {
    m_handlers[static_cast<uint8_t>(type)].push_back(std::move(handler));
}

void UICommandBus::Dispatch() {
    std::vector<UICommand> commands;
    Drain(commands);

    for (const auto& cmd : commands) {
        auto it = m_handlers.find(static_cast<uint8_t>(cmd.type));
        if (it != m_handlers.end()) {
            for (auto& handler : it->second) {
                handler(cmd);
            }
        }
    }
}

} // namespace atlas::ui
