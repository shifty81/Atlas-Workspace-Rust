#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

using CommandId = std::string;

enum class CommandScope : uint8_t {
    Global,
    Window,
    Panel,
    Widget,
    TextInput,
    Viewport,
    Graph
};

enum class CommandSource : uint8_t {
    Unknown,
    Keyboard,
    Toolbar,
    Menu,
    ContextMenu,
    CommandPalette,
    AtlasAI,
    Notification,
    Script
};

struct KeyChord {
    bool Ctrl = false;
    bool Alt = false;
    bool Shift = false;
    bool Win = false;
    int Key = 0;

    [[nodiscard]] bool IsValid() const noexcept { return Key != 0; }
    [[nodiscard]] std::string ToString() const;

    friend bool operator==(const KeyChord& a, const KeyChord& b) noexcept {
        return a.Ctrl == b.Ctrl && a.Alt == b.Alt && a.Shift == b.Shift &&
               a.Win == b.Win && a.Key == b.Key;
    }
};

struct CommandContext {
    CommandSource Source = CommandSource::Unknown;
    CommandScope Scope = CommandScope::Global;
    void* Window = nullptr;
    void* Panel = nullptr;
    void* Widget = nullptr;
    void* UserData = nullptr;
};

struct CommandState {
    bool Enabled = true;
    bool Visible = true;
    bool Checked = false;
};

struct CommandBinding {
    KeyChord Primary;
    KeyChord Secondary;
};

using CommandExecuteFn = std::function<void(const CommandContext&)>;
using CommandStateFn = std::function<CommandState(const CommandContext&)>;

struct CommandSpec {
    CommandId Id;
    std::string Label;
    std::string Description;
    std::string Category;
    CommandScope Scope = CommandScope::Global;
    CommandBinding Binding;
    CommandExecuteFn Execute;
    CommandStateFn QueryState;
};

} // namespace NF::UI::AtlasUI
