#pragma once
// NF::Editor — AI nav mesh editor v1: nav mesh zone authoring and bake control
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Anv1BakeStatus : uint8_t { Idle, Baking, Done, Failed };
enum class Anv1AreaType   : uint8_t { Walkable, NotWalkable, Jump, Swim, Fly };

struct Anv1AgentConfig {
    float radius     = 0.35f;
    float height     = 1.8f;
    float maxSlope   = 45.f;
    float stepHeight = 0.4f;
    [[nodiscard]] bool isValid() const { return radius > 0.f && height > 0.f; }
};

struct Anv1Zone {
    uint64_t      id   = 0;
    std::string   name;
    Anv1AreaType  area = Anv1AreaType::Walkable;
    bool          enabled = true;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Anv1BakeCallback = std::function<void(Anv1BakeStatus)>;

class AINavMeshEditorV1 {
public:
    bool addZone(const Anv1Zone& z) {
        if (!z.isValid()) return false;
        for (const auto& ez : m_zones) if (ez.id == z.id) return false;
        m_zones.push_back(z);
        return true;
    }

    bool removeZone(uint64_t id) {
        for (auto it = m_zones.begin(); it != m_zones.end(); ++it) {
            if (it->id == id) { m_zones.erase(it); return true; }
        }
        return false;
    }

    bool setAgentConfig(const Anv1AgentConfig& cfg) {
        if (!cfg.isValid()) return false;
        m_agent = cfg;
        return true;
    }

    bool startBake() {
        if (m_status == Anv1BakeStatus::Baking) return false;
        m_status = Anv1BakeStatus::Baking;
        if (m_onBake) m_onBake(m_status);
        return true;
    }

    bool finishBake(bool success) {
        if (m_status != Anv1BakeStatus::Baking) return false;
        m_status = success ? Anv1BakeStatus::Done : Anv1BakeStatus::Failed;
        if (m_onBake) m_onBake(m_status);
        return true;
    }

    bool cancelBake() {
        if (m_status != Anv1BakeStatus::Baking) return false;
        m_status = Anv1BakeStatus::Idle;
        if (m_onBake) m_onBake(m_status);
        return true;
    }

    [[nodiscard]] Anv1BakeStatus   bakeStatus()    const { return m_status;       }
    [[nodiscard]] size_t           zoneCount()     const { return m_zones.size(); }
    [[nodiscard]] Anv1AgentConfig  agentConfig()   const { return m_agent;        }

    void setOnBake(Anv1BakeCallback cb) { m_onBake = std::move(cb); }

private:
    std::vector<Anv1Zone> m_zones;
    Anv1AgentConfig       m_agent;
    Anv1BakeStatus        m_status = Anv1BakeStatus::Idle;
    Anv1BakeCallback      m_onBake;
};

} // namespace NF
