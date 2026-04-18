/**
 * Tests for FPSInteriorGenerator (Phase 23):
 *   - Grammar produces correct room sequence
 *   - Room constraints (entry angles, poly budget, elevation)
 *   - StartRoom / ObjectiveRoom accessors
 *   - Validation pass / fail cases
 *   - Optional side room toggling
 */

#include <cassert>
#include <string>
#include "../cpp_client/include/gameplay/fps_interior_generator.h"

using namespace atlas::gameplay;

void test_interior_room_type_names() {
    assert(std::string(InteriorRoomTypeName(InteriorRoomType::AIRLOCK))        == "Airlock");
    assert(std::string(InteriorRoomTypeName(InteriorRoomType::CHOKE))          == "Choke");
    assert(std::string(InteriorRoomTypeName(InteriorRoomType::PRIMARY_COMBAT)) == "Primary Combat");
    assert(std::string(InteriorRoomTypeName(InteriorRoomType::SIDE_ROOM))      == "Side Room");
    assert(std::string(InteriorRoomTypeName(InteriorRoomType::OBJECTIVE))      == "Objective");
}

void test_interior_generate_with_side_room() {
    FPSInteriorGenerator gen;
    InteriorGenOptions opts;
    opts.seed            = 42;
    opts.difficulty      = 1;
    opts.includeSideRoom = true;

    auto layout = gen.Generate("mission_01", opts);
    assert(layout.missionID == "mission_01");
    assert(layout.rooms.size() == 5);  // airlock, choke, combat, side, objective
}

void test_interior_generate_without_side_room() {
    FPSInteriorGenerator gen;
    InteriorGenOptions opts;
    opts.seed            = 0;
    opts.includeSideRoom = false;

    auto layout = gen.Generate("mission_02", opts);
    assert(layout.rooms.size() == 4);  // airlock, choke, combat, objective
}

void test_interior_starts_with_airlock() {
    FPSInteriorGenerator gen;
    auto layout = gen.Generate("mission_03");
    const auto* start = layout.StartRoom();
    assert(start != nullptr);
    assert(start->type == InteriorRoomType::AIRLOCK);
}

void test_interior_ends_with_objective() {
    FPSInteriorGenerator gen;
    auto layout = gen.Generate("mission_04");
    const auto* obj = layout.ObjectiveRoom();
    assert(obj != nullptr);
    assert(obj->type == InteriorRoomType::OBJECTIVE);
}

void test_interior_rooms_have_valid_entry_angles() {
    FPSInteriorGenerator gen;
    auto layout = gen.Generate("mission_05");
    for (const auto& room : layout.rooms) {
        assert(room.entryAngles >= 1);
        assert(room.entryAngles <= 3);
    }
}

void test_interior_rooms_within_poly_budget() {
    FPSInteriorGenerator gen;
    auto layout = gen.Generate("mission_06");
    for (const auto& room : layout.rooms) {
        int maxFaces = (room.type == InteriorRoomType::AIRLOCK ||
                        room.type == InteriorRoomType::CHOKE)
                       ? FPSInteriorGenerator::CORRIDOR_MAX_FACES
                       : FPSInteriorGenerator::ROOM_MAX_FACES;
        assert(room.polyFaceCount <= maxFaces);
    }
}

void test_interior_validation_passes() {
    FPSInteriorGenerator gen;
    auto layout = gen.Generate("mission_07");
    std::string err = gen.Validate(layout);
    assert(err.empty());
}

void test_interior_validation_fails_no_rooms() {
    FPSInteriorGenerator gen;
    FPSInteriorLayout empty;
    empty.missionID = "empty";
    std::string err = gen.Validate(empty);
    assert(!err.empty());
}

void test_interior_validation_fails_wrong_start() {
    FPSInteriorGenerator gen;
    FPSInteriorLayout layout;
    layout.missionID = "bad";
    InteriorRoom r;
    r.id   = "r1";
    r.type = InteriorRoomType::CHOKE;  // wrong start
    r.entryAngles = 2;
    r.polyFaceCount = 20;
    layout.rooms.push_back(r);
    InteriorRoom r2;
    r2.id   = "r2";
    r2.type = InteriorRoomType::OBJECTIVE;
    r2.entryAngles = 2;
    r2.polyFaceCount = 30;
    layout.rooms.push_back(r2);
    std::string err = gen.Validate(layout);
    assert(!err.empty());
}

void test_interior_rooms_connected() {
    FPSInteriorGenerator gen;
    InteriorGenOptions opts;
    opts.includeSideRoom = false;
    auto layout = gen.Generate("mission_08", opts);
    // Choke should connect to both airlock and combat
    const auto& choke = layout.rooms[1];
    assert(choke.connections.size() == 2);
}

void test_interior_difficulty_3_has_elevation() {
    FPSInteriorGenerator gen;
    InteriorGenOptions opts;
    opts.seed       = 1;
    opts.difficulty = 3;
    auto layout = gen.Generate("mission_09", opts);
    bool hasElevation = false;
    for (const auto& room : layout.rooms)
        if (room.hasElevation) hasElevation = true;
    assert(hasElevation);
}
