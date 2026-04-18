// S198 editor tests: CameraRigEditorV1, LevelStreamEditorV1, AtmosphereEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/CameraRigEditorV1.h"
#include "NF/Editor/LevelStreamEditorV1.h"
#include "NF/Editor/AtmosphereEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── CameraRigEditorV1 ───────────────────────────────────────────────────────

TEST_CASE("Crv1Rig validity", "[Editor][S198]") {
    Crv1Rig r;
    REQUIRE(!r.isValid());
    r.id = 1; r.name = "MainCam";
    REQUIRE(r.isValid());
}

TEST_CASE("CameraRigEditorV1 addRig and rigCount", "[Editor][S198]") {
    CameraRigEditorV1 cr;
    REQUIRE(cr.rigCount() == 0);
    Crv1Rig r; r.id = 1; r.name = "R1";
    REQUIRE(cr.addRig(r));
    REQUIRE(cr.rigCount() == 1);
}

TEST_CASE("CameraRigEditorV1 addRig invalid fails", "[Editor][S198]") {
    CameraRigEditorV1 cr;
    REQUIRE(!cr.addRig(Crv1Rig{}));
}

TEST_CASE("CameraRigEditorV1 addRig duplicate fails", "[Editor][S198]") {
    CameraRigEditorV1 cr;
    Crv1Rig r; r.id = 1; r.name = "A";
    cr.addRig(r);
    REQUIRE(!cr.addRig(r));
}

TEST_CASE("CameraRigEditorV1 removeRig", "[Editor][S198]") {
    CameraRigEditorV1 cr;
    Crv1Rig r; r.id = 2; r.name = "B";
    cr.addRig(r);
    REQUIRE(cr.removeRig(2));
    REQUIRE(cr.rigCount() == 0);
    REQUIRE(!cr.removeRig(2));
}

TEST_CASE("CameraRigEditorV1 findRig", "[Editor][S198]") {
    CameraRigEditorV1 cr;
    Crv1Rig r; r.id = 3; r.name = "C";
    cr.addRig(r);
    REQUIRE(cr.findRig(3) != nullptr);
    REQUIRE(cr.findRig(99) == nullptr);
}

TEST_CASE("CameraRigEditorV1 setState", "[Editor][S198]") {
    CameraRigEditorV1 cr;
    Crv1Rig r; r.id = 1; r.name = "R";
    cr.addRig(r);
    REQUIRE(cr.setState(1, Crv1RigState::Recording));
    REQUIRE(cr.findRig(1)->state == Crv1RigState::Recording);
}

TEST_CASE("CameraRigEditorV1 addWaypoint", "[Editor][S198]") {
    CameraRigEditorV1 cr;
    Crv1Rig r; r.id = 1; r.name = "R";
    cr.addRig(r);
    Crv1Waypoint wp; wp.id = 10; wp.posX = 1.f; wp.time = 0.f;
    REQUIRE(cr.addWaypoint(1, wp));
    REQUIRE(cr.findRig(1)->waypoints.size() == 1);
}

TEST_CASE("CameraRigEditorV1 addWaypoint duplicate fails", "[Editor][S198]") {
    CameraRigEditorV1 cr;
    Crv1Rig r; r.id = 1; r.name = "R";
    cr.addRig(r);
    Crv1Waypoint wp; wp.id = 10;
    cr.addWaypoint(1, wp);
    REQUIRE(!cr.addWaypoint(1, wp));
}

TEST_CASE("CameraRigEditorV1 setFov clamped", "[Editor][S198]") {
    CameraRigEditorV1 cr;
    Crv1Rig r; r.id = 1; r.name = "R";
    cr.addRig(r);
    cr.setFov(1, 200.f);
    REQUIRE(cr.findRig(1)->fov == Approx(170.f));
    cr.setFov(1, 5.f);
    REQUIRE(cr.findRig(1)->fov == Approx(10.f));
}

TEST_CASE("CameraRigEditorV1 activeCount", "[Editor][S198]") {
    CameraRigEditorV1 cr;
    Crv1Rig r1; r1.id = 1; r1.name = "A";
    Crv1Rig r2; r2.id = 2; r2.name = "B";
    cr.addRig(r1); cr.addRig(r2);
    cr.setState(2, Crv1RigState::Disabled);
    REQUIRE(cr.activeCount() == 1);
}

