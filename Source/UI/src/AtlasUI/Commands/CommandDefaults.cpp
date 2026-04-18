#include "NF/UI/AtlasUI/Commands/CommandDefaults.h"
#include "NF/UI/AtlasUI/Commands/CommandHelpers.h"
#include "NF/UI/AtlasUI/Commands/CommandManager.h"

namespace NF::UI::AtlasUI {

namespace {
    constexpr int KeyS = 'S';
    constexpr int KeyP = 'P';
    constexpr int KeyB = 'B';
} // namespace

void RegisterCoreCommands() {
    auto& registry = CommandManager::Get().Registry();

    registry.Register(CommandSpec{
        .Id = "workspace.save",
        .Label = "Save",
        .Description = "Writes current workspace or project state to disk.",
        .Category = "File",
        .Scope = CommandScope::Global,
        .Binding = {MakeChord(KeyS, true), {}},
        .Execute = [](const CommandContext&) {}
    });

    registry.Register(CommandSpec{
        .Id = "workspace.command_palette",
        .Label = "Command Palette",
        .Description = "Opens the global command palette.",
        .Category = "Tools",
        .Scope = CommandScope::Global,
        .Binding = {MakeChord(KeyP, true, false, true), {}},
        .Execute = [](const CommandContext&) {}
    });

    registry.Register(CommandSpec{
        .Id = "workspace.build",
        .Label = "Build",
        .Description = "Starts the active build pipeline.",
        .Category = "Build",
        .Scope = CommandScope::Global,
        .Binding = {MakeChord(KeyB, true), {}},
        .Execute = [](const CommandContext&) {}
    });

    CommandManager::Get().RebuildRouter();
}

} // namespace NF::UI::AtlasUI
