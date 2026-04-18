#include "NF/UI/AtlasUI/Commands/ShortcutRouter.h"

namespace NF::UI::AtlasUI {

ShortcutRouter::ShortcutRouter(const CommandRegistry& registry)
    : m_registry(&registry) {
}

bool ShortcutRouter::TryExecute(const KeyChord& chord,
                                 const CommandContext& context) const {
    const auto id = m_registry->FindByChord(chord, context.Scope);
    if (!id) {
        return false;
    }

    const CommandSpec* spec = m_registry->Find(*id);
    if (!spec || !spec->Execute) {
        return false;
    }

    const CommandState state = spec->QueryState ? spec->QueryState(context) : CommandState{};
    if (!state.Enabled || !state.Visible) {
        return false;
    }

    spec->Execute(context);
    return true;
}

} // namespace NF::UI::AtlasUI
