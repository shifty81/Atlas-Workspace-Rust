#include "TimeModel.h"

namespace atlas::sim {

void TimeModel::SetTickRate(uint32_t hz) {
    m_ctx.sim.tickRate = hz > 0 ? hz : 1;
    m_ctx.sim.fixedDeltaTime = 1.0f / static_cast<float>(m_ctx.sim.tickRate);
}

uint32_t TimeModel::TickRate() const {
    return m_ctx.sim.tickRate;
}

void TimeModel::AdvanceTick() {
    m_ctx.sim.tick++;
    m_ctx.world.Advance(m_ctx.sim.fixedDeltaTime);
}

void TimeModel::SetWorldDilation(float dilation) {
    m_ctx.world.dilation = dilation >= 0.0f ? dilation : 0.0f;
}

float TimeModel::WorldDilation() const {
    return m_ctx.world.dilation;
}

void TimeModel::SetWorldPaused(bool paused) {
    m_ctx.world.paused = paused;
}

bool TimeModel::IsWorldPaused() const {
    return m_ctx.world.paused;
}

void TimeModel::SetPresentationAlpha(float alpha) {
    m_ctx.presentation.interpAlpha = alpha;
}

void TimeModel::SetWallElapsed(double elapsed) {
    m_ctx.presentation.wallElapsed = elapsed;
}

const TimeContext& TimeModel::Context() const {
    return m_ctx;
}

void TimeModel::SetTick(uint64_t tick) {
    m_ctx.sim.tick = tick;
}

void TimeModel::Reset() {
    m_ctx = TimeContext{};
}

}  // namespace atlas::sim
