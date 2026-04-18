#include "../engine/sim/TimeModel.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace atlas::sim;

void test_time_model_defaults() {
    TimeModel tm;
    const auto& ctx = tm.Context();
    assert(ctx.sim.tick == 0);
    assert(ctx.sim.tickRate == 30);
    assert(ctx.world.elapsed == 0.0);
    assert(ctx.world.dilation == 1.0f);
    assert(!ctx.world.paused);
    assert(ctx.presentation.wallElapsed == 0.0);

    std::cout << "[PASS] test_time_model_defaults" << std::endl;
}

void test_time_model_set_tick_rate() {
    TimeModel tm;
    tm.SetTickRate(60);
    assert(tm.TickRate() == 60);
    assert(std::abs(tm.Context().sim.fixedDeltaTime - 1.0f / 60.0f) < 1e-6f);

    // Zero tick rate should clamp to 1
    tm.SetTickRate(0);
    assert(tm.TickRate() == 1);

    std::cout << "[PASS] test_time_model_set_tick_rate" << std::endl;
}

void test_time_model_advance_tick() {
    TimeModel tm;
    tm.SetTickRate(30);

    tm.AdvanceTick();
    assert(tm.Context().sim.tick == 1);
    assert(tm.Context().world.elapsed > 0.0);

    tm.AdvanceTick();
    assert(tm.Context().sim.tick == 2);

    // World time should match expected elapsed
    double expected = 2.0 * (1.0 / 30.0);
    assert(std::abs(tm.Context().world.elapsed - expected) < 1e-6);

    std::cout << "[PASS] test_time_model_advance_tick" << std::endl;
}

void test_time_model_world_dilation() {
    TimeModel tm;
    tm.SetTickRate(30);

    // 2x speed
    tm.SetWorldDilation(2.0f);
    assert(tm.WorldDilation() == 2.0f);

    tm.AdvanceTick();
    double expected = (1.0 / 30.0) * 2.0;
    assert(std::abs(tm.Context().world.elapsed - expected) < 1e-6);

    // Negative dilation clamps to 0
    tm.SetWorldDilation(-1.0f);
    assert(tm.WorldDilation() == 0.0f);

    std::cout << "[PASS] test_time_model_world_dilation" << std::endl;
}

void test_time_model_world_pause() {
    TimeModel tm;
    tm.SetTickRate(30);

    tm.AdvanceTick();
    double elapsed1 = tm.Context().world.elapsed;

    tm.SetWorldPaused(true);
    assert(tm.IsWorldPaused());

    tm.AdvanceTick();
    // Simulation tick advances but world time does not
    assert(tm.Context().sim.tick == 2);
    assert(tm.Context().world.elapsed == elapsed1);

    tm.SetWorldPaused(false);
    tm.AdvanceTick();
    assert(tm.Context().world.elapsed > elapsed1);

    std::cout << "[PASS] test_time_model_world_pause" << std::endl;
}

void test_time_model_presentation() {
    TimeModel tm;
    tm.SetPresentationAlpha(0.5f);
    assert(tm.Context().presentation.interpAlpha == 0.5f);

    tm.SetWallElapsed(3.14);
    assert(tm.Context().presentation.wallElapsed == 3.14);

    std::cout << "[PASS] test_time_model_presentation" << std::endl;
}

void test_time_model_reset() {
    TimeModel tm;
    tm.SetTickRate(60);
    tm.AdvanceTick();
    tm.AdvanceTick();
    tm.SetWorldDilation(2.0f);
    tm.SetPresentationAlpha(0.7f);

    tm.Reset();
    const auto& ctx = tm.Context();
    assert(ctx.sim.tick == 0);
    assert(ctx.world.elapsed == 0.0);
    assert(ctx.presentation.wallElapsed == 0.0);

    std::cout << "[PASS] test_time_model_reset" << std::endl;
}

void test_simulation_time_elapsed() {
    SimulationTime st;
    st.tick = 100;
    st.fixedDeltaTime = 1.0f / 30.0f;
    double expected = 100.0 / 30.0;
    assert(std::abs(st.ElapsedSeconds() - expected) < 1e-6);

    std::cout << "[PASS] test_simulation_time_elapsed" << std::endl;
}
