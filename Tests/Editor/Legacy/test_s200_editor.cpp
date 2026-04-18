// S200 editor tests: TagSystemEditorV1, DebugDrawEditorV1, ObjectPoolEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/TagSystemEditorV1.h"
#include "NF/Editor/DebugDrawEditorV1.h"
#include "NF/Editor/ObjectPoolEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── TagSystemEditorV1 ───────────────────────────────────────────────────────

TEST_CASE("Tsv1Tag validity", "[Editor][S200]") {
    Tsv1Tag t;
    REQUIRE(!t.isValid());
    t.id = 1; t.name = "Enemy";
    REQUIRE(t.isValid());
}

TEST_CASE("TagSystemEditorV1 addTag and tagCount", "[Editor][S200]") {
    TagSystemEditorV1 te;
    REQUIRE(te.tagCount() == 0);
    Tsv1Tag t; t.id = 1; t.name = "Tag1";
    REQUIRE(te.addTag(t));
    REQUIRE(te.tagCount() == 1);
}

TEST_CASE("TagSystemEditorV1 addTag invalid fails", "[Editor][S200]") {
    TagSystemEditorV1 te;
    REQUIRE(!te.addTag(Tsv1Tag{}));
}

TEST_CASE("TagSystemEditorV1 addTag duplicate fails", "[Editor][S200]") {
    TagSystemEditorV1 te;
    Tsv1Tag t; t.id = 1; t.name = "A";
    te.addTag(t);
    REQUIRE(!te.addTag(t));
}

TEST_CASE("TagSystemEditorV1 removeTag", "[Editor][S200]") {
    TagSystemEditorV1 te;
    Tsv1Tag t; t.id = 2; t.name = "B";
    te.addTag(t);
    REQUIRE(te.removeTag(2));
    REQUIRE(te.tagCount() == 0);
    REQUIRE(!te.removeTag(2));
}

TEST_CASE("TagSystemEditorV1 findTag", "[Editor][S200]") {
    TagSystemEditorV1 te;
    Tsv1Tag t; t.id = 3; t.name = "C";
    te.addTag(t);
    REQUIRE(te.findTag(3) != nullptr);
    REQUIRE(te.findTag(99) == nullptr);
}

TEST_CASE("TagSystemEditorV1 setCategory", "[Editor][S200]") {
    TagSystemEditorV1 te;
    Tsv1Tag t; t.id = 1; t.name = "T";
    te.addTag(t);
    REQUIRE(te.setCategory(1, "Gameplay"));
    REQUIRE(te.findTag(1)->category == "Gameplay");
}

TEST_CASE("TagSystemEditorV1 setColor", "[Editor][S200]") {
    TagSystemEditorV1 te;
    Tsv1Tag t; t.id = 1; t.name = "T";
    te.addTag(t);
    REQUIRE(te.setColor(1, 0xFF0000FFu));
    REQUIRE(te.findTag(1)->color == 0xFF0000FFu);
}

TEST_CASE("TagSystemEditorV1 hasTag", "[Editor][S200]") {
    TagSystemEditorV1 te;
    Tsv1Tag t; t.id = 1; t.name = "Player";
    te.addTag(t);
    REQUIRE(te.hasTag("Player"));
    REQUIRE(!te.hasTag("Enemy"));
}

TEST_CASE("TagSystemEditorV1 countByCategory", "[Editor][S200]") {
    TagSystemEditorV1 te;
    Tsv1Tag t1; t1.id = 1; t1.name = "A"; t1.category = "Gameplay";
    Tsv1Tag t2; t2.id = 2; t2.name = "B"; t2.category = "Audio";
    Tsv1Tag t3; t3.id = 3; t3.name = "C"; t3.category = "Gameplay";
    te.addTag(t1); te.addTag(t2); te.addTag(t3);
    REQUIRE(te.countByCategory("Gameplay") == 2);
    REQUIRE(te.countByCategory("Audio") == 1);
    REQUIRE(te.countByCategory("Visual") == 0);
}