TEST_CASE("CameraRigEditorV1 countByType", "[Editor][S198]") {
    CameraRigEditorV1 cr;
    Crv1Rig r1; r1.id = 1; r1.name = "A"; r1.type = Crv1RigType::Rail;
    Crv1Rig r2; r2.id = 2; r2.name = "B"; r2.type = Crv1RigType::Orbit;
    Crv1Rig r3; r3.id = 3; r3.name = "C"; r3.type = Crv1RigType::Rail;
    cr.addRig(r1); cr.addRig(r2); cr.addRig(r3);
    REQUIRE(cr.countByType(Crv1RigType::Rail) == 2);
    REQUIRE(cr.countByType(Crv1RigType::Orbit) == 1);
}

TEST_CASE("CameraRigEditorV1 onChange callback", "[Editor][S198]") {
    CameraRigEditorV1 cr;
    uint64_t notified = 0;
    cr.setOnChange([&](uint64_t id){ notified = id; });
    Crv1Rig r; r.id = 5; r.name = "X";
    cr.addRig(r);
    REQUIRE(notified == 5);
}

TEST_CASE("crv1RigTypeName all values", "[Editor][S198]") {
    REQUIRE(std::string(crv1RigTypeName(Crv1RigType::Rail))   == "Rail");
    REQUIRE(std::string(crv1RigTypeName(Crv1RigType::Crane))  == "Crane");
    REQUIRE(std::string(crv1RigTypeName(Crv1RigType::Orbit))  == "Orbit");
    REQUIRE(std::string(crv1RigTypeName(Crv1RigType::Follow)) == "Follow");
    REQUIRE(std::string(crv1RigTypeName(Crv1RigType::Static)) == "Static");
    REQUIRE(std::string(crv1RigTypeName(Crv1RigType::Shake))  == "Shake");
}

// ── LevelStreamEditorV1 ─────────────────────────────────────────────────────

TEST_CASE("Lsv1Chunk validity", "[Editor][S198]") {
    Lsv1Chunk c;
    REQUIRE(!c.isValid());
    c.id = 1; c.name = "Chunk0";
    REQUIRE(c.isValid());
}

TEST_CASE("LevelStreamEditorV1 addChunk and chunkCount", "[Editor][S198]") {
    LevelStreamEditorV1 ls;
    REQUIRE(ls.chunkCount() == 0);
    Lsv1Chunk c; c.id = 1; c.name = "C1";
    REQUIRE(ls.addChunk(c));
    REQUIRE(ls.chunkCount() == 1);
}

TEST_CASE("LevelStreamEditorV1 addChunk invalid fails", "[Editor][S198]") {
    LevelStreamEditorV1 ls;
    REQUIRE(!ls.addChunk(Lsv1Chunk{}));
}

TEST_CASE("LevelStreamEditorV1 addChunk duplicate fails", "[Editor][S198]") {
    LevelStreamEditorV1 ls;
    Lsv1Chunk c; c.id = 1; c.name = "A";
    ls.addChunk(c);
    REQUIRE(!ls.addChunk(c));
}

TEST_CASE("LevelStreamEditorV1 removeChunk", "[Editor][S198]") {
    LevelStreamEditorV1 ls;
    Lsv1Chunk c; c.id = 2; c.name = "B";
    ls.addChunk(c);
    REQUIRE(ls.removeChunk(2));
    REQUIRE(ls.chunkCount() == 0);
    REQUIRE(!ls.removeChunk(2));
}

TEST_CASE("LevelStreamEditorV1 findChunk", "[Editor][S198]") {
    LevelStreamEditorV1 ls;
    Lsv1Chunk c; c.id = 3; c.name = "C";
    ls.addChunk(c);
    REQUIRE(ls.findChunk(3) != nullptr);
    REQUIRE(ls.findChunk(99) == nullptr);
}

