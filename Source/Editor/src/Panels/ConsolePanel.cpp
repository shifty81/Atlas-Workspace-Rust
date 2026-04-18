#include "ConsolePanel.h"
#include "../ai/AIAggregator.h"
#include <sstream>

namespace atlas::editor {

void ConsolePanel::Draw() {
    m_drawList.Clear();

    // Background
    m_drawList.DrawRect({0, 0, 600, 400}, {30, 30, 30, 255});

    // Title bar
    m_drawList.DrawRect({0, 0, 600, 24}, {50, 50, 50, 255});
    m_drawList.DrawText({4, 4, 200, 20}, "Console", {220, 220, 220, 255});

    // History lines
    int32_t y = 28;
    for (const auto& line : m_history) {
        atlas::ui::UIColor color = {200, 200, 200, 255};
        if (!line.empty() && line[0] == '>') {
            color = {100, 200, 255, 255};
        }
        m_drawList.DrawText({4, y, 592, 16}, line, color);
        y += 18;
    }

    // Input field
    m_drawList.DrawRect({0, 380, 600, 20}, {40, 40, 40, 255});
    m_drawList.DrawText({4, 382, 592, 16}, "> " + m_inputBuffer, {180, 220, 255, 255});
}

void ConsolePanel::AddLine(const std::string& line) {
    m_history.push_back(line);
}

void ConsolePanel::SetAIAggregator(atlas::ai::AIAggregator* agg) {
    m_aiAggregator = agg;
}

void ConsolePanel::Execute(const std::string& command) {
    m_history.push_back("> " + command);

    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;

    if (cmd == "spawn_entity") {
        auto id = m_world.CreateEntity();
        m_history.push_back("Created entity " + std::to_string(id));
    } else if (cmd == "ecs.dump") {
        auto entities = m_world.GetEntities();
        m_history.push_back("Entities: " + std::to_string(entities.size()));
        for (auto e : entities) {
            auto types = m_world.GetComponentTypes(e);
            m_history.push_back("  Entity " + std::to_string(e) + " (" + std::to_string(types.size()) + " components)");
        }
    } else if (cmd == "set") {
        std::string key;
        iss >> key;
        if (key == "tickrate") {
            uint32_t rate = 0;
            iss >> rate;
            if (rate > 0) {
                m_scheduler.SetTickRate(rate);
                m_history.push_back("Tick rate set to " + std::to_string(rate));
            } else {
                m_history.push_back("Invalid tick rate");
            }
        } else {
            m_history.push_back("Unknown setting: " + key);
        }
    } else if (cmd == "net.mode") {
        auto mode = m_net.Mode();
        std::string modeStr;
        switch (mode) {
            case net::NetMode::Standalone: modeStr = "Standalone"; break;
            case net::NetMode::Client: modeStr = "Client"; break;
            case net::NetMode::Server: modeStr = "Server"; break;
            case net::NetMode::P2P_Host: modeStr = "P2P_Host"; break;
            case net::NetMode::P2P_Peer: modeStr = "P2P_Peer"; break;
        }
        m_history.push_back("Net mode: " + modeStr);
    } else if (cmd == "help") {
        m_history.push_back("Commands: spawn_entity, ecs.dump, set tickrate <N>, net.mode, save <path>, load <path>, time.info, help");
    } else if (cmd == "save") {
        std::string path;
        iss >> path;
        if (path.empty()) {
            m_history.push_back("Usage: save <path>");
        } else if (!m_saveSystem) {
            m_history.push_back("Save system not available");
        } else {
            auto ecsData = m_world.Serialize();
            uint64_t tick = m_timeModel ? m_timeModel->Context().sim.tick : 0;
            uint32_t tickRate = m_scheduler.TickRate();
            auto result = m_saveSystem->Save(path, tick, tickRate, 0, ecsData);
            if (result == sim::SaveResult::Success) {
                m_history.push_back("Saved at tick " + std::to_string(tick) + " to " + path);
            } else {
                m_history.push_back("Save failed");
            }
        }
    } else if (cmd == "load") {
        std::string path;
        iss >> path;
        if (path.empty()) {
            m_history.push_back("Usage: load <path>");
        } else if (!m_saveSystem) {
            m_history.push_back("Save system not available");
        } else {
            auto result = m_saveSystem->Load(path);
            if (result == sim::SaveResult::Success) {
                m_world.Deserialize(m_saveSystem->ECSData());
                m_history.push_back("Loaded from " + path + " at tick " + std::to_string(m_saveSystem->Header().saveTick));
            } else if (result == sim::SaveResult::FileNotFound) {
                m_history.push_back("File not found: " + path);
            } else {
                m_history.push_back("Load failed");
            }
        }
    } else if (cmd == "time.info") {
        if (!m_timeModel) {
            m_history.push_back("Time model not available");
        } else {
            const auto& ctx = m_timeModel->Context();
            m_history.push_back("Tick: " + std::to_string(ctx.sim.tick)
                + " Rate: " + std::to_string(ctx.sim.tickRate) + " Hz"
                + " World: " + std::to_string(ctx.world.elapsed) + "s"
                + " Dilation: " + std::to_string(ctx.world.dilation));
        }
    } else if (cmd == "ecs.count") {
        m_history.push_back("Entity count: " + std::to_string(m_world.EntityCount()));
    } else if (cmd == "ecs.destroy") {
        uint32_t id = 0;
        iss >> id;
        if (m_world.IsAlive(id)) {
            m_world.DestroyEntity(id);
            m_history.push_back("Destroyed entity " + std::to_string(id));
        } else {
            m_history.push_back("Invalid or dead entity " + std::to_string(id));
        }
    } else if (cmd == "net.stats") {
        m_history.push_back("Peers total: " + std::to_string(m_net.Peers().size()));
    } else if (cmd == "net.peers") {
        m_history.push_back("Connected peers: " + std::to_string(m_net.Peers().size()));
    } else if (cmd == "clear") {
        m_history.clear();
    } else if (cmd == "ai.query") {
        std::string rest;
        std::getline(iss >> std::ws, rest);
        if (rest.empty()) {
            m_history.push_back("Usage: ai.query <prompt>");
        } else if (!m_aiAggregator) {
            m_history.push_back("AI backend not connected");
        } else {
            ai::AIContext ctx;
            auto resp = m_aiAggregator->Execute(ai::AIRequestType::Analysis, rest, ctx);
            m_history.push_back("[AI] " + resp.content);
            m_history.push_back("[AI] Confidence: " + std::to_string(resp.confidence));
        }
    } else if (cmd == "list") {
        m_history.push_back("Available panels: Console, Viewport, Inspector, Hierarchy");
    } else if (cmd == "status") {
        m_history.push_back("Entities: " + std::to_string(m_world.EntityCount()));
        std::string modeStr;
        switch (m_net.Mode()) {
            case net::NetMode::Standalone: modeStr = "Standalone"; break;
            case net::NetMode::Client: modeStr = "Client"; break;
            case net::NetMode::Server: modeStr = "Server"; break;
            case net::NetMode::P2P_Host: modeStr = "P2P_Host"; break;
            case net::NetMode::P2P_Peer: modeStr = "P2P_Peer"; break;
        }
        m_history.push_back("Net Mode: " + modeStr);
    } else {
        m_history.push_back("Unknown command: " + cmd);
    }
}

}
