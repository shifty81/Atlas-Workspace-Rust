#include "DesyncVisualizerPanel.h"
#include <sstream>

namespace atlas::editor {

void DesyncVisualizerPanel::SetMirrorController(
    const atlas::sim::SimMirrorController* controller) {
    m_controller = controller;
}

void DesyncVisualizerPanel::AddEvent(const DesyncDisplayEvent& event) {
    m_events.push_back(event);
}

void DesyncVisualizerPanel::ClearEvents() {
    m_events.clear();
    m_selectedIndex = 0;
}

const std::vector<DesyncDisplayEvent>& DesyncVisualizerPanel::Events() const {
    return m_events;
}

void DesyncVisualizerPanel::SelectEvent(size_t index) {
    if (index < m_events.size()) {
        m_selectedIndex = index;
    }
}

size_t DesyncVisualizerPanel::SelectedIndex() const {
    return m_selectedIndex;
}

void DesyncVisualizerPanel::Draw() {
    m_drawList.Clear();

    // Sync from mirror controller if connected
    if (m_controller && m_controller->HasDesync()) {
        for (const auto& d : m_controller->Desyncs()) {
            bool alreadyTracked = false;
            for (const auto& e : m_events) {
                if (e.tick == d.tick) { alreadyTracked = true; break; }
            }
            if (!alreadyTracked) {
                DesyncDisplayEvent de;
                de.tick = d.tick;
                de.serverHash = d.serverHash;
                de.clientHash = d.clientHash;
                m_events.push_back(de);
            }
        }
    }

    // Background
    m_drawList.DrawRect({0, 0, 600, 400}, {30, 30, 30, 255});

    // Title bar
    m_drawList.DrawRect({0, 0, 600, 24}, {50, 50, 50, 255});
    m_drawList.DrawText({4, 4, 300, 20}, "Desync Visualizer", {220, 220, 220, 255});

    int32_t y = 28;

    if (m_events.empty()) {
        m_drawList.DrawText({4, y, 590, 16}, "No desyncs detected",
                            {100, 200, 100, 255});
        return;
    }

    // Summary
    std::string summary = "Desyncs: " + std::to_string(m_events.size());
    m_drawList.DrawText({4, y, 590, 16}, summary, {200, 100, 100, 255});
    y += 20;

    // Column header
    m_drawList.DrawText({4, y, 100, 16}, "Tick", {180, 180, 180, 255});
    m_drawList.DrawText({110, y, 200, 16}, "Server Hash", {180, 180, 180, 255});
    m_drawList.DrawText({320, y, 200, 16}, "Client Hash", {180, 180, 180, 255});
    y += 18;

    // Event list
    for (size_t i = 0; i < m_events.size(); ++i) {
        const auto& evt = m_events[i];
        bool selected = (i == m_selectedIndex);

        atlas::ui::UIColor bg = selected
            ? atlas::ui::UIColor{80, 40, 40, 255}
            : atlas::ui::UIColor{40, 40, 40, 255};
        m_drawList.DrawRect({0, y, 600, 18}, bg);

        std::ostringstream tickStr;
        tickStr << evt.tick;
        m_drawList.DrawText({4, y + 1, 100, 16}, tickStr.str(),
                            {255, 80, 80, 255});

        std::ostringstream serverStr;
        serverStr << std::hex << evt.serverHash;
        m_drawList.DrawText({110, y + 1, 200, 16}, serverStr.str(),
                            {200, 200, 200, 255});

        std::ostringstream clientStr;
        clientStr << std::hex << evt.clientHash;
        m_drawList.DrawText({320, y + 1, 200, 16}, clientStr.str(),
                            {200, 200, 200, 255});
        y += 20;
    }

    // Detail view for selected event
    if (m_selectedIndex < m_events.size()) {
        const auto& sel = m_events[m_selectedIndex];
        if (!sel.details.empty()) {
            y += 4;
            m_drawList.DrawRect({0, y, 600, 2}, {80, 80, 80, 255});
            y += 6;
            m_drawList.DrawText({4, y, 590, 16}, "Field Details:",
                                {220, 220, 220, 255});
            y += 18;

            for (const auto& detail : sel.details) {
                std::string line = detail.entityName + "." +
                                   detail.componentName + "." +
                                   detail.fieldName;
                m_drawList.DrawText({8, y, 250, 16}, line,
                                    {200, 200, 200, 255});

                m_drawList.DrawText({270, y, 150, 16},
                                    "S: " + detail.serverValue,
                                    {100, 200, 100, 255});
                m_drawList.DrawText({430, y, 150, 16},
                                    "C: " + detail.clientValue,
                                    {200, 100, 100, 255});
                y += 18;
            }
        }
    }
}

}  // namespace atlas::editor
