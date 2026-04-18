#include "../engine/ui/HUDOverlay.h"
#include <iostream>
#include <cassert>

using namespace atlas::ui;

void test_hud_defaults() {
    HUDOverlay hud;
    assert(hud.GetCurrentTick() == 0);
    assert(hud.GetSimulationTime() == 0.0);
    assert(hud.GetTickRate() == 30);
    assert(hud.WarningCount() == 0);
    assert(!hud.IsPaused());
    assert(hud.GetSpeed() == 1.0);
    assert(hud.GetStateHash() == 0);
    assert(hud.IsDeterminismOk());
    assert(hud.IsVisible());
    std::cout << "[PASS] test_hud_defaults" << std::endl;
}

void test_hud_tick_display() {
    HUDOverlay hud;
    hud.SetCurrentTick(120);
    assert(hud.GetCurrentTick() == 120);

    hud.SetSimulationTime(4.0);
    assert(hud.GetSimulationTime() == 4.0);

    hud.SetTickRate(60);
    assert(hud.GetTickRate() == 60);
    std::cout << "[PASS] test_hud_tick_display" << std::endl;
}

void test_hud_warnings() {
    HUDOverlay hud;
    hud.SetSimulationTime(1.0);
    hud.AddWarning("Hash mismatch", WarningLevel::Warning, 10);
    hud.AddWarning("Info msg", WarningLevel::Info, 11);
    assert(hud.WarningCount() == 2);

    const auto& warnings = hud.ActiveWarnings();
    assert(warnings[0].message == "Hash mismatch");
    assert(warnings[0].level == WarningLevel::Warning);
    assert(warnings[0].tick == 10);
    assert(warnings[0].timestamp == 1.0);
    assert(warnings[1].message == "Info msg");

    hud.ClearWarnings();
    assert(hud.WarningCount() == 0);

    // Test max warnings cap
    hud.SetMaxWarnings(3);
    hud.AddWarning("w1", WarningLevel::Info, 1);
    hud.AddWarning("w2", WarningLevel::Info, 2);
    hud.AddWarning("w3", WarningLevel::Info, 3);
    hud.AddWarning("w4", WarningLevel::Info, 4);
    assert(hud.WarningCount() == 3);
    assert(hud.ActiveWarnings()[0].message == "w2");
    std::cout << "[PASS] test_hud_warnings" << std::endl;
}

void test_hud_critical_warning() {
    HUDOverlay hud;
    assert(!hud.HasCriticalWarning());

    hud.AddWarning("Minor issue", WarningLevel::Info, 1);
    assert(!hud.HasCriticalWarning());

    hud.AddWarning("Desync detected", WarningLevel::Critical, 5);
    assert(hud.HasCriticalWarning());
    std::cout << "[PASS] test_hud_critical_warning" << std::endl;
}

void test_hud_time_controls() {
    HUDOverlay hud;
    assert(!hud.IsPaused());
    assert(hud.GetSpeed() == 1.0);

    hud.SetPaused(true);
    assert(hud.IsPaused());

    hud.SetSpeed(2.0);
    assert(hud.GetSpeed() == 2.0);

    hud.SetSpeed(0.5);
    assert(hud.GetSpeed() == 0.5);

    // Negative speed clamped to 0
    hud.SetSpeed(-1.0);
    assert(hud.GetSpeed() == 0.0);

    const auto& tc = hud.GetTimeControl();
    assert(tc.paused == true);
    assert(tc.speed == 0.0);
    std::cout << "[PASS] test_hud_time_controls" << std::endl;
}

void test_hud_step_forward() {
    HUDOverlay hud;
    hud.SetCurrentTick(50);

    hud.StepForward();
    assert(hud.IsPaused());
    assert(hud.GetTimeControl().targetTick == 51);

    // Step backward
    hud.SetCurrentTick(50);
    hud.StepBackward();
    assert(hud.GetTimeControl().targetTick == 49);

    // Step backward at tick 0 stays at 0
    hud.SetCurrentTick(0);
    hud.StepBackward();
    assert(hud.GetTimeControl().targetTick == 0);
    std::cout << "[PASS] test_hud_step_forward" << std::endl;
}

void test_hud_determinism_status() {
    HUDOverlay hud;
    assert(hud.IsDeterminismOk());
    assert(hud.GetStateHash() == 0);

    hud.SetStateHash(0xDEADBEEF);
    assert(hud.GetStateHash() == 0xDEADBEEF);

    hud.SetDeterminismStatus(false);
    assert(!hud.IsDeterminismOk());

    hud.SetDeterminismStatus(true);
    assert(hud.IsDeterminismOk());
    std::cout << "[PASS] test_hud_determinism_status" << std::endl;
}

void test_hud_visibility() {
    HUDOverlay hud;
    assert(hud.IsVisible());

    hud.SetVisible(false);
    assert(!hud.IsVisible());

    hud.SetVisible(true);
    assert(hud.IsVisible());
    std::cout << "[PASS] test_hud_visibility" << std::endl;
}

void test_hud_world_dilation() {
    HUDOverlay hud;
    assert(hud.GetWorldDilation() == 1.0);

    hud.SetWorldDilation(2.0);
    assert(hud.GetWorldDilation() == 2.0);

    hud.SetWorldDilation(0.5);
    assert(hud.GetWorldDilation() == 0.5);

    // Negative dilation clamped to 0
    hud.SetWorldDilation(-1.0);
    assert(hud.GetWorldDilation() == 0.0);

    std::cout << "[PASS] test_hud_world_dilation" << std::endl;
}

void test_hud_world_paused() {
    HUDOverlay hud;
    assert(!hud.IsWorldPaused());

    hud.SetWorldPaused(true);
    assert(hud.IsWorldPaused());

    hud.SetWorldPaused(false);
    assert(!hud.IsWorldPaused());

    std::cout << "[PASS] test_hud_world_paused" << std::endl;
}

