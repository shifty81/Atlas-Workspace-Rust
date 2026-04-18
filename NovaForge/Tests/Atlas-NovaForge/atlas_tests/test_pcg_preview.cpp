#include <iostream>
#include <cassert>
#include <string>
#include "../editor/tools/PCGPreviewPanel.h"

// ── Helpers ─────────────────────────────────────────────────────────

static int passed = 0;

static void ok(const char* name) {
    // Output handled by RUN_TEST in main.cpp
    ++passed;
}

// ── Tests ───────────────────────────────────────────────────────────

void test_pcg_preview_defaults() {
    atlas::editor::PCGPreviewPanel panel;
    auto& s = panel.Settings();

    assert(s.mode         == atlas::editor::PCGPreviewMode::Ship);
    assert(s.seed         == 42);
    assert(s.version      == 1);
    assert(s.overrideHull == false);
    assert(s.overrideModuleCount == false);
    assert(s.shipClass    == 0);

    assert(std::string(panel.Name()) == "PCG Preview");
    assert(panel.GetShipPreview().populated     == false);
    assert(panel.GetStationPreview().populated  == false);
    assert(panel.GetInteriorPreview().populated == false);
    assert(panel.Log().empty());

    ok("test_pcg_preview_defaults");
}

void test_pcg_preview_generate_ship() {
    atlas::editor::PCGPreviewPanel panel;
    panel.Generate();  // defaults to Ship mode

    assert(panel.GetShipPreview().populated);
    auto& ship = panel.GetShipPreview().data;
    assert(!ship.shipName.empty());
    assert(ship.mass > 0.0f);
    assert(ship.thrust > 0.0f);
    assert(!panel.Log().empty());

    ok("test_pcg_preview_generate_ship");
}

void test_pcg_preview_generate_ship_override_hull() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s = panel.Settings();
    s.overrideHull = true;
    s.hullClass    = atlas::pcg::HullClass::Battleship;
    panel.SetSettings(s);
    panel.Generate();

    assert(panel.GetShipPreview().populated);
    assert(panel.GetShipPreview().data.hullClass ==
           atlas::pcg::HullClass::Battleship);

    ok("test_pcg_preview_generate_ship_override_hull");
}

void test_pcg_preview_generate_station() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s = panel.Settings();
    s.mode = atlas::editor::PCGPreviewMode::Station;
    panel.SetSettings(s);
    panel.Generate();

    assert(panel.GetStationPreview().populated);
    assert(!panel.GetStationPreview().data.modules.empty());
    assert(!panel.Log().empty());

    ok("test_pcg_preview_generate_station");
}

void test_pcg_preview_generate_station_override_count() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s = panel.Settings();
    s.mode                = atlas::editor::PCGPreviewMode::Station;
    s.overrideModuleCount = true;
    s.moduleCount         = 7;
    panel.SetSettings(s);
    panel.Generate();

    assert(panel.GetStationPreview().populated);
    assert(panel.GetStationPreview().data.modules.size() == 7);

    ok("test_pcg_preview_generate_station_override_count");
}

void test_pcg_preview_generate_interior() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s = panel.Settings();
    s.mode      = atlas::editor::PCGPreviewMode::Interior;
    s.shipClass = 2;  // Cruiser
    panel.SetSettings(s);
    panel.Generate();

    assert(panel.GetInteriorPreview().populated);
    assert(!panel.GetInteriorPreview().data.rooms.empty());
    assert(!panel.GetInteriorPreview().data.corridors.empty());

    ok("test_pcg_preview_generate_interior");
}

void test_pcg_preview_determinism() {
    // Same seed + settings must produce identical output.
    atlas::editor::PCGPreviewPanel a;
    atlas::editor::PCGPreviewPanel b;
    a.Generate();
    b.Generate();

    assert(a.GetShipPreview().data.shipName ==
           b.GetShipPreview().data.shipName);
    assert(a.GetShipPreview().data.mass ==
           b.GetShipPreview().data.mass);
    assert(a.GetShipPreview().data.turretSlots ==
           b.GetShipPreview().data.turretSlots);

    ok("test_pcg_preview_determinism");
}

void test_pcg_preview_randomize_changes_seed() {
    atlas::editor::PCGPreviewPanel panel;
    uint64_t originalSeed = panel.Settings().seed;
    panel.Randomize();
    assert(panel.Settings().seed != originalSeed);
    assert(panel.GetShipPreview().populated);

    ok("test_pcg_preview_randomize_changes_seed");
}

