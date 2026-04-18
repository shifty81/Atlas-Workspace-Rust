#pragma once

#include "NF/UI/AtlasUI/Commands/CommandTypes.h"
#include <optional>
#include <unordered_map>

namespace NF::UI::AtlasUI {

class CommandRegistry {
public:
    bool Register(CommandSpec spec);
    bool Unregister(const CommandId& id);

    [[nodiscard]] const CommandSpec* Find(const CommandId& id) const;
    [[nodiscard]] std::optional<CommandId> FindByChord(const KeyChord& chord,
                                                       CommandScope preferredScope) const;
    [[nodiscard]] std::vector<const CommandSpec*> List() const;

private:
    std::unordered_map<CommandId, CommandSpec> m_commands;
};

} // namespace NF::UI::AtlasUI
