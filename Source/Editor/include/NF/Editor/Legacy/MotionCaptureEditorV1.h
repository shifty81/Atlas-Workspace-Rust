#pragma once
// NF::Editor — Motion capture editor v1: actor, session, and bone mapping authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Mcv1SessionState : uint8_t { Idle, Calibrating, Recording, Review, Baked };
enum class Mcv1ActorRole    : uint8_t { Lead, Supporting, Background, Prop, Camera };
enum class Mcv1StreamSource : uint8_t { Optical, Inertial, Hybrid, Manual };

inline const char* mcv1SessionStateName(Mcv1SessionState s) {
    switch (s) {
        case Mcv1SessionState::Idle:        return "Idle";
        case Mcv1SessionState::Calibrating: return "Calibrating";
        case Mcv1SessionState::Recording:   return "Recording";
        case Mcv1SessionState::Review:      return "Review";
        case Mcv1SessionState::Baked:       return "Baked";
    }
    return "Unknown";
}

inline const char* mcv1ActorRoleName(Mcv1ActorRole r) {
    switch (r) {
        case Mcv1ActorRole::Lead:       return "Lead";
        case Mcv1ActorRole::Supporting: return "Supporting";
        case Mcv1ActorRole::Background: return "Background";
        case Mcv1ActorRole::Prop:       return "Prop";
        case Mcv1ActorRole::Camera:     return "Camera";
    }
    return "Unknown";
}

inline const char* mcv1StreamSourceName(Mcv1StreamSource s) {
    switch (s) {
        case Mcv1StreamSource::Optical:  return "Optical";
        case Mcv1StreamSource::Inertial: return "Inertial";
        case Mcv1StreamSource::Hybrid:   return "Hybrid";
        case Mcv1StreamSource::Manual:   return "Manual";
    }
    return "Unknown";
}

struct Mcv1BoneMapping {
    std::string sourceBone;
    std::string targetBone;
    float       weight = 1.f;
    bool        enabled = true;

    [[nodiscard]] bool isValid() const { return !sourceBone.empty() && !targetBone.empty(); }
};

struct Mcv1Actor {
    uint64_t          id           = 0;
    std::string       name;
    Mcv1ActorRole     role         = Mcv1ActorRole::Lead;
    Mcv1SessionState  state        = Mcv1SessionState::Idle;
    Mcv1StreamSource  streamSource = Mcv1StreamSource::Optical;
    std::string       rigAsset;
    std::vector<Mcv1BoneMapping> boneMappings;
    bool              isCalibrated = false;

    [[nodiscard]] bool isValid()     const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isRecording() const { return state == Mcv1SessionState::Recording; }
    [[nodiscard]] bool isBaked()     const { return state == Mcv1SessionState::Baked; }

    bool addBoneMapping(const Mcv1BoneMapping& bm) {
        if (!bm.isValid()) return false;
        for (const auto& m : boneMappings)
            if (m.sourceBone == bm.sourceBone) return false;
        boneMappings.push_back(bm);
        return true;
    }

    bool removeBoneMapping(const std::string& sourceBone) {
        for (auto it = boneMappings.begin(); it != boneMappings.end(); ++it) {
            if (it->sourceBone == sourceBone) { boneMappings.erase(it); return true; }
        }
        return false;
    }
};

using Mcv1ChangeCallback = std::function<void(uint64_t)>;

class MotionCaptureEditorV1 {
public:
    static constexpr size_t MAX_ACTORS = 64;

    bool addActor(const Mcv1Actor& actor) {
        if (!actor.isValid()) return false;
        for (const auto& a : m_actors) if (a.id == actor.id) return false;
        if (m_actors.size() >= MAX_ACTORS) return false;
        m_actors.push_back(actor);
        return true;
    }

    bool removeActor(uint64_t id) {
        for (auto it = m_actors.begin(); it != m_actors.end(); ++it) {
            if (it->id == id) { m_actors.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Mcv1Actor* findActor(uint64_t id) {
        for (auto& a : m_actors) if (a.id == id) return &a;
        return nullptr;
    }

    bool setState(uint64_t id, Mcv1SessionState state) {
        auto* a = findActor(id);
        if (!a) return false;
        a->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setCalibrated(uint64_t id, bool calibrated) {
        auto* a = findActor(id);
        if (!a) return false;
        a->isCalibrated = calibrated;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool addBoneMapping(uint64_t actorId, const Mcv1BoneMapping& bm) {
        auto* a = findActor(actorId);
        return a && a->addBoneMapping(bm);
    }

    bool removeBoneMapping(uint64_t actorId, const std::string& sourceBone) {
        auto* a = findActor(actorId);
        return a && a->removeBoneMapping(sourceBone);
    }

    [[nodiscard]] size_t actorCount()      const { return m_actors.size(); }
    [[nodiscard]] size_t recordingCount()  const {
        size_t c = 0; for (const auto& a : m_actors) if (a.isRecording()) ++c; return c;
    }
    [[nodiscard]] size_t bakedCount()      const {
        size_t c = 0; for (const auto& a : m_actors) if (a.isBaked())     ++c; return c;
    }
    [[nodiscard]] size_t calibratedCount() const {
        size_t c = 0; for (const auto& a : m_actors) if (a.isCalibrated)  ++c; return c;
    }
    [[nodiscard]] size_t countByRole(Mcv1ActorRole role) const {
        size_t c = 0; for (const auto& a : m_actors) if (a.role == role) ++c; return c;
    }

    void setOnChange(Mcv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Mcv1Actor> m_actors;
    Mcv1ChangeCallback     m_onChange;
};

} // namespace NF