void test_pcg_preview_clear() {
    atlas::editor::PCGPreviewPanel panel;
    panel.Generate();
    assert(panel.GetShipPreview().populated);
    assert(!panel.Log().empty());

    panel.ClearPreview();
    assert(!panel.GetShipPreview().populated);
    assert(!panel.GetStationPreview().populated);
    assert(!panel.GetInteriorPreview().populated);
    assert(panel.Log().empty());

    ok("test_pcg_preview_clear");
}

void test_pcg_preview_set_settings() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s;
    s.mode      = atlas::editor::PCGPreviewMode::Interior;
    s.seed      = 9999;
    s.shipClass = 4;  // Battleship-class interior
    panel.SetSettings(s);

    assert(panel.Settings().mode      == atlas::editor::PCGPreviewMode::Interior);
    assert(panel.Settings().seed      == 9999);
    assert(panel.Settings().shipClass == 4);

    ok("test_pcg_preview_set_settings");
}

void test_pcg_preview_different_seeds_differ() {
    atlas::editor::PCGPreviewPanel a;
    atlas::editor::PCGPreviewPanel b;

    atlas::editor::PCGPreviewSettings sa = a.Settings();
    sa.seed = 100;
    a.SetSettings(sa);

    atlas::editor::PCGPreviewSettings sb = b.Settings();
    sb.seed = 200;
    b.SetSettings(sb);

    a.Generate();
    b.Generate();

    // Different seeds should (almost certainly) produce different ships.
    assert(a.GetShipPreview().data.shipName !=
           b.GetShipPreview().data.shipName ||
           a.GetShipPreview().data.mass !=
           b.GetShipPreview().data.mass);

    ok("test_pcg_preview_different_seeds_differ");
}

void test_pcg_preview_draw_does_not_crash() {
    atlas::editor::PCGPreviewPanel panel;
    panel.Generate();
    panel.Draw();  // Should be a safe no-op stub
    ok("test_pcg_preview_draw_does_not_crash");
}

// ── SpineHull Preview Tests ─────────────────────────────────────────

void test_pcg_preview_generate_spine_hull() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s = panel.Settings();
    s.mode = atlas::editor::PCGPreviewMode::SpineHull;
    panel.SetSettings(s);
    panel.Generate();

    assert(panel.GetSpineHullPreview().populated);
    auto& hull = panel.GetSpineHullPreview().data;
    assert(hull.profile.length > 0.0f);
    assert(hull.profile.width_mid > 0.0f);
    assert(hull.engine_cluster_count > 0);
    assert(hull.zones.size() == 3);
    assert(hull.bilateral_symmetry);
    assert(hull.valid);
    assert(!panel.Log().empty());

    ok("test_pcg_preview_generate_spine_hull");
}

void test_pcg_preview_spine_hull_override_hull_class() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s = panel.Settings();
    s.mode         = atlas::editor::PCGPreviewMode::SpineHull;
    s.overrideHull = true;
    s.hullClass    = atlas::pcg::HullClass::Capital;
    panel.SetSettings(s);
    panel.Generate();

    assert(panel.GetSpineHullPreview().populated);
    assert(panel.GetSpineHullPreview().data.hull_class ==
           atlas::pcg::HullClass::Capital);

    ok("test_pcg_preview_spine_hull_override_hull_class");
}

void test_pcg_preview_spine_hull_override_faction() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s = panel.Settings();
    s.mode            = atlas::editor::PCGPreviewMode::SpineHull;
    s.overrideHull    = true;
    s.hullClass       = atlas::pcg::HullClass::Cruiser;
    s.overrideFaction = true;
    s.faction         = "Solari";
    panel.SetSettings(s);
    panel.Generate();

    assert(panel.GetSpineHullPreview().populated);
    assert(panel.GetSpineHullPreview().data.faction_style == "Solari");
    assert(panel.GetSpineHullPreview().data.valid);

    ok("test_pcg_preview_spine_hull_override_faction");
}

void test_pcg_preview_spine_hull_determinism() {
    atlas::editor::PCGPreviewSettings s;
    s.mode         = atlas::editor::PCGPreviewMode::SpineHull;
    s.seed         = 777;
    s.overrideHull = true;
    s.hullClass    = atlas::pcg::HullClass::Destroyer;

    atlas::editor::PCGPreviewPanel a;
    a.SetSettings(s);
    a.Generate();

    atlas::editor::PCGPreviewPanel b;
    b.SetSettings(s);
    b.Generate();

    assert(a.GetSpineHullPreview().data.spine ==
           b.GetSpineHullPreview().data.spine);
    assert(a.GetSpineHullPreview().data.profile.length ==
           b.GetSpineHullPreview().data.profile.length);
    assert(a.GetSpineHullPreview().data.engine_cluster_count ==
           b.GetSpineHullPreview().data.engine_cluster_count);

    ok("test_pcg_preview_spine_hull_determinism");
}

