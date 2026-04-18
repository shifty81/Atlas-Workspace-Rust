#pragma once
// NF::PIEGameSceneProvider — Game scene provider for Play-In-Editor.
//
// Implements IViewportSceneProvider for the PIE runtime session.
// During a PIE session the workspace replaces the editor scene provider
// with this provider so the viewport renders game-simulation content
// rather than the static editor preview.
//
// Game world state:
//   - Entities have positions that advance each tick (simple kinematics).
//   - Each entity carries a velocity applied per tickGame() call.
//   - The provider is "active" only between enterGame() and exitGame().
//   - While paused (pauseGame()) provideScene() still returns the last
//     snapshot so the user can inspect the frozen game frame.
//
// Phase 73 — PIE Viewport Full Integration

#include "NF/Workspace/IViewportSceneProvider.h"
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

// ── PIEGameEntity ──────────────────────────────────────────────────────────

/// A single simulated game entity tracked by PIEGameSceneProvider.
struct PIEGameEntity {
    uint32_t    id         = 0;
    std::string name;
    float       x = 0.f, y = 0.f, z = 0.f;     ///< world position
    float       vx = 0.f, vy = 0.f, vz = 0.f;   ///< velocity (units/s)
    float       halfExtent = 0.5f;               ///< bounding box half-size
    bool        selected   = false;
};

// ── PIEGameSceneMode ───────────────────────────────────────────────────────

enum class PIEGameSceneMode : uint8_t {
    Stopped,  ///< No game session; provideScene() returns empty state
    Playing,  ///< Simulation running; tickGame() advances positions
    Paused,   ///< Simulation suspended; last snapshot is held
};

inline const char* pieGameSceneModeName(PIEGameSceneMode m) {
    switch (m) {
    case PIEGameSceneMode::Stopped: return "Stopped";
    case PIEGameSceneMode::Playing: return "Playing";
    case PIEGameSceneMode::Paused:  return "Paused";
    }
    return "Unknown";
}

// ── PIEGameSceneProvider ───────────────────────────────────────────────────

class PIEGameSceneProvider final : public IViewportSceneProvider {
public:
    static constexpr uint32_t kMaxEntities = 64;

    // ── IViewportSceneProvider ─────────────────────────────────────────────

    ViewportSceneState provideScene(ViewportHandle       /*handle*/,
                                    const ViewportSlot&  /*slot*/) override {
        ViewportSceneState st;
        if (m_mode == PIEGameSceneMode::Stopped) return st;

        st.hasContent  = !m_entities.empty();
        st.entityCount = static_cast<uint32_t>(m_entities.size());
        // In PIE the game camera is not managed here — the slot camera
        // is set externally by the PIEViewportBridge.
        st.overrideCamera = false;
        st.clearColor     = m_clearColor;

        st.entities.reserve(m_entities.size());
        for (const auto& e : m_entities) {
            ViewportEntityProxy proxy;
            proxy.x        = e.x;
            proxy.y        = e.y;
            proxy.z        = e.z;
            proxy.halfW    = e.halfExtent;
            proxy.halfH    = e.halfExtent;
            proxy.halfD    = e.halfExtent;
            proxy.selected = e.selected;
            proxy.label    = e.name.c_str();
            st.entities.push_back(proxy);
        }
        return st;
    }

    // ── Game entity management ─────────────────────────────────────────────

    /// Add a game entity.  Returns false if at capacity or id is duplicate.
    bool addEntity(const PIEGameEntity& entity) {
        if (m_entities.size() >= kMaxEntities) return false;
        for (const auto& e : m_entities)
            if (e.id == entity.id) return false;
        m_entities.push_back(entity);
        return true;
    }

    /// Remove an entity by id.
    bool removeEntity(uint32_t id) {
        for (auto it = m_entities.begin(); it != m_entities.end(); ++it) {
            if (it->id == id) { m_entities.erase(it); return true; }
        }
        return false;
    }

    void clearEntities() { m_entities.clear(); }

    [[nodiscard]] uint32_t entityCount() const {
        return static_cast<uint32_t>(m_entities.size());
    }

    [[nodiscard]] const std::vector<PIEGameEntity>& entities() const {
        return m_entities;
    }

    /// Set the selected entity id (kInvalidHandle = no selection).
    void setSelectedId(uint32_t id) {
        for (auto& e : m_entities)
            e.selected = (e.id == id);
    }

    // ── Lifecycle ──────────────────────────────────────────────────────────

    /// Enter game mode.  Resets tick counter; retains existing entities.
    /// Returns false if already in Playing or Paused.
    bool enterGame() {
        if (m_mode != PIEGameSceneMode::Stopped) return false;
        m_tickCount = 0;
        m_mode      = PIEGameSceneMode::Playing;
        return true;
    }

    /// Exit game mode and clear simulation state.
    /// Returns false if already Stopped.
    bool exitGame() {
        if (m_mode == PIEGameSceneMode::Stopped) return false;
        m_mode = PIEGameSceneMode::Stopped;
        return true;
    }

    /// Pause the simulation.  Returns false if not Playing.
    bool pauseGame() {
        if (m_mode != PIEGameSceneMode::Playing) return false;
        m_mode = PIEGameSceneMode::Paused;
        return true;
    }

    /// Resume from pause.  Returns false if not Paused.
    bool resumeGame() {
        if (m_mode != PIEGameSceneMode::Paused) return false;
        m_mode = PIEGameSceneMode::Playing;
        return true;
    }

    /// Advance all entity positions by their velocity × dt.
    /// No-op if not in Playing mode.
    void tickGame(float dt) {
        if (m_mode != PIEGameSceneMode::Playing) return;
        for (auto& e : m_entities) {
            e.x += e.vx * dt;
            e.y += e.vy * dt;
            e.z += e.vz * dt;
        }
        ++m_tickCount;
    }

    // ── State queries ──────────────────────────────────────────────────────

    [[nodiscard]] PIEGameSceneMode mode()      const { return m_mode; }
    [[nodiscard]] bool isPlaying()             const { return m_mode == PIEGameSceneMode::Playing; }
    [[nodiscard]] bool isPaused()              const { return m_mode == PIEGameSceneMode::Paused; }
    [[nodiscard]] bool isStopped()             const { return m_mode == PIEGameSceneMode::Stopped; }
    [[nodiscard]] uint32_t tickCount()         const { return m_tickCount; }

    /// Background clear colour (RRGGBBAA).
    void setClearColor(uint32_t c) { m_clearColor = c; }
    [[nodiscard]] uint32_t clearColor() const { return m_clearColor; }

private:
    PIEGameSceneMode          m_mode       = PIEGameSceneMode::Stopped;
    uint32_t                  m_tickCount  = 0;
    uint32_t                  m_clearColor = 0x0D1B2AFFu; ///< dark-navy game sky
    std::vector<PIEGameEntity> m_entities;
};

} // namespace NF
