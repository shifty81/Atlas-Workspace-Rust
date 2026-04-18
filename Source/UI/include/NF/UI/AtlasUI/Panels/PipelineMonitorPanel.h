#pragma once

#include "NF/UI/AtlasUI/PanelBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

/// AtlasUI PipelineMonitorPanel — displays build pipeline events.
/// Replaces the legacy NF::Editor::PipelineMonitorPanel for the AtlasUI framework.
class PipelineMonitorPanel final : public PanelBase {
public:
    PipelineMonitorPanel()
        : PanelBase("atlas.pipeline_monitor", "Pipeline Monitor") {}

    void paint(IPaintContext& context) override;

    struct PipelineEvent {
        std::string type;
        std::string source;
        std::string details;
        float timestamp = 0.f;
    };

    void addEvent(const std::string& type, const std::string& source,
                  const std::string& details, float timestamp = 0.f) {
        m_events.push_back({type, source, details, timestamp});
        if (m_events.size() > kMaxEvents) {
            m_events.erase(m_events.begin());
        }
    }

    void clearEvents() { m_events.clear(); }
    [[nodiscard]] size_t eventCount() const { return m_events.size(); }
    [[nodiscard]] const std::vector<PipelineEvent>& events() const { return m_events; }

    static constexpr size_t kMaxEvents = 500;

private:
    std::vector<PipelineEvent> m_events;
};

} // namespace NF::UI::AtlasUI
