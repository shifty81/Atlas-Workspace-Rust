#pragma once
#include "ITool.h"

namespace atlas::tools {

/// In-client overlay tool for tweaking environment parameters.
///
/// Adjusts gravity, wind direction/strength, atmosphere density, and
/// time-of-day while the game is running — useful for tuning physics
/// and visual effects without restarting.
class EnvironmentControlTool : public ITool {
public:
    std::string Name() const override { return "Environment Control"; }
    void Activate() override;
    void Deactivate() override;
    void Update(float deltaTime) override;
    bool IsActive() const override { return m_active; }

    // Gravity
    void SetGravity(float x, float y, float z);
    float GravityX() const { return m_gravityX; }
    float GravityY() const { return m_gravityY; }
    float GravityZ() const { return m_gravityZ; }

    // Wind
    void SetWind(float x, float y, float z);
    float WindX() const { return m_windX; }
    float WindY() const { return m_windY; }
    float WindZ() const { return m_windZ; }

    // Atmosphere density (0.0 = vacuum, 1.0 = sea-level)
    void SetAtmosphereDensity(float density);
    float AtmosphereDensity() const { return m_atmosphereDensity; }

    // Time of day (0.0–24.0 hours)
    void SetTimeOfDay(float hours);
    float TimeOfDay() const { return m_timeOfDay; }

    float ElapsedTime() const { return m_elapsed; }

private:
    bool m_active = false;
    float m_gravityX = 0.0f, m_gravityY = -9.81f, m_gravityZ = 0.0f;
    float m_windX = 0.0f, m_windY = 0.0f, m_windZ = 0.0f;
    float m_atmosphereDensity = 1.0f;
    float m_timeOfDay = 12.0f;
    float m_elapsed = 0.0f;
};

} // namespace atlas::tools