TEST_CASE("TagSystemEditorV1 onChange callback", "[Editor][S200]") {
    TagSystemEditorV1 te;
    uint64_t notified = 0;
    te.setOnChange([&](uint64_t id){ notified = id; });
    Tsv1Tag t; t.id = 5; t.name = "X";
    te.addTag(t);
    REQUIRE(notified == 5);
}

// ── DebugDrawEditorV1 ───────────────────────────────────────────────────────

TEST_CASE("Ddv1Shape validity", "[Editor][S200]") {
    Ddv1Shape s;
    REQUIRE(!s.isValid());
    s.id = 1; s.name = "GridLine";
    REQUIRE(s.isValid());
}

TEST_CASE("DebugDrawEditorV1 addShape and shapeCount", "[Editor][S200]") {
    DebugDrawEditorV1 de;
    REQUIRE(de.shapeCount() == 0);
    Ddv1Shape s; s.id = 1; s.name = "S1";
    REQUIRE(de.addShape(s));
    REQUIRE(de.shapeCount() == 1);
}

TEST_CASE("DebugDrawEditorV1 addShape invalid fails", "[Editor][S200]") {
    DebugDrawEditorV1 de;
    REQUIRE(!de.addShape(Ddv1Shape{}));
}

TEST_CASE("DebugDrawEditorV1 addShape duplicate fails", "[Editor][S200]") {
    DebugDrawEditorV1 de;
    Ddv1Shape s; s.id = 1; s.name = "A";
    de.addShape(s);
    REQUIRE(!de.addShape(s));
}

TEST_CASE("DebugDrawEditorV1 removeShape", "[Editor][S200]") {
    DebugDrawEditorV1 de;
    Ddv1Shape s; s.id = 2; s.name = "B";
    de.addShape(s);
    REQUIRE(de.removeShape(2));
    REQUIRE(de.shapeCount() == 0);
    REQUIRE(!de.removeShape(2));
}

TEST_CASE("DebugDrawEditorV1 findShape", "[Editor][S200]") {
    DebugDrawEditorV1 de;
    Ddv1Shape s; s.id = 3; s.name = "C";
    de.addShape(s);
    REQUIRE(de.findShape(3) != nullptr);
    REQUIRE(de.findShape(99) == nullptr);
}

TEST_CASE("DebugDrawEditorV1 setState", "[Editor][S200]") {
    DebugDrawEditorV1 de;
    Ddv1Shape s; s.id = 1; s.name = "S";
    de.addShape(s);
    REQUIRE(de.setState(1, false));
    REQUIRE(!de.findShape(1)->enabled);
    REQUIRE(de.setState(1, true));
    REQUIRE(de.findShape(1)->enabled);
}

TEST_CASE("DebugDrawEditorV1 setDuration clamped", "[Editor][S200]") {
    DebugDrawEditorV1 de;
    Ddv1Shape s; s.id = 1; s.name = "S";
    de.addShape(s);
    de.setDuration(1, 5000.f);
    REQUIRE(de.findShape(1)->duration == Approx(3600.f));
    de.setDuration(1, -1.f);
    REQUIRE(de.findShape(1)->duration == Approx(0.f));
}

TEST_CASE("DebugDrawEditorV1 enabledCount", "[Editor][S200]") {
    DebugDrawEditorV1 de;
    Ddv1Shape s1; s1.id = 1; s1.name = "A";
    Ddv1Shape s2; s2.id = 2; s2.name = "B";
    de.addShape(s1); de.addShape(s2);
    de.setState(2, false);
    REQUIRE(de.enabledCount() == 1);
}

