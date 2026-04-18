#pragma once
// NF::Editor — AI spawn editor v1: spawn point and wave configuration authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Asv1SpawnTrigger: uint8_t { OnStart, OnTrigger, OnTimer, OnDeath, Manual };
enum class Asv1TeamSide    : uint8_t { Neutral, Friendly, Enemy, Player };

struct Asv1SpawnPoint {
    uint64_t        id      = 0;
    std::string     name;
    float           x       = 0.f;
    float           y       = 0.f;
    float           z       = 0.f;
    Asv1TeamSide    team    = Asv1TeamSide::Neutral;
    bool            enabled = true;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

struct Asv1Wave {
    uint64_t          id        = 0;
    std::string       label;
    Asv1SpawnTrigger  trigger   = Asv1SpawnTrigger::OnStart;
    float             delay     = 0.f;
    uint32_t          count     = 1;
    uint64_t          spawnPointId = 0;
    [[nodiscard]] bool isValid() const { return id != 0 && !label.empty() && count > 0; }
};

using Asv1SpawnCallback = std::function<void(const Asv1Wave&)>;

class AISpawnEditorV1 {
public:
    bool addSpawnPoint(const Asv1SpawnPoint& sp) {
        if (!sp.isValid()) return false;
        for (const auto& ep : m_points) if (ep.id == sp.id) return false;
        m_points.push_back(sp);
        return true;
    }

    bool removeSpawnPoint(uint64_t id) {
        for (auto it = m_points.begin(); it != m_points.end(); ++it) {
            if (it->id == id) { m_points.erase(it); return true; }
        }
        return false;
    }

    bool addWave(const Asv1Wave& w) {
        if (!w.isValid()) return false;
        for (const auto& ew : m_waves) if (ew.id == w.id) return false;
        m_waves.push_back(w);
        return true;
    }

    bool removeWave(uint64_t id) {
        for (auto it = m_waves.begin(); it != m_waves.end(); ++it) {
            if (it->id == id) { m_waves.erase(it); return true; }
        }
        return false;
    }

    bool triggerWave(uint64_t id) {
        for (const auto& w : m_waves) {
            if (w.id == id) {
                if (m_onSpawn) m_onSpawn(w);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] size_t spawnPointCount() const { return m_points.size(); }
    [[nodiscard]] size_t waveCount()       const { return m_waves.size();  }

    void setOnSpawn(Asv1SpawnCallback cb) { m_onSpawn = std::move(cb); }

private:
    std::vector<Asv1SpawnPoint> m_points;
    std::vector<Asv1Wave>       m_waves;
    Asv1SpawnCallback           m_onSpawn;
};

} // namespace NF
