#include <cassert>
#include <cmath>
#include <string>
#include "../editor/tools/FleetFormationPanel.h"
#include "../cpp_server/include/pcg/generation_style.h"

using namespace atlas::pcg;

// ── Helpers ──────────────────────────────────────────────────────────

static void ok(const char* name) { (void)name; }

// ── Tests ────────────────────────────────────────────────────────────

void test_fmtn_defaults() {
    atlas::editor::FleetFormationPanel panel;
    assert(panel.GetFormationType() ==
           atlas::editor::FleetFormationPanel::FormationType::Arrow);
    assert(panel.GetFleetSize() == 5);
    assert(panel.GetSpacing() == 500.0f);
    assert(panel.SlotCount() == 5);
    assert(panel.SelectedSlot() == -1);
    assert(!panel.Log().empty()); // init message
    ok("test_fmtn_defaults");
}

void test_fmtn_set_formation_type() {
    atlas::editor::FleetFormationPanel panel;
    panel.SetFormationType(
        atlas::editor::FleetFormationPanel::FormationType::Diamond);
    assert(panel.GetFormationType() ==
           atlas::editor::FleetFormationPanel::FormationType::Diamond);
    ok("test_fmtn_set_formation_type");
}

void test_fmtn_formation_type_names() {
    using FT = atlas::editor::FleetFormationPanel::FormationType;
    assert(std::string(atlas::editor::FleetFormationPanel::FormationTypeName(FT::None))    == "None");
    assert(std::string(atlas::editor::FleetFormationPanel::FormationTypeName(FT::Arrow))   == "Arrow");
    assert(std::string(atlas::editor::FleetFormationPanel::FormationTypeName(FT::Line))    == "Line");
    assert(std::string(atlas::editor::FleetFormationPanel::FormationTypeName(FT::Wedge))   == "Wedge");
    assert(std::string(atlas::editor::FleetFormationPanel::FormationTypeName(FT::Spread))  == "Spread");
    assert(std::string(atlas::editor::FleetFormationPanel::FormationTypeName(FT::Diamond)) == "Diamond");
    ok("test_fmtn_formation_type_names");
}

void test_fmtn_set_fleet_size() {
    atlas::editor::FleetFormationPanel panel;
    panel.SetFleetSize(10);
    assert(panel.GetFleetSize() == 10);
    assert(panel.SlotCount() == 10);
    ok("test_fmtn_set_fleet_size");
}

void test_fmtn_fleet_size_clamped_low() {
    atlas::editor::FleetFormationPanel panel;
    panel.SetFleetSize(0);
    assert(panel.GetFleetSize() == 1);
    ok("test_fmtn_fleet_size_clamped_low");
}

void test_fmtn_fleet_size_clamped_high() {
    atlas::editor::FleetFormationPanel panel;
    panel.SetFleetSize(999);
    assert(panel.GetFleetSize() == 256);
    ok("test_fmtn_fleet_size_clamped_high");
}

void test_fmtn_set_spacing() {
    atlas::editor::FleetFormationPanel panel;
    panel.SetSpacing(1000.0f);
    assert(panel.GetSpacing() == 1000.0f);
    ok("test_fmtn_set_spacing");
}

void test_fmtn_spacing_clamped() {
    atlas::editor::FleetFormationPanel panel;
    panel.SetSpacing(1.0f);
    assert(panel.GetSpacing() == 10.0f);
    panel.SetSpacing(99999.0f);
    assert(panel.GetSpacing() == 10000.0f);
    ok("test_fmtn_spacing_clamped");
}

void test_fmtn_commander_at_origin() {
    atlas::editor::FleetFormationPanel panel;
    panel.ComputeOffsets();
    const auto& cmd = panel.GetSlot(0);
    assert(cmd.slotIndex == 0);
    assert(cmd.offsetX == 0.0f);
    assert(cmd.offsetY == 0.0f);
    assert(cmd.offsetZ == 0.0f);
    ok("test_fmtn_commander_at_origin");
}

void test_fmtn_arrow_offsets_nonzero() {
    atlas::editor::FleetFormationPanel panel;
    panel.SetFormationType(
        atlas::editor::FleetFormationPanel::FormationType::Arrow);
    assert(panel.SlotCount() >= 2);
    const auto& s1 = panel.GetSlot(1);
    // Arrow: slot 1 should have a non-zero X and negative Z offset
    assert(s1.offsetX != 0.0f || s1.offsetZ != 0.0f);
    ok("test_fmtn_arrow_offsets_nonzero");
}

void test_fmtn_line_offsets_lateral() {
    atlas::editor::FleetFormationPanel panel;
    panel.SetFormationType(
        atlas::editor::FleetFormationPanel::FormationType::Line);
    panel.SetFleetSize(3);
    // Line formation: slots fan out along X, Z stays 0
    for (size_t i = 1; i < panel.SlotCount(); ++i) {
        const auto& s = panel.GetSlot(i);
        assert(s.offsetZ == 0.0f);
        assert(s.offsetX != 0.0f);
    }
    ok("test_fmtn_line_offsets_lateral");
}

