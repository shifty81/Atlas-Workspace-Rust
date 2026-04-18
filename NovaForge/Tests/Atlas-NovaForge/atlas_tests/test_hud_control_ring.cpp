/**
 * Tests for the Ship HUD Control Ring widgets:
 *   - capacitorVerticalBar
 *   - velocityArc
 *
 * Validates data flow, clamping, and edge cases without requiring
 * an OpenGL context (headless renderer stubs used in test builds).
 */

#include "../cpp_client/include/ui/atlas/atlas_widgets.h"
#include "../cpp_client/include/ui/atlas/atlas_hud.h"
#include <cassert>
#include <cmath>

// ── capacitorVerticalBar tests ─────────────────────────────────────

void test_cap_vbar_zero_fraction() {
    atlas::AtlasContext ctx;
    atlas::Rect r{100, 100, 20, 120};
    // Should not crash with 0% capacitor
    atlas::capacitorVerticalBar(ctx, r, 0.0f, 16);
}

void test_cap_vbar_full_fraction() {
    atlas::AtlasContext ctx;
    atlas::Rect r{100, 100, 20, 120};
    atlas::capacitorVerticalBar(ctx, r, 1.0f, 16);
}

void test_cap_vbar_half_fraction() {
    atlas::AtlasContext ctx;
    atlas::Rect r{100, 100, 20, 120};
    atlas::capacitorVerticalBar(ctx, r, 0.5f, 10);
}

void test_cap_vbar_clamp_over() {
    atlas::AtlasContext ctx;
    atlas::Rect r{100, 100, 20, 120};
    // Fraction > 1.0 should clamp to 1.0, not crash
    atlas::capacitorVerticalBar(ctx, r, 2.5f, 8);
}

void test_cap_vbar_clamp_under() {
    atlas::AtlasContext ctx;
    atlas::Rect r{100, 100, 20, 120};
    // Negative fraction should clamp to 0.0
    atlas::capacitorVerticalBar(ctx, r, -1.0f, 8);
}

void test_cap_vbar_single_segment() {
    atlas::AtlasContext ctx;
    atlas::Rect r{100, 100, 20, 120};
    atlas::capacitorVerticalBar(ctx, r, 0.75f, 1);
}

void test_cap_vbar_many_segments() {
    atlas::AtlasContext ctx;
    atlas::Rect r{100, 100, 20, 240};
    atlas::capacitorVerticalBar(ctx, r, 0.6f, 64);
}

void test_cap_vbar_zero_segments() {
    atlas::AtlasContext ctx;
    atlas::Rect r{100, 100, 20, 120};
    // 0 segments should be clamped to 1
    atlas::capacitorVerticalBar(ctx, r, 0.5f, 0);
}

// ── velocityArc tests ──────────────────────────────────────────────

void test_vel_arc_stopped() {
    atlas::AtlasContext ctx;
    atlas::Vec2 centre{400, 400};
    // Speed 0 with default mode
    atlas::velocityArc(ctx, centre, 50.0f, 60.0f, 0.0f, 0);
}

void test_vel_arc_full_speed() {
    atlas::AtlasContext ctx;
    atlas::Vec2 centre{400, 400};
    atlas::velocityArc(ctx, centre, 50.0f, 60.0f, 1.0f, 1);
}

void test_vel_arc_approach_mode() {
    atlas::AtlasContext ctx;
    atlas::Vec2 centre{400, 400};
    atlas::velocityArc(ctx, centre, 50.0f, 60.0f, 0.75f, 1);
}

void test_vel_arc_orbit_mode() {
    atlas::AtlasContext ctx;
    atlas::Vec2 centre{400, 400};
    atlas::velocityArc(ctx, centre, 50.0f, 60.0f, 0.5f, 2);
}

void test_vel_arc_keep_range_mode() {
    atlas::AtlasContext ctx;
    atlas::Vec2 centre{400, 400};
    atlas::velocityArc(ctx, centre, 50.0f, 60.0f, 0.3f, 3);
}

void test_vel_arc_warp_mode() {
    atlas::AtlasContext ctx;
    atlas::Vec2 centre{400, 400};
    atlas::velocityArc(ctx, centre, 50.0f, 60.0f, 1.0f, 4);
}

void test_vel_arc_clamp_over() {
    atlas::AtlasContext ctx;
    atlas::Vec2 centre{400, 400};
    atlas::velocityArc(ctx, centre, 50.0f, 60.0f, 5.0f, 1);
}

void test_vel_arc_clamp_under() {
    atlas::AtlasContext ctx;
    atlas::Vec2 centre{400, 400};
    atlas::velocityArc(ctx, centre, 50.0f, 60.0f, -2.0f, 0);
}

void test_vel_arc_unknown_mode() {
    atlas::AtlasContext ctx;
    atlas::Vec2 centre{400, 400};
    // Unknown mode (99) should use default color
    atlas::velocityArc(ctx, centre, 50.0f, 60.0f, 0.5f, 99);
}

void test_vel_arc_default_mode_param() {
    atlas::AtlasContext ctx;
    atlas::Vec2 centre{400, 400};
    // Test default parameter (movementMode defaults to 0)
    atlas::velocityArc(ctx, centre, 50.0f, 60.0f, 0.5f);
}

// ── Integration: HUD with new widgets ──────────────────────────────

void test_hud_ship_data_with_cap() {
    atlas::ShipHUDData ship;
    ship.capacitorPct = 0.65f;
    ship.currentSpeed = 150.0f;
    ship.maxSpeed = 300.0f;

    // Verify data is accessible
    assert(std::fabs(ship.capacitorPct - 0.65f) < 0.001f);
    assert(std::fabs(ship.currentSpeed - 150.0f) < 0.001f);
    assert(std::fabs(ship.maxSpeed - 300.0f) < 0.001f);
}

void test_hud_ship_data_warp_state() {
    atlas::ShipHUDData ship;
    ship.warpActive = true;
    ship.warpPhase = 3;
    ship.warpProgress = 0.7f;
    ship.warpSpeedAU = 3.5f;

    assert(ship.warpActive);
    assert(ship.warpPhase == 3);
    assert(std::fabs(ship.warpProgress - 0.7f) < 0.001f);
    assert(std::fabs(ship.warpSpeedAU - 3.5f) < 0.001f);
}
