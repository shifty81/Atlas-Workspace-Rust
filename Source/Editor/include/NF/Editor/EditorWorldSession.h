#pragma once
// NF::Editor — Editor world session + play-in-editor
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

enum class PlayState : uint8_t {
    Stopped = 0,
    Running,
    Paused
};

inline const char* playStateName(PlayState s) {
    switch (s) {
        case PlayState::Stopped: return "Stopped";
        case PlayState::Running: return "Running";
        case PlayState::Paused:  return "Paused";
    }
    return "Unknown";
}

/// Snapshot of the world state captured before Play-in-Editor starts,
/// so that stop restores everything.
struct EditorWorldSnapshot {
    std::string worldPath;
    std::vector<PlacedEntity> placedEntities;
    NoiseParams pcgParams;
    Vec3 cameraPosition;
    float cameraYaw   = 0.f;
    float cameraPitch  = 0.f;
    bool valid = false;

    void capture(const std::string& path,
                 const std::vector<PlacedEntity>& entities,
                 const NoiseParams& params,
                 const Vec3& camPos, float yaw, float pitch) {
        worldPath       = path;
        placedEntities  = entities;
        pcgParams       = params;
        cameraPosition  = camPos;
        cameraYaw       = yaw;
        cameraPitch     = pitch;
        valid           = true;
    }

    void invalidate() { valid = false; }
};

/// Manages an in-editor play session (PIE).
///
/// Lifecycle:
///   1. User presses Play → snapshot captured, state → Running
///   2. User presses Pause → state → Paused (time frozen)
///   3. User presses Stop → state → Stopped, world restored from snapshot
class EditorWorldSession {
public:
    [[nodiscard]] PlayState state() const { return m_state; }
    [[nodiscard]] float elapsedTime() const { return m_elapsed; }
    [[nodiscard]] uint64_t frameCount() const { return m_frames; }
    [[nodiscard]] const EditorWorldSnapshot& snapshot() const { return m_snapshot; }
    [[nodiscard]] bool hasSnapshot() const { return m_snapshot.valid; }

    /// Begin a play session — captures the current snapshot.
    bool start(const std::string& worldPath,
               const std::vector<PlacedEntity>& entities,
               const NoiseParams& params,
               const Vec3& camPos, float yaw, float pitch) {
        if (m_state != PlayState::Stopped) return false;
        m_snapshot.capture(worldPath, entities, params, camPos, yaw, pitch);
        m_state   = PlayState::Running;
        m_elapsed = 0.f;
        m_frames  = 0;
        return true;
    }

    bool pause() {
        if (m_state != PlayState::Running) return false;
        m_state = PlayState::Paused;
        return true;
    }

    bool resume() {
        if (m_state != PlayState::Paused) return false;
        m_state = PlayState::Running;
        return true;
    }

    bool stop() {
        if (m_state == PlayState::Stopped) return false;
        m_state = PlayState::Stopped;
        // Snapshot remains valid for restoration
        return true;
    }

    void tick(float dt) {
        if (m_state == PlayState::Running) {
            m_elapsed += dt;
            ++m_frames;
        }
    }

private:
    PlayState m_state = PlayState::Stopped;
    float m_elapsed   = 0.f;
    uint64_t m_frames = 0;
    EditorWorldSnapshot m_snapshot;
};

/// High-level Play-in-Editor controller that integrates with the rest of
/// the editor — captures/restores entity placement, PCG params, camera.
class PlayInEditorSystem {
public:
    PlayInEditorSystem() = default;

    explicit PlayInEditorSystem(EntityPlacementTool* placement,
                                PCGTuningPanel* pcg,
                                ViewportPanel* viewport)
        : m_placement(placement), m_pcg(pcg), m_viewport(viewport) {}

    void setPlacementTool(EntityPlacementTool* t) { m_placement = t; }
    void setPCGTuningPanel(PCGTuningPanel* p) { m_pcg = p; }
    void setViewportPanel(ViewportPanel* v) { m_viewport = v; }

    [[nodiscard]] PlayState state() const { return m_session.state(); }
    [[nodiscard]] float elapsedTime() const { return m_session.elapsedTime(); }
    [[nodiscard]] uint64_t frameCount() const { return m_session.frameCount(); }
    [[nodiscard]] const EditorWorldSession& session() const { return m_session; }

    [[nodiscard]] bool isRunning() const { return m_session.state() == PlayState::Running; }
    [[nodiscard]] bool isPaused()  const { return m_session.state() == PlayState::Paused; }
    [[nodiscard]] bool isStopped() const { return m_session.state() == PlayState::Stopped; }

    /// Start play — snapshots the current editor state.
    bool start(const std::string& worldPath = "") {
        // Gather current state
        std::vector<PlacedEntity> entities;
        NoiseParams params;
        Vec3 camPos{};
        float yaw = 0.f, pitch = 0.f;

        if (m_placement)
            entities = m_placement->placedEntities();
        if (m_pcg)
            params = m_pcg->noiseParams();
        if (m_viewport) {
            camPos = m_viewport->cameraPosition();
            yaw    = m_viewport->cameraYaw();
            pitch  = m_viewport->cameraPitch();
        }

        bool ok = m_session.start(worldPath, entities, params, camPos, yaw, pitch);
        if (ok) {
            NF_LOG_INFO("PIE", "Play-in-Editor started");
        }
        return ok;
    }

    bool pause() {
        bool ok = m_session.pause();
        if (ok) NF_LOG_INFO("PIE", "Play-in-Editor paused");
        return ok;
    }

    bool resume() {
        bool ok = m_session.resume();
        if (ok) NF_LOG_INFO("PIE", "Play-in-Editor resumed");
        return ok;
    }

    /// Stop play — restores the pre-play snapshot to the editor state.
    bool stop() {
        bool ok = m_session.stop();
        if (!ok) return false;

        // Restore from snapshot
        const auto& snap = m_session.snapshot();
        if (snap.valid) {
            if (m_placement) {
                m_placement->clear();
                for (auto& e : snap.placedEntities)
                    m_placement->addEntity(e);
            }
            if (m_pcg)
                m_pcg->setNoiseParams(snap.pcgParams);
            if (m_viewport) {
                m_viewport->setCameraPosition(snap.cameraPosition);
                m_viewport->setCameraYaw(snap.cameraYaw);
                m_viewport->setCameraPitch(snap.cameraPitch);
            }
        }
        NF_LOG_INFO("PIE", "Play-in-Editor stopped — world restored");
        return true;
    }

    /// Toggle: Start/Resume if stopped/paused, Pause if running.
    void togglePlay() {
        switch (m_session.state()) {
            case PlayState::Stopped: start(); break;
            case PlayState::Running: pause(); break;
            case PlayState::Paused:  resume(); break;
        }
    }

    void tick(float dt) { m_session.tick(dt); }

private:
    EditorWorldSession m_session;
    EntityPlacementTool* m_placement = nullptr;
    PCGTuningPanel*      m_pcg       = nullptr;
    ViewportPanel*       m_viewport  = nullptr;
};

// ── M4/S3 Asset Pipeline ──────────────────────────────────────────

/// 128-bit asset GUID for stable references across renames and moves.

} // namespace NF
