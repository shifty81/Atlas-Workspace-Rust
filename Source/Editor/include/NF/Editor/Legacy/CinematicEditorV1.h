#pragma once
// NF::Editor — Cinematic editor v1: cut/shot management and camera bind authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Cev1ShotType     : uint8_t { CutScene, Gameplay, Transition, StingerCut };
enum class Cev1PlaybackMode : uint8_t { Once, Loop, PingPong, Hold };
enum class Cev1EditState    : uint8_t { Draft, Locked, Review, Approved };

inline const char* cev1ShotTypeName(Cev1ShotType t) {
    switch (t) {
        case Cev1ShotType::CutScene:   return "CutScene";
        case Cev1ShotType::Gameplay:   return "Gameplay";
        case Cev1ShotType::Transition: return "Transition";
        case Cev1ShotType::StingerCut: return "StingerCut";
    }
    return "Unknown";
}

inline const char* cev1PlaybackModeName(Cev1PlaybackMode m) {
    switch (m) {
        case Cev1PlaybackMode::Once:     return "Once";
        case Cev1PlaybackMode::Loop:     return "Loop";
        case Cev1PlaybackMode::PingPong: return "PingPong";
        case Cev1PlaybackMode::Hold:     return "Hold";
    }
    return "Unknown";
}

inline const char* cev1EditStateName(Cev1EditState s) {
    switch (s) {
        case Cev1EditState::Draft:    return "Draft";
        case Cev1EditState::Locked:   return "Locked";
        case Cev1EditState::Review:   return "Review";
        case Cev1EditState::Approved: return "Approved";
    }
    return "Unknown";
}

struct Cev1Shot {
    uint64_t         id           = 0;
    std::string      name;
    Cev1ShotType     type         = Cev1ShotType::CutScene;
    Cev1PlaybackMode playbackMode = Cev1PlaybackMode::Once;
    Cev1EditState    editState    = Cev1EditState::Draft;
    float            startTime    = 0.f;
    float            duration     = 1.f;
    std::string      cameraName;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty() && duration > 0.f; }
    [[nodiscard]] bool isApproved() const { return editState == Cev1EditState::Approved; }
    [[nodiscard]] float endTime()   const { return startTime + duration; }
};

using Cev1ChangeCallback = std::function<void(uint64_t)>;

class CinematicEditorV1 {
public:
    static constexpr size_t MAX_SHOTS = 512;

    bool addShot(const Cev1Shot& shot) {
        if (!shot.isValid()) return false;
        for (const auto& s : m_shots) if (s.id == shot.id) return false;
        if (m_shots.size() >= MAX_SHOTS) return false;
        m_shots.push_back(shot);
        return true;
    }

    bool removeShot(uint64_t id) {
        for (auto it = m_shots.begin(); it != m_shots.end(); ++it) {
            if (it->id == id) {
                if (m_activeId == id) m_activeId = 0;
                m_shots.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] Cev1Shot* findShot(uint64_t id) {
        for (auto& s : m_shots) if (s.id == id) return &s;
        return nullptr;
    }

    bool setActive(uint64_t id) {
        for (const auto& s : m_shots) if (s.id == id) { m_activeId = id; return true; }
        return false;
    }

    bool setEditState(uint64_t id, Cev1EditState state) {
        auto* s = findShot(id);
        if (!s) return false;
        s->editState = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setDuration(uint64_t id, float dur) {
        if (dur <= 0.f) return false;
        auto* s = findShot(id);
        if (!s) return false;
        s->duration = dur;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] uint64_t activeId()      const { return m_activeId; }
    [[nodiscard]] size_t   shotCount()     const { return m_shots.size(); }
    [[nodiscard]] size_t   approvedCount() const {
        size_t c = 0; for (const auto& s : m_shots) if (s.isApproved()) ++c; return c;
    }
    [[nodiscard]] size_t   countByType(Cev1ShotType t) const {
        size_t c = 0; for (const auto& s : m_shots) if (s.type == t) ++c; return c;
    }
    [[nodiscard]] float    totalDuration() const {
        float d = 0.f; for (const auto& s : m_shots) d += s.duration; return d;
    }

    void setOnChange(Cev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Cev1Shot> m_shots;
    uint64_t              m_activeId = 0;
    Cev1ChangeCallback    m_onChange;
};

} // namespace NF