void test_fmtn_spread_circular() {
    atlas::editor::FleetFormationPanel panel;
    panel.SetFormationType(
        atlas::editor::FleetFormationPanel::FormationType::Spread);
    panel.SetFleetSize(4);
    // All non-commander slots should be at roughly the same distance
    float firstDist = 0.0f;
    for (size_t i = 1; i < panel.SlotCount(); ++i) {
        const auto& s = panel.GetSlot(i);
        float d = std::sqrt(s.offsetX * s.offsetX + s.offsetZ * s.offsetZ);
        if (i == 1) firstDist = d;
        assert(std::abs(d - firstDist) < 1.0f); // equal radius
    }
    ok("test_fmtn_spread_circular");
}

void test_fmtn_bounding_radius() {
    atlas::editor::FleetFormationPanel panel;
    panel.SetFormationType(
        atlas::editor::FleetFormationPanel::FormationType::Arrow);
    panel.SetFleetSize(5);
    float r = panel.BoundingRadius();
    assert(r > 0.0f);
    ok("test_fmtn_bounding_radius");
}

void test_fmtn_centre_of_mass() {
    atlas::editor::FleetFormationPanel panel;
    panel.SetFormationType(
        atlas::editor::FleetFormationPanel::FormationType::Line);
    panel.SetFleetSize(3);
    float cx, cy, cz;
    panel.CentreOfMass(cx, cy, cz);
    // Line is symmetric, so centre of mass should be near 0 on X
    assert(std::abs(cx) < 1.0f);
    assert(cy == 0.0f);
    ok("test_fmtn_centre_of_mass");
}

void test_fmtn_select_slot() {
    atlas::editor::FleetFormationPanel panel;
    panel.SelectSlot(2);
    assert(panel.SelectedSlot() == 2);
    panel.ClearSelection();
    assert(panel.SelectedSlot() == -1);
    ok("test_fmtn_select_slot");
}

void test_fmtn_select_invalid_ignored() {
    atlas::editor::FleetFormationPanel panel;
    panel.SelectSlot(999);
    assert(panel.SelectedSlot() == -1); // unchanged
    ok("test_fmtn_select_invalid_ignored");
}

void test_fmtn_import_fleet() {
    GeneratedFleetCompositionResult fleet;
    fleet.fleetId = 42;
    fleet.doctrineName = "TestDoctrine";
    fleet.capitalCount = 1;
    fleet.subcapCount = 2;
    fleet.aggressionRating = 0.7f;
    fleet.valid = true;

    GeneratedFleetShip ship1;
    ship1.shipId = 1; ship1.hullClass = HullClass::Battleship;
    ship1.role = "command"; ship1.shipName = "Flagship";

    GeneratedFleetShip ship2;
    ship2.shipId = 2; ship2.hullClass = HullClass::Cruiser;
    ship2.role = "dps"; ship2.shipName = "Striker";

    GeneratedFleetShip ship3;
    ship3.shipId = 3; ship3.hullClass = HullClass::Frigate;
    ship3.role = "tackle"; ship3.shipName = "Scout";

    fleet.ships = {ship1, ship2, ship3};

    atlas::editor::FleetFormationPanel panel;
    panel.ImportFleet(fleet);

    assert(panel.GetFleetSize() == 3);
    assert(panel.SlotCount() == 3);
    assert(panel.GetSlot(0).shipName == "Flagship");
    assert(panel.GetSlot(0).role == "command");
    assert(panel.GetSlot(1).shipName == "Striker");
    assert(panel.GetSlot(2).role == "tackle");
    assert(panel.GetSlot(2).hullClass == HullClass::Frigate);
    ok("test_fmtn_import_fleet");
}

void test_fmtn_none_formation_no_offsets() {
    atlas::editor::FleetFormationPanel panel;
    panel.SetFormationType(
        atlas::editor::FleetFormationPanel::FormationType::None);
    panel.SetFleetSize(5);
    for (size_t i = 0; i < panel.SlotCount(); ++i) {
        const auto& s = panel.GetSlot(i);
        assert(s.offsetX == 0.0f);
        assert(s.offsetY == 0.0f);
        assert(s.offsetZ == 0.0f);
    }
    ok("test_fmtn_none_formation_no_offsets");
}

void test_fmtn_spacing_affects_offsets() {
    atlas::editor::FleetFormationPanel panel;
    panel.SetFormationType(
        atlas::editor::FleetFormationPanel::FormationType::Arrow);
    panel.SetFleetSize(3);

    panel.SetSpacing(100.0f);
    float r1 = panel.BoundingRadius();

    panel.SetSpacing(200.0f);
    float r2 = panel.BoundingRadius();

    assert(r2 > r1);
    ok("test_fmtn_spacing_affects_offsets");
}

void test_fmtn_draw_does_not_crash() {
    atlas::editor::FleetFormationPanel panel;
    panel.SetFleetSize(10);
    panel.SetFormationType(
        atlas::editor::FleetFormationPanel::FormationType::Diamond);
    panel.ImportFleet(GeneratedFleetCompositionResult{});
    panel.Draw(); // headless — should be a no-op
    ok("test_fmtn_draw_does_not_crash");
}

void test_fmtn_log_after_actions() {
    atlas::editor::FleetFormationPanel panel;
    size_t initialLog = panel.Log().size();
    panel.SetFleetSize(8);
    panel.SetFormationType(
        atlas::editor::FleetFormationPanel::FormationType::Wedge);
    panel.SetSpacing(750.0f);
    assert(panel.Log().size() > initialLog);
    ok("test_fmtn_log_after_actions");
}
