#pragma once
// NF::Editor — Pipeline monitor panel
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>
#include "NF/Editor/ToolWindowManager.h"
#include "NF/Editor/EditorPanel.h"

namespace NF {

class PipelineMonitorPanel : public EditorPanel {
public:
    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Bottom; }
    void update(float /*dt*/) override {}

    void render(UIRenderer& ui, const Rect& bounds, const EditorTheme& theme) override {
        ui.drawRect(bounds, theme.panelBackground);
        Rect hdr{bounds.x, bounds.y, bounds.w, 22.f};
        ui.drawRect(hdr, theme.panelHeader);
        ui.drawText(bounds.x + 8.f, bounds.y + 4.f, "Pipeline Monitor", theme.panelText);
        ui.drawRectOutline(bounds, theme.panelBorder, 1.f);

        float y = bounds.y + 26.f;
        for (auto& ev : m_events) {
            if (y > bounds.y + bounds.h - 4.f) break;
            std::string line = "[" + ev.type + "] " + ev.source + ": " + ev.details;
            ui.drawText(bounds.x + 8.f, y, line, theme.panelText);
            y += 16.f;
        }
    }

    void renderUI(UIContext& ctx, const Rect& bounds) override {
        ctx.beginPanel("Pipeline Monitor", bounds);
        ctx.beginHorizontal();
        if (ctx.button("Clear")) m_events.clear();
        if (ctx.button("Refresh")) { /* poll pipeline */ }
        ctx.endHorizontal();
        ctx.separator();
        float contentH = static_cast<float>(m_events.size()) * 18.f;
        Rect scrollR{bounds.x, bounds.y + 60.f, bounds.w, bounds.h - 64.f};
        ctx.beginScrollArea("pipeline_scroll", scrollR, contentH);
        for (auto& ev : m_events) {
            std::string line = "[" + ev.type + "] " + ev.source + ": " + ev.details;
            ctx.label(line);
        }
        ctx.endScrollArea();
        ctx.endPanel();
    }

    void addEvent(const std::string& type, const std::string& source,
                  const std::string& details, float timestamp) {
        PipelineEventEntry e;
        e.type = type;
        e.source = source;
        e.details = details;
        e.timestamp = timestamp;
        m_events.push_back(std::move(e));
        if (m_events.size() > 500) m_events.erase(m_events.begin());
    }

    void clearEvents() { m_events.clear(); }
    [[nodiscard]] size_t eventCount() const { return m_events.size(); }
    [[nodiscard]] const std::vector<PipelineEventEntry>& events() const { return m_events; }

private:
    std::string m_name = "PipelineMonitor";
    std::vector<PipelineEventEntry> m_events;
};

// ── Frame Stats ──────────────────────────────────────────────────


} // namespace NF
