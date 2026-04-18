#pragma once
// NF::Editor — Particle system editor v1: emitter configuration and simulation
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Psv1EmitterShape : uint8_t { Point, Sphere, Box, Cone, Disc, Mesh };

inline const char* psv1EmitterShapeName(Psv1EmitterShape s) {
    switch(s){
        case Psv1EmitterShape::Point:  return "Point";
        case Psv1EmitterShape::Sphere: return "Sphere";
        case Psv1EmitterShape::Box:    return "Box";
        case Psv1EmitterShape::Cone:   return "Cone";
        case Psv1EmitterShape::Disc:   return "Disc";
        case Psv1EmitterShape::Mesh:   return "Mesh";
    }
    return "Unknown";
}

enum class Psv1SimSpace : uint8_t { World, Local };

struct Psv1Emitter {
    uint32_t         id           = 0;
    Psv1EmitterShape shape        = Psv1EmitterShape::Point;
    Psv1SimSpace     simSpace     = Psv1SimSpace::World;
    float            emissionRate = 10.f;
    float            lifespan     = 2.f;
    float            speed        = 1.f;
    bool             looping      = true;
    bool             active       = false;
    [[nodiscard]] bool isValid() const { return id != 0; }
};

using Psv1EmitCallback = std::function<void(uint32_t)>;

class ParticleSystemEditorV1 {
public:
    bool addEmitter(const Psv1Emitter& em) {
        if (!em.isValid()) return false;
        for (const auto& e : m_emitters) if (e.id == em.id) return false;
        m_emitters.push_back(em);
        return true;
    }

    bool removeEmitter(uint32_t id) {
        for (auto it = m_emitters.begin(); it != m_emitters.end(); ++it) {
            if (it->id == id) { m_emitters.erase(it); return true; }
        }
        return false;
    }

    bool startEmitter(uint32_t id) {
        Psv1Emitter* e = findEmitter_(id);
        if (!e) return false;
        e->active = true;
        if (m_onEmit) m_onEmit(id);
        return true;
    }

    bool stopEmitter(uint32_t id) {
        Psv1Emitter* e = findEmitter_(id);
        if (!e) return false;
        e->active = false;
        return true;
    }

    bool resetEmitter(uint32_t id) {
        Psv1Emitter* e = findEmitter_(id);
        if (!e) return false;
        e->active = false;
        return true;
    }

    void simulate(float /*deltaMs*/) { ++m_simTick; }

    [[nodiscard]] size_t emitterCount() const { return m_emitters.size(); }
    [[nodiscard]] size_t activeCount()  const {
        size_t n = 0;
        for (const auto& e : m_emitters) if (e.active) ++n;
        return n;
    }

    void setOnEmit(Psv1EmitCallback cb) { m_onEmit = std::move(cb); }

private:
    Psv1Emitter* findEmitter_(uint32_t id) {
        for (auto& e : m_emitters) if (e.id == id) return &e;
        return nullptr;
    }

    std::vector<Psv1Emitter> m_emitters;
    Psv1EmitCallback         m_onEmit;
    size_t                   m_simTick = 0;
};

} // namespace NF
