/**
 * Tests for NetworkInterpolationBuffer — client-side entity position
 * smoothing with LERP and velocity-based dead-reckoning.
 */

#include <cassert>
#include <cmath>
#include "../engine/net/NetworkInterpolationBuffer.h"

using namespace atlas::net;

static bool approxEq(float a, float b, float eps = 0.01f) {
    return std::fabs(a - b) < eps;
}

// ══════════════════════════════════════════════════════════════════
// Basic setup and configuration
// ══════════════════════════════════════════════════════════════════

void test_interp_defaults() {
    NetworkInterpolationBuffer buf;
    assert(buf.TrackedEntityCount() == 0);
    assert(buf.BufferDepth() == 8);
    assert(approxEq(buf.MaxExtrapolationTicks(), 3.0f));
}

void test_interp_custom_config() {
    NetworkInterpolationBuffer buf(16, 5.0f);
    assert(buf.BufferDepth() == 16);
    assert(approxEq(buf.MaxExtrapolationTicks(), 5.0f));
}

void test_interp_push_snapshot() {
    NetworkInterpolationBuffer buf;
    EntitySnapshot snap;
    snap.entityId = 1;
    snap.tick = 100;
    snap.posX = 10.0f;
    buf.PushSnapshot(snap);
    assert(buf.TrackedEntityCount() == 1);
    assert(buf.SnapshotCount(1) == 1);
}

void test_interp_push_multiple_entities() {
    NetworkInterpolationBuffer buf;
    EntitySnapshot s1{1, 100, 0, 0, 0, 0, 0, 0, 0};
    EntitySnapshot s2{2, 100, 5, 5, 5, 0, 0, 0, 0};
    buf.PushSnapshot(s1);
    buf.PushSnapshot(s2);
    assert(buf.TrackedEntityCount() == 2);
    assert(buf.SnapshotCount(1) == 1);
    assert(buf.SnapshotCount(2) == 1);
}

void test_interp_buffer_depth_limit() {
    NetworkInterpolationBuffer buf(4); // depth 4
    for (uint32_t t = 0; t < 10; ++t) {
        EntitySnapshot snap{1, t, static_cast<float>(t), 0, 0, 0, 0, 0, 0};
        buf.PushSnapshot(snap);
    }
    assert(buf.SnapshotCount(1) == 4);
}

// ══════════════════════════════════════════════════════════════════
// LERP interpolation between snapshots
// ══════════════════════════════════════════════════════════════════

void test_interp_lerp_midpoint() {
    NetworkInterpolationBuffer buf;
    EntitySnapshot a{1, 100, 0, 0, 0, 1, 0, 0, 0};
    EntitySnapshot b{1, 110, 10, 0, 0, 1, 0, 0, 0};
    buf.PushSnapshot(a);
    buf.PushSnapshot(b);

    // Midpoint between tick 100 and 110
    auto result = buf.Interpolate(1, 105.0f);
    assert(result.valid);
    assert(approxEq(result.posX, 5.0f));
}

void test_interp_lerp_quarter() {
    NetworkInterpolationBuffer buf;
    EntitySnapshot a{1, 0, 0, 0, 0, 0, 0, 0, 0};
    EntitySnapshot b{1, 100, 100, 200, 300, 0, 0, 0, 0};
    buf.PushSnapshot(a);
    buf.PushSnapshot(b);

    auto result = buf.Interpolate(1, 25.0f);
    assert(result.valid);
    assert(approxEq(result.posX, 25.0f));
    assert(approxEq(result.posY, 50.0f));
    assert(approxEq(result.posZ, 75.0f));
}

void test_interp_lerp_at_exact_tick() {
    NetworkInterpolationBuffer buf;
    EntitySnapshot a{1, 10, 100, 200, 300, 0, 0, 0, 45.0f};
    buf.PushSnapshot(a);

    auto result = buf.Interpolate(1, 10.0f);
    assert(result.valid);
    assert(approxEq(result.posX, 100.0f));
    assert(approxEq(result.posY, 200.0f));
    assert(approxEq(result.posZ, 300.0f));
    assert(approxEq(result.rotYaw, 45.0f));
}