TEST_CASE("LevelStreamEditorV1 setState", "[Editor][S198]") {
    LevelStreamEditorV1 ls;
    Lsv1Chunk c; c.id = 1; c.name = "C";
    ls.addChunk(c);
    REQUIRE(ls.setState(1, Lsv1StreamState::Loaded));
    REQUIRE(ls.findChunk(1)->isLoaded());
}

TEST_CASE("LevelStreamEditorV1 setPriority", "[Editor][S198]") {
    LevelStreamEditorV1 ls;
    Lsv1Chunk c; c.id = 1; c.name = "C";
    ls.addChunk(c);
    REQUIRE(ls.setPriority(1, Lsv1Priority::Critical));
    REQUIRE(ls.findChunk(1)->priority == Lsv1Priority::Critical);
}

TEST_CASE("LevelStreamEditorV1 addDependency", "[Editor][S198]") {
    LevelStreamEditorV1 ls;
    Lsv1Chunk c; c.id = 1; c.name = "C";
    ls.addChunk(c);
    REQUIRE(ls.addDependency(1, 42));
    REQUIRE(ls.findChunk(1)->dependencies.size() == 1);
    REQUIRE(!ls.addDependency(1, 42)); // duplicate dep
}

TEST_CASE("LevelStreamEditorV1 loadedCount", "[Editor][S198]") {
    LevelStreamEditorV1 ls;
    Lsv1Chunk c1; c1.id = 1; c1.name = "A";
    Lsv1Chunk c2; c2.id = 2; c2.name = "B";
    ls.addChunk(c1); ls.addChunk(c2);
    ls.setState(1, Lsv1StreamState::Loaded);
    REQUIRE(ls.loadedCount() == 1);
}

TEST_CASE("LevelStreamEditorV1 totalSizeMB", "[Editor][S198]") {
    LevelStreamEditorV1 ls;
    Lsv1Chunk c1; c1.id = 1; c1.name = "A"; c1.sizeMB = 10.f;
    Lsv1Chunk c2; c2.id = 2; c2.name = "B"; c2.sizeMB = 25.5f;
    ls.addChunk(c1); ls.addChunk(c2);
    REQUIRE(ls.totalSizeMB() == Approx(35.5f));
}

TEST_CASE("LevelStreamEditorV1 countByPriority", "[Editor][S198]") {
    LevelStreamEditorV1 ls;
    Lsv1Chunk c1; c1.id = 1; c1.name = "A"; c1.priority = Lsv1Priority::High;
    Lsv1Chunk c2; c2.id = 2; c2.name = "B"; c2.priority = Lsv1Priority::Low;
    Lsv1Chunk c3; c3.id = 3; c3.name = "C"; c3.priority = Lsv1Priority::High;
    ls.addChunk(c1); ls.addChunk(c2); ls.addChunk(c3);
    REQUIRE(ls.countByPriority(Lsv1Priority::High) == 2);
    REQUIRE(ls.countByPriority(Lsv1Priority::Low) == 1);
}

TEST_CASE("LevelStreamEditorV1 onChange callback", "[Editor][S198]") {
    LevelStreamEditorV1 ls;
    uint64_t notified = 0;
    ls.setOnChange([&](uint64_t id){ notified = id; });
    Lsv1Chunk c; c.id = 7; c.name = "X";
    ls.addChunk(c);
    REQUIRE(notified == 7);
}

TEST_CASE("lsv1StreamStateName all values", "[Editor][S198]") {
    REQUIRE(std::string(lsv1StreamStateName(Lsv1StreamState::Unloaded))  == "Unloaded");
    REQUIRE(std::string(lsv1StreamStateName(Lsv1StreamState::Loading))   == "Loading");
    REQUIRE(std::string(lsv1StreamStateName(Lsv1StreamState::Loaded))    == "Loaded");
    REQUIRE(std::string(lsv1StreamStateName(Lsv1StreamState::Streaming)) == "Streaming");
    REQUIRE(std::string(lsv1StreamStateName(Lsv1StreamState::Error))     == "Error");
}

