#pragma once
#include "../ui/EditorPanel.h"
#include "../tools/PlayInEditor.h"
#include "ReplayTimelinePanel.h"
#include "StateHashDiffPanel.h"
#include "JobTracePanel.h"
#include "../../engine/ui/UIDrawList.h"
#include <string>
#include <sstream>

namespace atlas::editor {

/// Combined "Truth UI" dashboard panel.
/// Aggregates simulation control, state hash inspection, replay timeline,
/// and job execution trace into a single debugger view.
class TruthUIPanel : public EditorPanel {
public:
    const char* Name() const override { return "Truth UI"; }

    void Draw() override {
        m_drawList.Clear();

        // Panel background
        atlas::ui::UIRect panelRect{m_x, m_y, m_width, m_height};
        m_drawList.DrawRect(panelRect, {43, 43, 43, 255});
        m_drawList.DrawBorder(panelRect, 1, {70, 73, 75, 255});

        // Header
        atlas::ui::UIRect headerRect{m_x, m_y, m_width, 28};
        m_drawList.DrawRect(headerRect, {60, 63, 65, 255});
        m_drawList.DrawText(headerRect, "Truth UI", {230, 230, 230, 255});

        int32_t yOffset = m_y + 32;

        // Section 1: Simulation Control
        atlas::ui::UIRect simLabel{m_x + 8, yOffset, m_width - 16, 20};
        if (m_pie) {
            std::string modeStr;
            switch (m_pie->Mode()) {
                case PIEMode::Stopped:    modeStr = "Stopped"; break;
                case PIEMode::Simulating: modeStr = "Simulating"; break;
                case PIEMode::Paused:     modeStr = "Paused"; break;
                case PIEMode::Possessed:  modeStr = "Possessed"; break;
            }
            m_drawList.DrawText(simLabel, "Mode: " + modeStr, {220, 220, 220, 255});
            yOffset += 22;

            atlas::ui::UIRect tickLabel{m_x + 8, yOffset, m_width - 16, 20};
            m_drawList.DrawText(tickLabel, "Ticks: " + std::to_string(m_pie->TicksSimulated()),
                                {160, 160, 160, 255});
            yOffset += 22;
        } else {
            m_drawList.DrawText(simLabel, "No simulation attached", {160, 160, 160, 255});
            yOffset += 22;
        }

        // Separator
        atlas::ui::UIRect sep1{m_x + 4, yOffset, m_width - 8, 1};
        m_drawList.DrawRect(sep1, {50, 50, 50, 255});
        yOffset += 6;

        // Section 2: Determinism View
        atlas::ui::UIRect detLabel{m_x + 8, yOffset, m_width - 16, 20};
        m_drawList.DrawText(detLabel, "Determinism", {230, 230, 230, 255});
        yOffset += 22;

        if (m_hashDiff && m_hashDiff->HasDivergence()) {
            atlas::ui::UIRect divLabel{m_x + 8, yOffset, m_width - 16, 20};
            m_drawList.DrawText(divLabel,
                "Divergence at tick " + std::to_string(m_hashDiff->FirstDivergenceTick()),
                {230, 80, 80, 255});
            yOffset += 22;
        } else {
            atlas::ui::UIRect okLabel{m_x + 8, yOffset, m_width - 16, 20};
            m_drawList.DrawText(okLabel, "No divergence", {80, 200, 120, 255});
            yOffset += 22;
        }

        // Section 3: Job Trace
        if (m_jobTrace) {
            atlas::ui::UIRect sep2{m_x + 4, yOffset, m_width - 8, 1};
            m_drawList.DrawRect(sep2, {50, 50, 50, 255});
            yOffset += 6;

            atlas::ui::UIRect jtLabel{m_x + 8, yOffset, m_width - 16, 20};
            if (m_jobTrace->HasOrderMismatch()) {
                m_drawList.DrawText(jtLabel,
                    "Order mismatch at tick " + std::to_string(m_jobTrace->FirstMismatchTick()),
                    {230, 180, 60, 255});
            } else {
                m_drawList.DrawText(jtLabel, "Job order OK", {80, 200, 120, 255});
            }
            yOffset += 22;
        }

        // Section 4: Replay Timeline
        if (m_timeline) {
            atlas::ui::UIRect sep3{m_x + 4, yOffset, m_width - 8, 1};
            m_drawList.DrawRect(sep3, {50, 50, 50, 255});
            yOffset += 6;

            atlas::ui::UIRect tlLabel{m_x + 8, yOffset, m_width - 16, 20};
            std::string tlText = "Replay: tick " + std::to_string(m_timeline->GetCurrentTick())
                                 + " / " + std::to_string(m_timeline->TotalTicks());
            m_drawList.DrawText(tlLabel, tlText, {160, 160, 160, 255});
        }
    }

    /// Access the draw list produced by the last Draw() call.
    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

    /// Set the panel bounds for rendering.
    void SetBounds(int32_t x, int32_t y, int32_t w, int32_t h) {
        m_x = x; m_y = y; m_width = w; m_height = h;
    }

    // --- Wiring ---

    void SetPlayInEditor(PlayInEditor* pie) { m_pie = pie; }
    PlayInEditor* GetPlayInEditor() const { return m_pie; }

    void SetReplayTimeline(ReplayTimelinePanel* timeline) { m_timeline = timeline; }
    ReplayTimelinePanel* GetReplayTimeline() const { return m_timeline; }

    void SetStateHashDiff(StateHashDiffPanel* hashDiff) { m_hashDiff = hashDiff; }
    StateHashDiffPanel* GetStateHashDiff() const { return m_hashDiff; }

    void SetJobTrace(JobTracePanel* jobTrace) { m_jobTrace = jobTrace; }
    JobTracePanel* GetJobTrace() const { return m_jobTrace; }

    // --- Status queries ---

    bool IsSimulating() const {
        if (!m_pie) return false;
        auto mode = m_pie->Mode();
        return mode == PIEMode::Simulating || mode == PIEMode::Possessed;
    }

    bool IsPaused() const {
        if (!m_pie) return false;
        return m_pie->Mode() == PIEMode::Paused;
    }

    bool HasDivergence() const {
        if (m_hashDiff && m_hashDiff->HasDivergence()) return true;
        if (m_timeline && m_timeline->HasDivergence()) return true;
        return false;
    }

    bool HasOrderMismatch() const {
        if (!m_jobTrace) return false;
        return m_jobTrace->HasOrderMismatch();
    }

    std::string Summary() const {
        std::ostringstream oss;
        oss << "Truth UI";

        if (m_pie) {
            switch (m_pie->Mode()) {
                case PIEMode::Stopped:    oss << " [Stopped]"; break;
                case PIEMode::Simulating: oss << " [Simulating]"; break;
                case PIEMode::Paused:     oss << " [Paused]"; break;
                case PIEMode::Possessed:  oss << " [Possessed]"; break;
            }
            oss << " Ticks: " << m_pie->TicksSimulated();
        }

        if (HasDivergence()) oss << " ⚠ DIVERGENCE";
        if (HasOrderMismatch()) oss << " ⚠ ORDER MISMATCH";

        return oss.str();
    }

private:
    PlayInEditor* m_pie = nullptr;
    ReplayTimelinePanel* m_timeline = nullptr;
    StateHashDiffPanel* m_hashDiff = nullptr;
    JobTracePanel* m_jobTrace = nullptr;
    atlas::ui::UIDrawList m_drawList;
    int32_t m_x = 0;
    int32_t m_y = 0;
    int32_t m_width = 400;
    int32_t m_height = 300;
};

} // namespace atlas::editor