void test_interp_clamp_before_first() {
    NetworkInterpolationBuffer buf;
    EntitySnapshot a{1, 50, 100, 0, 0, 0, 0, 0, 0};
    buf.PushSnapshot(a);

    // Render tick before the first snapshot → clamp to first
    auto result = buf.Interpolate(1, 10.0f);
    assert(result.valid);
    assert(approxEq(result.posX, 100.0f));
}

// ══════════════════════════════════════════════════════════════════
// Velocity extrapolation
// ══════════════════════════════════════════════════════════════════

void test_interp_extrapolation() {
    NetworkInterpolationBuffer buf;
    EntitySnapshot s{1, 100, 0, 0, 0, 10.0f, 5.0f, 0, 0};
    buf.PushSnapshot(s);

    // 2 ticks beyond → should extrapolate using velocity
    auto result = buf.Interpolate(1, 102.0f);
    assert(result.valid);
    assert(approxEq(result.posX, 20.0f));
    assert(approxEq(result.posY, 10.0f));
}

void test_interp_extrapolation_max_exceeded() {
    NetworkInterpolationBuffer buf(8, 3.0f);
    EntitySnapshot s{1, 100, 0, 0, 0, 10.0f, 0, 0, 0};
    buf.PushSnapshot(s);

    // 4 ticks beyond (max is 3) → should return invalid
    auto result = buf.Interpolate(1, 104.0f);
    assert(!result.valid);
}

// ══════════════════════════════════════════════════════════════════
// Edge cases
// ══════════════════════════════════════════════════════════════════

void test_interp_unknown_entity() {
    NetworkInterpolationBuffer buf;
    auto result = buf.Interpolate(999, 50.0f);
    assert(!result.valid);
}

void test_interp_remove_entity() {
    NetworkInterpolationBuffer buf;
    EntitySnapshot s{1, 100, 0, 0, 0, 0, 0, 0, 0};
    buf.PushSnapshot(s);
    assert(buf.TrackedEntityCount() == 1);

    buf.RemoveEntity(1);
    assert(buf.TrackedEntityCount() == 0);
    assert(buf.SnapshotCount(1) == 0);
}

void test_interp_clear() {
    NetworkInterpolationBuffer buf;
    buf.PushSnapshot({1, 100, 0, 0, 0, 0, 0, 0, 0});
    buf.PushSnapshot({2, 100, 0, 0, 0, 0, 0, 0, 0});
    assert(buf.TrackedEntityCount() == 2);

    buf.Clear();
    assert(buf.TrackedEntityCount() == 0);
}

void test_interp_duplicate_tick_replaces() {
    NetworkInterpolationBuffer buf;
    buf.PushSnapshot({1, 100, 0, 0, 0, 0, 0, 0, 0});
    buf.PushSnapshot({1, 100, 99, 0, 0, 0, 0, 0, 0});
    assert(buf.SnapshotCount(1) == 1);

    auto result = buf.Interpolate(1, 100.0f);
    assert(result.valid);
    assert(approxEq(result.posX, 99.0f));
}

void test_interp_out_of_order_ticks() {
    NetworkInterpolationBuffer buf;
    buf.PushSnapshot({1, 110, 10, 0, 0, 0, 0, 0, 0});
    buf.PushSnapshot({1, 100, 0, 0, 0, 0, 0, 0, 0});
    assert(buf.SnapshotCount(1) == 2);

    // Should be sorted correctly and interpolate
    auto result = buf.Interpolate(1, 105.0f);
    assert(result.valid);
    assert(approxEq(result.posX, 5.0f));
}

void test_interp_multiple_segments() {
    NetworkInterpolationBuffer buf;
    buf.PushSnapshot({1, 0,   0, 0, 0, 0, 0, 0, 0});
    buf.PushSnapshot({1, 10, 10, 0, 0, 0, 0, 0, 0});
    buf.PushSnapshot({1, 20, 30, 0, 0, 0, 0, 0, 0}); // non-linear

    // Between tick 10 and 20 at midpoint
    auto result = buf.Interpolate(1, 15.0f);
    assert(result.valid);
    assert(approxEq(result.posX, 20.0f));
}

void test_interp_snapshot_count_nonexistent() {
    NetworkInterpolationBuffer buf;
    assert(buf.SnapshotCount(999) == 0);
}