TEST_CASE("lsv1PriorityName all values", "[Editor][S198]") {
    REQUIRE(std::string(lsv1PriorityName(Lsv1Priority::Critical))   == "Critical");
    REQUIRE(std::string(lsv1PriorityName(Lsv1Priority::High))       == "High");
    REQUIRE(std::string(lsv1PriorityName(Lsv1Priority::Normal))     == "Normal");
    REQUIRE(std::string(lsv1PriorityName(Lsv1Priority::Low))        == "Low");
    REQUIRE(std::string(lsv1PriorityName(Lsv1Priority::Background)) == "Background");
}

// ── AtmosphereEditorV1 ──────────────────────────────────────────────────────

TEST_CASE("Aev1AtmoPreset validity", "[Editor][S198]") {
    Aev1AtmoPreset p;
    REQUIRE(!p.isValid());
    p.id = 1; p.name = "Sunny";
    REQUIRE(p.isValid());
}

TEST_CASE("AtmosphereEditorV1 addPreset and presetCount", "[Editor][S198]") {
    AtmosphereEditorV1 ae;
    REQUIRE(ae.presetCount() == 0);
    Aev1AtmoPreset p; p.id = 1; p.name = "P1";
    REQUIRE(ae.addPreset(p));
    REQUIRE(ae.presetCount() == 1);
}

TEST_CASE("AtmosphereEditorV1 addPreset invalid fails", "[Editor][S198]") {
    AtmosphereEditorV1 ae;
    REQUIRE(!ae.addPreset(Aev1AtmoPreset{}));
}

TEST_CASE("AtmosphereEditorV1 addPreset duplicate fails", "[Editor][S198]") {
    AtmosphereEditorV1 ae;
    Aev1AtmoPreset p; p.id = 1; p.name = "A";
    ae.addPreset(p);
    REQUIRE(!ae.addPreset(p));
}

TEST_CASE("AtmosphereEditorV1 removePreset", "[Editor][S198]") {
    AtmosphereEditorV1 ae;
    Aev1AtmoPreset p; p.id = 2; p.name = "B";
    ae.addPreset(p);
    REQUIRE(ae.removePreset(2));
    REQUIRE(ae.presetCount() == 0);
    REQUIRE(!ae.removePreset(2));
}

TEST_CASE("AtmosphereEditorV1 findPreset", "[Editor][S198]") {
    AtmosphereEditorV1 ae;
    Aev1AtmoPreset p; p.id = 3; p.name = "C";
    ae.addPreset(p);
    REQUIRE(ae.findPreset(3) != nullptr);
    REQUIRE(ae.findPreset(99) == nullptr);
}

TEST_CASE("AtmosphereEditorV1 selectPreset", "[Editor][S198]") {
    AtmosphereEditorV1 ae;
    Aev1AtmoPreset p; p.id = 1; p.name = "P";
    ae.addPreset(p);
    REQUIRE(ae.selectPreset(1));
    REQUIRE(ae.activePresetId() == 1);
    REQUIRE(!ae.selectPreset(99));
}

TEST_CASE("AtmosphereEditorV1 setFogDensity", "[Editor][S198]") {
    AtmosphereEditorV1 ae;
    Aev1AtmoPreset p; p.id = 1; p.name = "P";
    ae.addPreset(p);
    REQUIRE(ae.setFogDensity(1, 0.05f));
    REQUIRE(ae.findPreset(1)->fogDensity == Approx(0.05f));
}

TEST_CASE("AtmosphereEditorV1 setFogDensity clamped to zero", "[Editor][S198]") {
    AtmosphereEditorV1 ae;
    Aev1AtmoPreset p; p.id = 1; p.name = "P";
    ae.addPreset(p);
    ae.setFogDensity(1, -5.f);
    REQUIRE(ae.findPreset(1)->fogDensity == Approx(0.f));
}

TEST_CASE("AtmosphereEditorV1 setSunIntensity", "[Editor][S198]") {
    AtmosphereEditorV1 ae;
    Aev1AtmoPreset p; p.id = 1; p.name = "P";
    ae.addPreset(p);
    REQUIRE(ae.setSunIntensity(1, 2.5f));
    REQUIRE(ae.findPreset(1)->sunIntensity == Approx(2.5f));
}

