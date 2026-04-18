/**
 * Tests for LowPolyStyleValidator (Phase 23):
 *   - Valid mesh passes all checks
 *   - Each violation detected individually
 *   - Palette validation (6–10 colours)
 *   - Budget helpers (Corridor, Room, Door)
 */

#include <cassert>
#include <string>
#include <array>
#include "../cpp_client/include/gameplay/lowpoly_style.h"

using namespace atlas::gameplay;

// ── Helpers ───────────────────────────────────────────────────────────────────

static LowPolyMesh ValidMesh(int faces = 30, int materials = 1) {
    LowPolyMesh m;
    m.id             = "test_mesh";
    m.faceCount      = faces;
    m.hasFlatNormals = true;
    m.hasBevelEdges  = false;
    m.materialCount  = materials;
    m.hasRoughnessMap = false;
    m.hasNormalMap    = false;
    m.maxSidedCylinder = 0;
    return m;
}

// ── Tests ─────────────────────────────────────────────────────────────────────

void test_lowpoly_valid_mesh_passes() {
    LowPolyStyleValidator v;
    auto result = v.Validate(ValidMesh());
    assert(result.valid);
    assert(result.error.empty());
}

void test_lowpoly_fails_no_flat_normals() {
    LowPolyStyleValidator v;
    auto m = ValidMesh();
    m.hasFlatNormals = false;
    auto result = v.Validate(m);
    assert(!result.valid);
    assert(!result.error.empty());
}

void test_lowpoly_fails_bevel_edges() {
    LowPolyStyleValidator v;
    auto m = ValidMesh();
    m.hasBevelEdges = true;
    auto result = v.Validate(m);
    assert(!result.valid);
}

void test_lowpoly_fails_over_budget() {
    LowPolyStyleValidator v;
    auto m = ValidMesh(100);  // 100 > 60 room max
    auto result = v.Validate(m, RoomBudget());
    assert(!result.valid);
}

void test_lowpoly_passes_exactly_at_budget() {
    LowPolyStyleValidator v;
    auto m = ValidMesh(60);
    auto result = v.Validate(m, RoomBudget());
    assert(result.valid);
}

void test_lowpoly_fails_too_many_materials() {
    LowPolyStyleValidator v;
    auto m = ValidMesh(30, 3);  // 3 > 2 limit
    auto result = v.Validate(m);
    assert(!result.valid);
}

void test_lowpoly_fails_roughness_map() {
    LowPolyStyleValidator v;
    auto m = ValidMesh();
    m.hasRoughnessMap = true;
    auto result = v.Validate(m);
    assert(!result.valid);
}

void test_lowpoly_fails_normal_map() {
    LowPolyStyleValidator v;
    auto m = ValidMesh();
    m.hasNormalMap = true;
    auto result = v.Validate(m);
    assert(!result.valid);
}

void test_lowpoly_fails_cylinder_too_many_sides() {
    LowPolyStyleValidator v;
    auto m = ValidMesh();
    m.maxSidedCylinder = 12;  // > 8 allowed
    auto result = v.Validate(m);
    assert(!result.valid);
}

void test_lowpoly_passes_cylinder_8_sides() {
    LowPolyStyleValidator v;
    auto m = ValidMesh();
    m.maxSidedCylinder = 8;
    auto result = v.Validate(m);
    assert(result.valid);
}

void test_lowpoly_corridor_budget_tighter() {
    auto corridor = CorridorBudget();
    auto room     = RoomBudget();
    assert(corridor.maxFaces < room.maxFaces);
}

void test_lowpoly_door_budget_smallest() {
    auto door     = DoorBudget();
    auto corridor = CorridorBudget();
    assert(door.maxFaces <= corridor.maxFaces);
}

void test_lowpoly_is_valid_shorthand() {
    LowPolyStyleValidator v;
    auto m = ValidMesh();
    assert(v.IsValid(m));
    m.hasFlatNormals = false;
    assert(!v.IsValid(m));
}

void test_lowpoly_palette_valid() {
    LowPolyStyleValidator v;
    BiomePalette p;
    p.biomeID = "space_station";
    for (int i = 0; i < 8; ++i)
        p.colours.push_back({ static_cast<uint8_t>(i * 30), 100, 150 });
    auto result = v.ValidatePalette(p);
    assert(result.valid);
}

void test_lowpoly_palette_too_few_colours() {
    LowPolyStyleValidator v;
    BiomePalette p;
    for (int i = 0; i < 4; ++i)
        p.colours.push_back({ 0, 0, 0 });
    auto result = v.ValidatePalette(p);
    assert(!result.valid);
}

void test_lowpoly_palette_too_many_colours() {
    LowPolyStyleValidator v;
    BiomePalette p;
    for (int i = 0; i < 12; ++i)
        p.colours.push_back({ 0, 0, 0 });
    auto result = v.ValidatePalette(p);
    assert(!result.valid);
}

void test_lowpoly_palette_exactly_6_valid() {
    LowPolyStyleValidator v;
    BiomePalette p;
    for (int i = 0; i < 6; ++i)
        p.colours.push_back({ 0, 0, 0 });
    auto result = v.ValidatePalette(p);
    assert(result.valid);
}

void test_lowpoly_palette_exactly_10_valid() {
    LowPolyStyleValidator v;
    BiomePalette p;
    for (int i = 0; i < 10; ++i)
        p.colours.push_back({ 0, 0, 0 });
    auto result = v.ValidatePalette(p);
    assert(result.valid);
}
