#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/ecs/ECS.h"
#include "../../engine/net/NetContext.h"
#include "../../engine/sim/TickScheduler.h"
#include "../../engine/sim/TimeModel.h"
#include "../../engine/sim/SaveSystem.h"
#include "../../engine/ui/UIDrawList.h"
#include <string>
#include <vector>
#include <sstream>

namespace atlas::ai { class AIAggregator; }

namespace atlas::editor {

class ConsolePanel : public EditorPanel {
public:
    ConsolePanel(ecs::World& world, net::NetContext& net, sim::TickScheduler& scheduler)
        : m_world(world), m_net(net), m_scheduler(scheduler) {}

    ConsolePanel(ecs::World& world, net::NetContext& net, sim::TickScheduler& scheduler,
                 sim::TimeModel& timeModel, sim::SaveSystem& saveSystem)
        : m_world(world), m_net(net), m_scheduler(scheduler),
          m_timeModel(&timeModel), m_saveSystem(&saveSystem) {}

    const char* Name() const override { return "Console"; }
    void Draw() override;

    void AddLine(const std::string& line);
    void Execute(const std::string& command);
    void SetAIAggregator(atlas::ai::AIAggregator* agg);

    const std::vector<std::string>& History() const { return m_history; }

    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    std::vector<std::string> m_history;
    std::string m_inputBuffer;
    ecs::World& m_world;
    net::NetContext& m_net;
    sim::TickScheduler& m_scheduler;
    sim::TimeModel* m_timeModel = nullptr;
    sim::SaveSystem* m_saveSystem = nullptr;
    atlas::ui::UIDrawList m_drawList;
    atlas::ai::AIAggregator* m_aiAggregator = nullptr;
};

}
