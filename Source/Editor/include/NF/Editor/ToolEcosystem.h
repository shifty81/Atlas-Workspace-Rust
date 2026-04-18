#pragma once
// NF::Editor — Standalone tool runner, health monitor, orchestrator, ecosystem
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

namespace NF {

enum class ToolStatus : uint8_t {
    Stopped, Starting, Running, Unhealthy, Stopping, Crashed, Unknown, Disabled
};

inline const char* toolStatusName(ToolStatus s) {
    switch (s) {
        case ToolStatus::Stopped:   return "Stopped";
        case ToolStatus::Starting:  return "Starting";
        case ToolStatus::Running:   return "Running";
        case ToolStatus::Unhealthy: return "Unhealthy";
        case ToolStatus::Stopping:  return "Stopping";
        case ToolStatus::Crashed:   return "Crashed";
        case ToolStatus::Unknown:   return "Unknown";
        case ToolStatus::Disabled:  return "Disabled";
    }
    return "Unknown";
}

struct ToolInstanceInfo {
    std::string name;
    std::string executablePath;
    ToolStatus status = ToolStatus::Stopped;
    int pid = -1;
    float uptimeSeconds = 0.f;
    size_t eventsHandled = 0;
    float lastHeartbeatAge = 0.f;  // seconds since last heartbeat
};

struct ToolEcosystemConfig {
    std::string pipelineDir = ".atlas/pipeline";
    float heartbeatIntervalSec = 5.f;
    float unhealthyThresholdSec = 15.f;
    float crashThresholdSec = 30.f;
    size_t maxEventsPerTick = 16;
    bool autoRestart = true;
};

class StandaloneToolRunner {
public:
    void setName(const std::string& n) { m_info.name = n; }
    void setExecutablePath(const std::string& p) { m_info.executablePath = p; }

    [[nodiscard]] const std::string& name() const { return m_info.name; }
    [[nodiscard]] const std::string& executablePath() const { return m_info.executablePath; }
    [[nodiscard]] ToolStatus status() const { return m_info.status; }
    [[nodiscard]] const ToolInstanceInfo& info() const { return m_info; }
    [[nodiscard]] float uptimeSeconds() const { return m_info.uptimeSeconds; }
    [[nodiscard]] size_t eventsHandled() const { return m_info.eventsHandled; }

    bool start() {
        if (m_info.status == ToolStatus::Running || m_info.status == ToolStatus::Starting) return false;
        m_info.status = ToolStatus::Starting;
        m_info.pid = static_cast<int>(std::hash<std::string>{}(m_info.name) % 65536);
        m_info.uptimeSeconds = 0.f;
        m_info.eventsHandled = 0;
        m_info.status = ToolStatus::Running;
        return true;
    }

    bool stop() {
        if (m_info.status != ToolStatus::Running && m_info.status != ToolStatus::Unhealthy) return false;
        m_info.status = ToolStatus::Stopping;
        m_info.pid = -1;
        m_info.status = ToolStatus::Stopped;
        return true;
    }

    void recordHeartbeat() {
        m_info.lastHeartbeatAge = 0.f;
    }

    void recordEvent() {
        m_info.eventsHandled++;
    }

    void tickUptime(float dt) {
        if (m_info.status == ToolStatus::Running || m_info.status == ToolStatus::Unhealthy) {
            m_info.uptimeSeconds += dt;
            m_info.lastHeartbeatAge += dt;
        }
    }

    void markCrashed() {
        m_info.status = ToolStatus::Crashed;
        m_info.pid = -1;
    }

    void markUnhealthy() {
        if (m_info.status == ToolStatus::Running)
            m_info.status = ToolStatus::Unhealthy;
    }

    [[nodiscard]] bool isAlive() const {
        return m_info.status == ToolStatus::Running || m_info.status == ToolStatus::Unhealthy;
    }

private:
    ToolInstanceInfo m_info;
};

class ToolHealthMonitor {
public:
    void setConfig(const ToolEcosystemConfig& config) { m_config = config; }

    void addRunner(StandaloneToolRunner* runner) {
        if (m_runners.size() < kMaxTools && runner)
            m_runners.push_back(runner);
    }

    void removeRunner(const std::string& name) {
        m_runners.erase(
            std::remove_if(m_runners.begin(), m_runners.end(),
                [&name](const StandaloneToolRunner* r) { return r->name() == name; }),
            m_runners.end());
    }

    void checkHealth() {
        for (auto* r : m_runners) {
            if (!r->isAlive()) continue;
            if (r->info().lastHeartbeatAge >= m_config.crashThresholdSec) {
                r->markCrashed();
            } else if (r->info().lastHeartbeatAge >= m_config.unhealthyThresholdSec) {
                r->markUnhealthy();
            }
        }
    }

    [[nodiscard]] size_t healthyCount() const {
        size_t count = 0;
        for (const auto* r : m_runners) {
            if (r->status() == ToolStatus::Running) ++count;
        }
        return count;
    }

