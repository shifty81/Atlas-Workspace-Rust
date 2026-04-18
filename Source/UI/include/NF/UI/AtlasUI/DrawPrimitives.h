#pragma once

#include "NF/Core/Core.h"
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace NF::UI::AtlasUI {

using Color = uint32_t;
using FontId = uint32_t;

struct DrawRectCmd {
    NF::Rect rect{};
    Color color = 0xFFFFFFFF;
};

struct DrawImageCmd {
    NF::Rect rect{};
    uint32_t textureId = 0;
    Color tint = 0xFFFFFFFF;
};

struct FillRectCmd {
    NF::Rect rect{};
    Color color = 0xFFFFFFFF;
};

struct DrawTextCmd {
    NF::Rect rect{};
    std::string text;
    FontId font = 0;
    Color color = 0xFFFFFFFF;
};

struct PushClipCmd {
    NF::Rect rect{};
};

struct PopClipCmd {};

using DrawCommand = std::variant<DrawRectCmd, FillRectCmd, DrawTextCmd, DrawImageCmd, PushClipCmd, PopClipCmd>;

class DrawList {
public:
    void clear() { m_commands.clear(); }

    void push(const DrawCommand& command) { m_commands.push_back(command); }
    void push(DrawCommand&& command) { m_commands.push_back(std::move(command)); }

    [[nodiscard]] const std::vector<DrawCommand>& commands() const { return m_commands; }
    [[nodiscard]] bool empty() const { return m_commands.empty(); }
    [[nodiscard]] size_t size() const { return m_commands.size(); }

private:
    std::vector<DrawCommand> m_commands;
};

} // namespace NF::UI::AtlasUI
