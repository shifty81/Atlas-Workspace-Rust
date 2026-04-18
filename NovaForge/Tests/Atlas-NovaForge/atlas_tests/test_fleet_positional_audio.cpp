/**
 * Tests for FleetPositionalAudioSystem — 3D audio source positions
 * from fleet formation and warp tunnel reverb effects.
 */

#include <cassert>
#include <cmath>
#include "../cpp_server/include/systems/fleet_positional_audio_system.h"
#include "../cpp_server/include/components/fleet_components.h"

using namespace atlas::systems;
using namespace atlas::components;

static bool approxEq(float a, float b, float eps = 0.001f) {
    return std::fabs(a - b) < eps;
}

// ══════════════════════════════════════════════════════════════════
// Component defaults
// ══════════════════════════════════════════════════════════════════

void test_positional_audio_defaults() {
    PositionalAudioSource src;
    assert(approxEq(src.listener_x, 0.0f));
    assert(approxEq(src.source_x, 0.0f));
    assert(approxEq(src.min_range, 5.0f));
    assert(approxEq(src.max_range, 200.0f));
    assert(approxEq(src.attenuation, 1.0f));
    assert(approxEq(src.reverb_wet_mix, 0.0f));
    assert(approxEq(src.reverb_decay, 0.0f));
}

// ══════════════════════════════════════════════════════════════════
// computeAudioPosition
// ══════════════════════════════════════════════════════════════════

void test_positional_audio_position_origin() {
    float ox, oy, oz;
    FleetPositionalAudioSystem::computeAudioPosition(
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        ox, oy, oz);
    assert(approxEq(ox, 0.0f));
    assert(approxEq(oy, 0.0f));
    assert(approxEq(oz, 0.0f));
}

void test_positional_audio_position_offset() {
    float ox, oy, oz;
    FleetPositionalAudioSystem::computeAudioPosition(
        100.0f, 200.0f, 300.0f,
        10.0f, -5.0f, 20.0f,
        ox, oy, oz);
    assert(approxEq(ox, 110.0f));
    assert(approxEq(oy, 195.0f));
    assert(approxEq(oz, 320.0f));
}

// ══════════════════════════════════════════════════════════════════
// computeAttenuation
// ══════════════════════════════════════════════════════════════════

void test_positional_audio_atten_within_min() {
    float a = FleetPositionalAudioSystem::computeAttenuation(3.0f, 5.0f, 200.0f);
    assert(approxEq(a, 1.0f));
}

void test_positional_audio_atten_at_min() {
    float a = FleetPositionalAudioSystem::computeAttenuation(5.0f, 5.0f, 200.0f);
    assert(approxEq(a, 1.0f));
}

void test_positional_audio_atten_beyond_max() {
    float a = FleetPositionalAudioSystem::computeAttenuation(250.0f, 5.0f, 200.0f);
    assert(approxEq(a, 0.0f));
}

void test_positional_audio_atten_midpoint() {
    // At midpoint between 0 and 200 (min=0, max=200): distance 100 → 0.5
    float a = FleetPositionalAudioSystem::computeAttenuation(100.0f, 0.0f, 200.0f);
    assert(approxEq(a, 0.5f));
}

void test_positional_audio_atten_invalid_range() {
    // max <= min → 0.0
    float a = FleetPositionalAudioSystem::computeAttenuation(10.0f, 200.0f, 5.0f);
    assert(approxEq(a, 0.0f));
}

// ══════════════════════════════════════════════════════════════════
// computeWarpReverb
// ══════════════════════════════════════════════════════════════════

void test_positional_audio_reverb_not_in_warp() {
    float wet, decay;
    FleetPositionalAudioSystem::computeWarpReverb(false, 5.0f, wet, decay);
    assert(approxEq(wet, 0.0f));
    assert(approxEq(decay, 0.0f));
}

void test_positional_audio_reverb_in_warp_low_speed() {
    float wet, decay;
    FleetPositionalAudioSystem::computeWarpReverb(true, 0.0f, wet, decay);
    // speed_factor = 0 → base values
    assert(approxEq(wet, 0.3f));
    assert(approxEq(decay, 1.5f));
}

void test_positional_audio_reverb_in_warp_high_speed() {
    float wet, decay;
    FleetPositionalAudioSystem::computeWarpReverb(true, 10.0f, wet, decay);
    // speed_factor = 1.0 → max values
    assert(approxEq(wet, 0.6f));
    assert(approxEq(decay, 2.5f));
}

void test_positional_audio_reverb_in_warp_mid_speed() {
    float wet, decay;
    FleetPositionalAudioSystem::computeWarpReverb(true, 5.0f, wet, decay);
    // speed_factor = 0.5 → mid values
    assert(approxEq(wet, 0.45f));
    assert(approxEq(decay, 2.0f));
}

void test_positional_audio_reverb_clamped_high() {
    float wet, decay;
    FleetPositionalAudioSystem::computeWarpReverb(true, 100.0f, wet, decay);
    // speed_factor clamped to 1.0
    assert(approxEq(wet, 0.6f));
    assert(approxEq(decay, 2.5f));
}