TEST_CASE("AtmosphereEditorV1 setTimeOfDay", "[Editor][S198]") {
    AtmosphereEditorV1 ae;
    Aev1AtmoPreset p; p.id = 1; p.name = "P";
    ae.addPreset(p);
    REQUIRE(ae.setTimeOfDay(1, Aev1TimeOfDay::Dusk));
    REQUIRE(ae.findPreset(1)->timeOfDay == Aev1TimeOfDay::Dusk);
}

TEST_CASE("AtmosphereEditorV1 countByTimeOfDay", "[Editor][S198]") {
    AtmosphereEditorV1 ae;
    Aev1AtmoPreset p1; p1.id = 1; p1.name = "A"; p1.timeOfDay = Aev1TimeOfDay::Dawn;
    Aev1AtmoPreset p2; p2.id = 2; p2.name = "B"; p2.timeOfDay = Aev1TimeOfDay::Night;
    Aev1AtmoPreset p3; p3.id = 3; p3.name = "C"; p3.timeOfDay = Aev1TimeOfDay::Dawn;
    ae.addPreset(p1); ae.addPreset(p2); ae.addPreset(p3);
    REQUIRE(ae.countByTimeOfDay(Aev1TimeOfDay::Dawn) == 2);
    REQUIRE(ae.countByTimeOfDay(Aev1TimeOfDay::Night) == 1);
}

TEST_CASE("AtmosphereEditorV1 countByAtmoType", "[Editor][S198]") {
    AtmosphereEditorV1 ae;
    Aev1AtmoPreset p1; p1.id = 1; p1.name = "A"; p1.scatterType = Aev1AtmoType::Rayleigh;
    Aev1AtmoPreset p2; p2.id = 2; p2.name = "B"; p2.scatterType = Aev1AtmoType::Mie;
    ae.addPreset(p1); ae.addPreset(p2);
    REQUIRE(ae.countByAtmoType(Aev1AtmoType::Rayleigh) == 1);
    REQUIRE(ae.countByAtmoType(Aev1AtmoType::Mie) == 1);
}

TEST_CASE("AtmosphereEditorV1 onChange callback", "[Editor][S198]") {
    AtmosphereEditorV1 ae;
    uint64_t notified = 0;
    ae.setOnChange([&](uint64_t id){ notified = id; });
    Aev1AtmoPreset p; p.id = 5; p.name = "X";
    ae.addPreset(p);
    REQUIRE(notified == 5);
}

TEST_CASE("aev1AtmoTypeName all values", "[Editor][S198]") {
    REQUIRE(std::string(aev1AtmoTypeName(Aev1AtmoType::Rayleigh)) == "Rayleigh");
    REQUIRE(std::string(aev1AtmoTypeName(Aev1AtmoType::Mie))      == "Mie");
    REQUIRE(std::string(aev1AtmoTypeName(Aev1AtmoType::Combined)) == "Combined");
    REQUIRE(std::string(aev1AtmoTypeName(Aev1AtmoType::Custom))   == "Custom");
}

TEST_CASE("aev1FogTypeName all values", "[Editor][S198]") {
    REQUIRE(std::string(aev1FogTypeName(Aev1FogType::Linear))             == "Linear");
    REQUIRE(std::string(aev1FogTypeName(Aev1FogType::Exponential))        == "Exponential");
    REQUIRE(std::string(aev1FogTypeName(Aev1FogType::ExponentialSquared)) == "ExponentialSquared");
    REQUIRE(std::string(aev1FogTypeName(Aev1FogType::Height))             == "Height");
}

TEST_CASE("aev1TimeOfDayName all values", "[Editor][S198]") {
    REQUIRE(std::string(aev1TimeOfDayName(Aev1TimeOfDay::Dawn))      == "Dawn");
    REQUIRE(std::string(aev1TimeOfDayName(Aev1TimeOfDay::Morning))   == "Morning");
    REQUIRE(std::string(aev1TimeOfDayName(Aev1TimeOfDay::Noon))      == "Noon");
    REQUIRE(std::string(aev1TimeOfDayName(Aev1TimeOfDay::Afternoon)) == "Afternoon");
    REQUIRE(std::string(aev1TimeOfDayName(Aev1TimeOfDay::Dusk))      == "Dusk");
    REQUIRE(std::string(aev1TimeOfDayName(Aev1TimeOfDay::Night))     == "Night");
}
