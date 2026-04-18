#include "InteractionDebugPanel.h"

namespace atlas::editor {

void InteractionDebugPanel::Draw() {
    m_drawList.Clear();

    // Background
    m_drawList.DrawRect({0, 0, 700, 400}, {30, 30, 30, 255});

    // Title
    m_drawList.DrawRect({0, 0, 700, 24}, {50, 50, 50, 255});
    m_drawList.DrawText({4, 4, 200, 20}, "Interaction Debugger", {220, 220, 220, 255});

    // Column headers
    m_drawList.DrawText({4, 28, 120, 16}, "Speaker", {160, 160, 160, 255});
    m_drawList.DrawText({130, 28, 250, 16}, "Text", {160, 160, 160, 255});
    m_drawList.DrawText({390, 28, 150, 16}, "Intent", {160, 160, 160, 255});
    m_drawList.DrawText({550, 28, 80, 16}, "Confidence", {160, 160, 160, 255});

    // Log entries
    int32_t y = 46;
    for (const auto& entry : m_log) {
        m_drawList.DrawText({4, y, 120, 16}, entry.speaker, {200, 200, 200, 255});
        m_drawList.DrawText({130, y, 250, 16}, entry.text, {200, 200, 200, 255});
        m_drawList.DrawText({390, y, 150, 16}, entry.resolvedIntent, {180, 220, 180, 255});
        std::string conf = std::to_string(entry.confidence).substr(0, 4);
        m_drawList.DrawText({550, y, 80, 16}, conf, {200, 200, 200, 255});
        y += 18;
    }
}

void InteractionDebugPanel::LogInteraction(const InteractionLogEntry& entry) {
    m_log.push_back(entry);
}

}