TEST_CASE("DebugDrawEditorV1 clearAll", "[Editor][S200]") {
    DebugDrawEditorV1 de;
    Ddv1Shape s1; s1.id = 1; s1.name = "A";
    Ddv1Shape s2; s2.id = 2; s2.name = "B";
    de.addShape(s1); de.addShape(s2);
    de.clearAll();
    REQUIRE(de.shapeCount() == 0);
}

TEST_CASE("DebugDrawEditorV1 countByType", "[Editor][S200]") {
    DebugDrawEditorV1 de;
    Ddv1Shape s1; s1.id = 1; s1.name = "A"; s1.type = Ddv1ShapeType::Box;
    Ddv1Shape s2; s2.id = 2; s2.name = "B"; s2.type = Ddv1ShapeType::Sphere;
    Ddv1Shape s3; s3.id = 3; s3.name = "C"; s3.type = Ddv1ShapeType::Box;
    de.addShape(s1); de.addShape(s2); de.addShape(s3);
    REQUIRE(de.countByType(Ddv1ShapeType::Box) == 2);
    REQUIRE(de.countByType(Ddv1ShapeType::Sphere) == 1);
}

TEST_CASE("DebugDrawEditorV1 onChange callback", "[Editor][S200]") {
    DebugDrawEditorV1 de;
    uint64_t notified = 0;
    de.setOnChange([&](uint64_t id){ notified = id; });
    Ddv1Shape s; s.id = 7; s.name = "X";
    de.addShape(s);
    REQUIRE(notified == 7);
}

TEST_CASE("ddv1ShapeTypeName all values", "[Editor][S200]") {
    REQUIRE(std::string(ddv1ShapeTypeName(Ddv1ShapeType::Line))    == "Line");
    REQUIRE(std::string(ddv1ShapeTypeName(Ddv1ShapeType::Box))     == "Box");
    REQUIRE(std::string(ddv1ShapeTypeName(Ddv1ShapeType::Sphere))  == "Sphere");
    REQUIRE(std::string(ddv1ShapeTypeName(Ddv1ShapeType::Arrow))   == "Arrow");
    REQUIRE(std::string(ddv1ShapeTypeName(Ddv1ShapeType::Capsule)) == "Capsule");
    REQUIRE(std::string(ddv1ShapeTypeName(Ddv1ShapeType::Grid))    == "Grid");
}

// ── ObjectPoolEditorV1 ──────────────────────────────────────────────────────

TEST_CASE("Opv1Pool validity", "[Editor][S200]") {
    Opv1Pool p;
    REQUIRE(!p.isValid());
    p.id = 1; p.name = "BulletPool";
    REQUIRE(p.isValid());
}

TEST_CASE("ObjectPoolEditorV1 addPool and poolCount", "[Editor][S200]") {
    ObjectPoolEditorV1 oe;
    REQUIRE(oe.poolCount() == 0);
    Opv1Pool p; p.id = 1; p.name = "P1";
    REQUIRE(oe.addPool(p));
    REQUIRE(oe.poolCount() == 1);
}

TEST_CASE("ObjectPoolEditorV1 addPool invalid fails", "[Editor][S200]") {
    ObjectPoolEditorV1 oe;
    REQUIRE(!oe.addPool(Opv1Pool{}));
}

TEST_CASE("ObjectPoolEditorV1 addPool duplicate fails", "[Editor][S200]") {
    ObjectPoolEditorV1 oe;
    Opv1Pool p; p.id = 1; p.name = "A";
    oe.addPool(p);
    REQUIRE(!oe.addPool(p));
}

TEST_CASE("ObjectPoolEditorV1 removePool", "[Editor][S200]") {
    ObjectPoolEditorV1 oe;
    Opv1Pool p; p.id = 2; p.name = "B";
    oe.addPool(p);
    REQUIRE(oe.removePool(2));
    REQUIRE(oe.poolCount() == 0);
    REQUIRE(!oe.removePool(2));
}