    [[nodiscard]] size_t unhealthyCount() const {
        size_t count = 0;
        for (const auto* r : m_runners) {
            if (r->status() == ToolStatus::Unhealthy) ++count;
        }
        return count;
    }

    [[nodiscard]] size_t crashedCount() const {
        size_t count = 0;
        for (const auto* r : m_runners) {
            if (r->status() == ToolStatus::Crashed) ++count;
        }
        return count;
    }

    [[nodiscard]] size_t runnerCount() const { return m_runners.size(); }

    static constexpr size_t kMaxTools = 8;

private:
    std::vector<StandaloneToolRunner*> m_runners;
    ToolEcosystemConfig m_config;
};

class ToolOrchestrator {
public:
    ToolOrchestrator() {
        m_atlasAI.setName("AtlasAI");
        m_atlasAI.setExecutablePath("Atlas/Workspace/AtlasAI/atlas_ai_cli.py");
        m_contractScanner.setName("ContractScanner");
        m_contractScanner.setExecutablePath("tools/contract_scanner");
        m_replayMinimizer.setName("ReplayMinimizer");
        m_replayMinimizer.setExecutablePath("tools/replay_minimizer");
    }

    bool startAll() {
        bool ok = true;
        ok &= m_atlasAI.start();
        ok &= m_contractScanner.start();
        ok &= m_replayMinimizer.start();
        return ok;
    }

    bool stopAll() {
        bool ok = true;
        ok &= m_atlasAI.stop();
        ok &= m_contractScanner.stop();
        ok &= m_replayMinimizer.stop();
        return ok;
    }

    StandaloneToolRunner* runner(const std::string& name) {
        if (name == "AtlasAI") return &m_atlasAI;
        if (name == "ContractScanner") return &m_contractScanner;
        if (name == "ReplayMinimizer") return &m_replayMinimizer;
        return nullptr;
    }

    const StandaloneToolRunner* runner(const std::string& name) const {
        if (name == "AtlasAI") return &m_atlasAI;
        if (name == "ContractScanner") return &m_contractScanner;
        if (name == "ReplayMinimizer") return &m_replayMinimizer;
        return nullptr;
    }

    [[nodiscard]] size_t runningCount() const {
        size_t c = 0;
        if (m_atlasAI.isAlive()) ++c;
        if (m_contractScanner.isAlive()) ++c;
        if (m_replayMinimizer.isAlive()) ++c;
        return c;
    }

    [[nodiscard]] size_t totalEventsHandled() const {
        return m_atlasAI.eventsHandled() +
               m_contractScanner.eventsHandled() + m_replayMinimizer.eventsHandled();
    }

    void tickAll(float dt) {
        m_atlasAI.tickUptime(dt);
        m_contractScanner.tickUptime(dt);
        m_replayMinimizer.tickUptime(dt);
    }

    static constexpr size_t kToolCount = 3;

private:
    StandaloneToolRunner m_atlasAI;
    StandaloneToolRunner m_contractScanner;
    StandaloneToolRunner m_replayMinimizer;
};

class ToolEcosystem {
public:
    void init(const ToolEcosystemConfig& config = {}) {
        m_config = config;
        m_monitor.setConfig(config);
        m_monitor.addRunner(m_orchestrator.runner("AtlasAI"));
        m_monitor.addRunner(m_orchestrator.runner("ContractScanner"));
        m_monitor.addRunner(m_orchestrator.runner("ReplayMinimizer"));
        m_initialized = true;
    }

    void shutdown() {
        m_orchestrator.stopAll();
        m_initialized = false;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    bool startAll() { return m_orchestrator.startAll(); }
    bool stopAll() { return m_orchestrator.stopAll(); }

    void tick(float dt) {
        m_orchestrator.tickAll(dt);
        m_monitor.checkHealth();
        m_tickCount++;

        if (m_config.autoRestart) {
            autoRestartCrashed();
        }
    }

    [[nodiscard]] const ToolOrchestrator& orchestrator() const { return m_orchestrator; }
    [[nodiscard]] ToolOrchestrator& orchestrator() { return m_orchestrator; }
    [[nodiscard]] const ToolHealthMonitor& monitor() const { return m_monitor; }
    [[nodiscard]] const ToolEcosystemConfig& config() const { return m_config; }
    [[nodiscard]] size_t tickCount() const { return m_tickCount; }

    [[nodiscard]] size_t healthyToolCount() const { return m_monitor.healthyCount(); }
    [[nodiscard]] size_t totalEventsHandled() const { return m_orchestrator.totalEventsHandled(); }

private:
    void autoRestartCrashed() {
        for (const char* name : {"AtlasAI", "ContractScanner", "ReplayMinimizer"}) {
            auto* r = m_orchestrator.runner(name);
            if (r && r->status() == ToolStatus::Crashed) {
                r->start();
            }
        }
    }

    ToolEcosystemConfig m_config;
    ToolOrchestrator m_orchestrator;
    ToolHealthMonitor m_monitor;
    bool m_initialized = false;
    size_t m_tickCount = 0;
};

// ── S9 — AtlasAI Integration ────────────────────────────────────


} // namespace NF
