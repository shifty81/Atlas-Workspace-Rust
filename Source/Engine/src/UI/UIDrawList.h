#pragma once
#include "UIRenderer.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::ui {

/// Describes a single deferred draw command.
/// Draw commands are buffered during the UI update pass and then
/// consumed by the rendering backend in one batch.  This decouples
/// UI logic from the graphics API, enables deterministic rendering
/// and makes it possible to inspect / replay UI frames.
struct UIDrawCmd {
    enum class Kind : uint8_t {
        Rect,
        Text,
        Icon,
        Border,
        Image
    };

    Kind kind = Kind::Rect;
    UIRect rect{};
    UIColor color{};
    int32_t thickness = 0;     ///< For borders
    uint32_t resourceId = 0;   ///< Icon or texture ID
    std::string text;          ///< For text commands
};

/// Accumulates draw commands for a single frame.
class UIDrawList {
public:
    void DrawRect(const UIRect& rect, const UIColor& color);
    void DrawText(const UIRect& rect, const std::string& text, const UIColor& color);
    void DrawIcon(const UIRect& rect, uint32_t iconId, const UIColor& tint);
    void DrawBorder(const UIRect& rect, int32_t thickness, const UIColor& color);
    void DrawImage(const UIRect& rect, uint32_t textureId, const UIColor& tint);

    /// Replay all buffered commands through a concrete UIRenderer.
    void Flush(UIRenderer* renderer) const;

    /// Discard all buffered commands (call at end of frame).
    void Clear();

    /// Number of buffered commands.
    size_t CommandCount() const;

    /// Read-only access to the command buffer (for inspection / replay).
    const std::vector<UIDrawCmd>& Commands() const;

private:
    std::vector<UIDrawCmd> m_commands;
};

} // namespace atlas::ui