TEST_CASE("ObjectPoolEditorV1 findPool", "[Editor][S200]") {
    ObjectPoolEditorV1 oe;
    Opv1Pool p; p.id = 3; p.name = "C";
    oe.addPool(p);
    REQUIRE(oe.findPool(3) != nullptr);
    REQUIRE(oe.findPool(99) == nullptr);
}

TEST_CASE("ObjectPoolEditorV1 setState", "[Editor][S200]") {
    ObjectPoolEditorV1 oe;
    Opv1Pool p; p.id = 1; p.name = "P";
    oe.addPool(p);
    REQUIRE(oe.setState(1, Opv1PoolState::Full));
    REQUIRE(oe.findPool(1)->state == Opv1PoolState::Full);
}

TEST_CASE("ObjectPoolEditorV1 setInitialSize clamped to min 1", "[Editor][S200]") {
    ObjectPoolEditorV1 oe;
    Opv1Pool p; p.id = 1; p.name = "P";
    oe.addPool(p);
    REQUIRE(oe.setInitialSize(1, 0));
    REQUIRE(oe.findPool(1)->initialSize == 1);
    REQUIRE(oe.setInitialSize(1, 10));
    REQUIRE(oe.findPool(1)->initialSize == 10);
}

TEST_CASE("ObjectPoolEditorV1 setMaxSize respects initialSize", "[Editor][S200]") {
    ObjectPoolEditorV1 oe;
    Opv1Pool p; p.id = 1; p.name = "P"; p.initialSize = 5;
    oe.addPool(p);
    oe.setMaxSize(1, 2); // below initialSize, should clamp
    REQUIRE(oe.findPool(1)->maxSize == 5);
    oe.setMaxSize(1, 20);
    REQUIRE(oe.findPool(1)->maxSize == 20);
}

TEST_CASE("ObjectPoolEditorV1 setGrowBy clamped to min 1", "[Editor][S200]") {
    ObjectPoolEditorV1 oe;
    Opv1Pool p; p.id = 1; p.name = "P";
    oe.addPool(p);
    REQUIRE(oe.setGrowBy(1, 0));
    REQUIRE(oe.findPool(1)->growBy == 1);
    REQUIRE(oe.setGrowBy(1, 5));
    REQUIRE(oe.findPool(1)->growBy == 5);
}

TEST_CASE("ObjectPoolEditorV1 activeCount", "[Editor][S200]") {
    ObjectPoolEditorV1 oe;
    Opv1Pool p1; p1.id = 1; p1.name = "A";
    Opv1Pool p2; p2.id = 2; p2.name = "B";
    oe.addPool(p1); oe.addPool(p2);
    oe.setState(2, Opv1PoolState::Inactive);
    REQUIRE(oe.activeCount() == 1);
}

TEST_CASE("ObjectPoolEditorV1 totalCapacity", "[Editor][S200]") {
    ObjectPoolEditorV1 oe;
    Opv1Pool p1; p1.id = 1; p1.name = "A"; p1.maxSize = 10;
    Opv1Pool p2; p2.id = 2; p2.name = "B"; p2.maxSize = 25;
    oe.addPool(p1); oe.addPool(p2);
    REQUIRE(oe.totalCapacity() == 35);
}

TEST_CASE("ObjectPoolEditorV1 onChange callback", "[Editor][S200]") {
    ObjectPoolEditorV1 oe;
    uint64_t notified = 0;
    oe.setOnChange([&](uint64_t id){ notified = id; });
    Opv1Pool p; p.id = 9; p.name = "X";
    oe.addPool(p);
    REQUIRE(notified == 9);
}

TEST_CASE("opv1PoolStateName all values", "[Editor][S200]") {
    REQUIRE(std::string(opv1PoolStateName(Opv1PoolState::Active))   == "Active");
    REQUIRE(std::string(opv1PoolStateName(Opv1PoolState::Inactive)) == "Inactive");
    REQUIRE(std::string(opv1PoolStateName(Opv1PoolState::Full))     == "Full");
}