void test_pcg_preview_spine_hull_aspect_ratio_clamped() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s = panel.Settings();
    s.mode         = atlas::editor::PCGPreviewMode::SpineHull;
    s.overrideHull = true;
    s.hullClass    = atlas::pcg::HullClass::Frigate;
    panel.SetSettings(s);
    panel.Generate();

    auto& hull = panel.GetSpineHullPreview().data;
    assert(hull.aspect_ratio >= 1.5f);
    assert(hull.aspect_ratio <= 15.0f);

    ok("test_pcg_preview_spine_hull_aspect_ratio_clamped");
}

// ── TurretPlacement Preview Tests ───────────────────────────────────

void test_pcg_preview_generate_turret_placement() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s = panel.Settings();
    s.mode         = atlas::editor::PCGPreviewMode::TurretPlacement;
    s.overrideHull = true;
    s.hullClass    = atlas::pcg::HullClass::Cruiser;
    panel.SetSettings(s);
    panel.Generate();

    assert(panel.GetTurretPlacementPreview().populated);
    auto& tp = panel.GetTurretPlacementPreview().data;
    assert(!tp.mounts.empty());
    assert(tp.coverage_score > 0.0f);

    // Underlying hull should also be populated.
    auto& hull = panel.GetTurretPlacementPreview().hull;
    assert(hull.valid);

    assert(!panel.Log().empty());

    ok("test_pcg_preview_generate_turret_placement");
}

void test_pcg_preview_turret_placement_override_slots() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s = panel.Settings();
    s.mode                = atlas::editor::PCGPreviewMode::TurretPlacement;
    s.overrideHull        = true;
    s.hullClass           = atlas::pcg::HullClass::Battleship;
    s.overrideTurretSlots = true;
    s.turretSlots         = 6;
    panel.SetSettings(s);
    panel.Generate();

    assert(panel.GetTurretPlacementPreview().populated);
    assert(panel.GetTurretPlacementPreview().data.mounts.size() == 6);

    ok("test_pcg_preview_turret_placement_override_slots");
}

void test_pcg_preview_turret_placement_with_faction() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s = panel.Settings();
    s.mode                = atlas::editor::PCGPreviewMode::TurretPlacement;
    s.overrideHull        = true;
    s.hullClass           = atlas::pcg::HullClass::Frigate;
    s.overrideFaction     = true;
    s.faction             = "Keldari";
    s.overrideTurretSlots = true;
    s.turretSlots         = 3;
    panel.SetSettings(s);
    panel.Generate();

    assert(panel.GetTurretPlacementPreview().populated);
    assert(panel.GetTurretPlacementPreview().data.mounts.size() == 3);
    assert(panel.GetTurretPlacementPreview().hull.faction_style == "Keldari");

    ok("test_pcg_preview_turret_placement_with_faction");
}

void test_pcg_preview_turret_placement_overlap_validation() {
    atlas::editor::PCGPreviewPanel panel;
    atlas::editor::PCGPreviewSettings s = panel.Settings();
    s.mode                = atlas::editor::PCGPreviewMode::TurretPlacement;
    s.overrideHull        = true;
    s.hullClass           = atlas::pcg::HullClass::Cruiser;
    s.overrideTurretSlots = true;
    s.turretSlots         = 4;
    panel.SetSettings(s);
    panel.Generate();

    auto& tp = panel.GetTurretPlacementPreview().data;
    // Coverage should be between 0 and 1
    assert(tp.coverage_score >= 0.0f && tp.coverage_score <= 1.0f);
    // Max overlap should be between 0 and 1
    assert(tp.max_overlap >= 0.0f && tp.max_overlap <= 1.0f);

    ok("test_pcg_preview_turret_placement_overlap_validation");
}

void test_pcg_preview_clear_includes_new_modes() {
    atlas::editor::PCGPreviewPanel panel;

    // Generate a spine hull
    atlas::editor::PCGPreviewSettings s = panel.Settings();
    s.mode = atlas::editor::PCGPreviewMode::SpineHull;
    panel.SetSettings(s);
    panel.Generate();
    assert(panel.GetSpineHullPreview().populated);

    panel.ClearPreview();
    assert(!panel.GetSpineHullPreview().populated);
    assert(!panel.GetTurretPlacementPreview().populated);
    assert(panel.Log().empty());

    ok("test_pcg_preview_clear_includes_new_modes");
}
