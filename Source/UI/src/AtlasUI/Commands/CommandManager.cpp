#include "NF/UI/AtlasUI/Commands/CommandManager.h"

namespace NF::UI::AtlasUI {

CommandManager& CommandManager::Get() {
    static CommandManager instance;
    return instance;
}

CommandManager::CommandManager()
    : m_router(m_registry) {
}

CommandRegistry& CommandManager::Registry() noexcept {
    return m_registry;
}

const CommandRegistry& CommandManager::Registry() const noexcept {
    return m_registry;
}

ShortcutRouter& CommandManager::Router() noexcept {
    return m_router;
}

const ShortcutRouter& CommandManager::Router() const noexcept {
    return m_router;
}

void CommandManager::RebuildRouter() {
    m_router = ShortcutRouter(m_registry);
}

} // namespace NF::UI::AtlasUI
