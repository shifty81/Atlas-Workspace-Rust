#include "NF/UI/AtlasUI/Commands/CommandRegistry.h"

namespace NF::UI::AtlasUI {

bool CommandRegistry::Register(CommandSpec spec) {
    if (spec.Id.empty() || !spec.Execute) {
        return false;
    }
    return m_commands.emplace(spec.Id, std::move(spec)).second;
}

bool CommandRegistry::Unregister(const CommandId& id) {
    return m_commands.erase(id) > 0;
}

const CommandSpec* CommandRegistry::Find(const CommandId& id) const {
    const auto it = m_commands.find(id);
    return it == m_commands.end() ? nullptr : &it->second;
}

std::optional<CommandId> CommandRegistry::FindByChord(const KeyChord& chord,
                                                      CommandScope preferredScope) const {
    for (const auto& [id, spec] : m_commands) {
        if (spec.Scope == preferredScope &&
            (spec.Binding.Primary == chord || spec.Binding.Secondary == chord)) {
            return id;
        }
    }
    for (const auto& [id, spec] : m_commands) {
        if (spec.Binding.Primary == chord || spec.Binding.Secondary == chord) {
            return id;
        }
    }
    return std::nullopt;
}

std::vector<const CommandSpec*> CommandRegistry::List() const {
    std::vector<const CommandSpec*> out;
    out.reserve(m_commands.size());
    for (const auto& [_, spec] : m_commands) {
        out.push_back(&spec);
    }
    return out;
}

} // namespace NF::UI::AtlasUI
