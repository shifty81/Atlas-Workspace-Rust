#pragma once
// NF::Editor — Water simulation editor v1: water body and wave management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Wsv1BodyType  : uint8_t { Ocean, Lake, River, Pool, Waterfall };
enum class Wsv1SimState  : uint8_t { Idle, Active, Paused, Error, Disabled };

inline const char* wsv1BodyTypeName(Wsv1BodyType t) {
    switch (t) {
        case Wsv1BodyType::Ocean:     return "Ocean";
        case Wsv1BodyType::Lake:      return "Lake";
        case Wsv1BodyType::River:     return "River";
        case Wsv1BodyType::Pool:      return "Pool";
        case Wsv1BodyType::Waterfall: return "Waterfall";
    }
    return "Unknown";
}

inline const char* wsv1SimStateName(Wsv1SimState s) {
    switch (s) {
        case Wsv1SimState::Idle:     return "Idle";
        case Wsv1SimState::Active:   return "Active";
        case Wsv1SimState::Paused:   return "Paused";
        case Wsv1SimState::Error:    return "Error";
        case Wsv1SimState::Disabled: return "Disabled";
    }
    return "Unknown";
}

struct Wsv1WaterBody {
    uint64_t     id    = 0;
    std::string  name;
    Wsv1BodyType type  = Wsv1BodyType::Ocean;
    Wsv1SimState state = Wsv1SimState::Idle;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive()   const { return state == Wsv1SimState::Active; }
    [[nodiscard]] bool isPaused()   const { return state == Wsv1SimState::Paused; }
};

struct Wsv1Wave {
    uint64_t id        = 0;
    uint64_t bodyId    = 0;
    float    amplitude = 1.f;
    float    frequency = 1.f;

    [[nodiscard]] bool isValid() const { return id != 0 && bodyId != 0; }
};

using Wsv1ChangeCallback = std::function<void(uint64_t)>;

class WaterSimEditorV1 {
public:
    static constexpr size_t MAX_BODIES = 512;
    static constexpr size_t MAX_WAVES  = 4096;

    bool addBody(const Wsv1WaterBody& body) {
        if (!body.isValid()) return false;
        for (const auto& b : m_bodies) if (b.id == body.id) return false;
        if (m_bodies.size() >= MAX_BODIES) return false;
        m_bodies.push_back(body);
        if (m_onChange) m_onChange(body.id);
        return true;
    }

    bool removeBody(uint64_t id) {
        for (auto it = m_bodies.begin(); it != m_bodies.end(); ++it) {
            if (it->id == id) { m_bodies.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Wsv1WaterBody* findBody(uint64_t id) {
        for (auto& b : m_bodies) if (b.id == id) return &b;
        return nullptr;
    }

    bool addWave(const Wsv1Wave& wave) {
        if (!wave.isValid()) return false;
        for (const auto& w : m_waves) if (w.id == wave.id) return false;
        if (m_waves.size() >= MAX_WAVES) return false;
        m_waves.push_back(wave);
        if (m_onChange) m_onChange(wave.bodyId);
        return true;
    }

    bool removeWave(uint64_t id) {
        for (auto it = m_waves.begin(); it != m_waves.end(); ++it) {
            if (it->id == id) { m_waves.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t bodyCount()  const { return m_bodies.size(); }
    [[nodiscard]] size_t waveCount()  const { return m_waves.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0; for (const auto& b : m_bodies) if (b.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t pausedCount() const {
        size_t c = 0; for (const auto& b : m_bodies) if (b.isPaused()) ++c; return c;
    }
    [[nodiscard]] size_t countByBodyType(Wsv1BodyType type) const {
        size_t c = 0; for (const auto& b : m_bodies) if (b.type == type) ++c; return c;
    }

    void setOnChange(Wsv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Wsv1WaterBody> m_bodies;
    std::vector<Wsv1Wave>      m_waves;
    Wsv1ChangeCallback         m_onChange;
};

} // namespace NF
