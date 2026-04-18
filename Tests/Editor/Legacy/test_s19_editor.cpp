#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/SceneSnapshot.h"

using namespace NF;

// --- SceneSnapshotType names ---

TEST_CASE("SceneSnapshotType names cover all 8 values", "[Editor][S19]") {
    REQUIRE(std::string(sceneSnapshotTypeName(SceneSnapshotType::Full))     == "Full");
    REQUIRE(std::string(sceneSnapshotTypeName(SceneSnapshotType::Delta))    == "Delta");
    REQUIRE(std::string(sceneSnapshotTypeName(SceneSnapshotType::Lighting)) == "Lighting");
    REQUIRE(std::string(sceneSnapshotTypeName(SceneSnapshotType::Physics))  == "Physics");
    REQUIRE(std::string(sceneSnapshotTypeName(SceneSnapshotType::AI))       == "AI");
    REQUIRE(std::string(sceneSnapshotTypeName(SceneSnapshotType::Audio))    == "Audio");
    REQUIRE(std::string(sceneSnapshotTypeName(SceneSnapshotType::Visual))   == "Visual");
    REQUIRE(std::string(sceneSnapshotTypeName(SceneSnapshotType::Meta))     == "Meta");
}

// --- SceneSnapshotFrame predicates ---

TEST_CASE("SceneSnapshotFrame state predicates", "[Editor][S19]") {
    SceneSnapshotFrame f;
    f.id = "f1";
    f.state = SceneSnapshotState::Valid;
    REQUIRE(f.isValid());
    REQUIRE_FALSE(f.isOutdated());
    REQUIRE_FALSE(f.isCorrupted());
    REQUIRE_FALSE(f.isPartial());
}

TEST_CASE("SceneSnapshotFrame markOutdated only from Valid", "[Editor][S19]") {
    SceneSnapshotFrame f;
    f.id = "f1"; f.state = SceneSnapshotState::Valid;
    f.markOutdated();
    REQUIRE(f.isOutdated());

    // already outdated — markOutdated is idempotent on non-valid states
    f.state = SceneSnapshotState::Corrupted;
    f.markOutdated();
    REQUIRE(f.isCorrupted()); // unchanged
}

TEST_CASE("SceneSnapshotFrame markCorrupted always succeeds", "[Editor][S19]") {
    SceneSnapshotFrame f;
    f.id = "f1"; f.state = SceneSnapshotState::Valid;
    f.markCorrupted();
    REQUIRE(f.isCorrupted());
}

// --- SceneSnapshotHistory ---

TEST_CASE("SceneSnapshotHistory push + duplicate rejection", "[Editor][S19]") {
    SceneSnapshotHistory hist;
    SceneSnapshotFrame f1; f1.id = "s1";
    SceneSnapshotFrame f1dup; f1dup.id = "s1"; // duplicate

    REQUIRE(hist.push(f1));
    REQUIRE_FALSE(hist.push(f1dup));
    REQUIRE(hist.frameCount() == 1);
}

TEST_CASE("SceneSnapshotHistory remove", "[Editor][S19]") {
    SceneSnapshotHistory hist;
    SceneSnapshotFrame f; f.id = "s1";
    hist.push(f);
    REQUIRE(hist.remove("s1"));
    REQUIRE(hist.empty());
    REQUIRE_FALSE(hist.remove("s1")); // already gone
}

TEST_CASE("SceneSnapshotHistory find", "[Editor][S19]") {
    SceneSnapshotHistory hist;
    SceneSnapshotFrame f; f.id = "s1"; f.label = "initial";
    hist.push(f);

    auto* found = hist.find("s1");
    REQUIRE(found != nullptr);
    REQUIRE(found->label == "initial");
    REQUIRE(hist.find("missing") == nullptr);
}

TEST_CASE("SceneSnapshotHistory latest returns last pushed", "[Editor][S19]") {
    SceneSnapshotHistory hist;
    REQUIRE(hist.latest() == nullptr);

    SceneSnapshotFrame f1; f1.id = "s1";
    SceneSnapshotFrame f2; f2.id = "s2";
    hist.push(f1);
    hist.push(f2);
    REQUIRE(hist.latest()->id == "s2");
}

TEST_CASE("SceneSnapshotHistory markAllOutdated", "[Editor][S19]") {
    SceneSnapshotHistory hist;
    SceneSnapshotFrame f1; f1.id = "s1"; f1.state = SceneSnapshotState::Valid;
    SceneSnapshotFrame f2; f2.id = "s2"; f2.state = SceneSnapshotState::Valid;
    hist.push(f1);
    hist.push(f2);

    hist.markAllOutdated();
    REQUIRE(hist.validCount() == 0);
}

TEST_CASE("SceneSnapshotHistory validCount + corruptedCount", "[Editor][S19]") {
    SceneSnapshotHistory hist;
    SceneSnapshotFrame v; v.id = "v1"; v.state = SceneSnapshotState::Valid;
    SceneSnapshotFrame c; c.id = "c1"; c.state = SceneSnapshotState::Corrupted;
    SceneSnapshotFrame p; p.id = "p1"; p.state = SceneSnapshotState::Partial;
    hist.push(v);
    hist.push(c);
    hist.push(p);

    REQUIRE(hist.validCount()     == 1);
    REQUIRE(hist.corruptedCount() == 1);
}

TEST_CASE("SceneSnapshotHistory totalDataSize", "[Editor][S19]") {
    SceneSnapshotHistory hist;
    SceneSnapshotFrame f1; f1.id = "s1"; f1.dataSize = 1024;
    SceneSnapshotFrame f2; f2.id = "s2"; f2.dataSize = 2048;
    hist.push(f1);
    hist.push(f2);
    REQUIRE(hist.totalDataSize() == 3072);
}

// --- SceneSnapshotSystem ---

TEST_CASE("SceneSnapshotSystem init/shutdown lifecycle", "[Editor][S19]") {
    SceneSnapshotSystem sys;
    REQUIRE_FALSE(sys.isInitialized());
    sys.init();
    REQUIRE(sys.isInitialized());
    sys.shutdown();
    REQUIRE_FALSE(sys.isInitialized());
    REQUIRE(sys.frameCount() == 0);
}

TEST_CASE("SceneSnapshotSystem capture blocked before init", "[Editor][S19]") {
    SceneSnapshotSystem sys;
    SceneSnapshotFrame f; f.id = "x";
    REQUIRE_FALSE(sys.capture(f));
}

TEST_CASE("SceneSnapshotSystem capture + discard + find", "[Editor][S19]") {
    SceneSnapshotSystem sys;
    sys.init();

    SceneSnapshotFrame f; f.id = "snap1"; f.dataSize = 512;
    REQUIRE(sys.capture(f));
    REQUIRE(sys.frameCount() == 1);

    REQUIRE(sys.find("snap1") != nullptr);
    REQUIRE(sys.discard("snap1"));
    REQUIRE(sys.find("snap1") == nullptr);
    REQUIRE(sys.frameCount() == 0);
}

TEST_CASE("SceneSnapshotSystem latest + invalidateAll", "[Editor][S19]") {
    SceneSnapshotSystem sys;
    sys.init();

    SceneSnapshotFrame f1; f1.id = "s1"; f1.state = SceneSnapshotState::Valid;
    SceneSnapshotFrame f2; f2.id = "s2"; f2.state = SceneSnapshotState::Valid;
    sys.capture(f1);
    sys.capture(f2);

    REQUIRE(sys.latest()->id == "s2");
    REQUIRE(sys.validCount() == 2);

    sys.invalidateAll();
    REQUIRE(sys.validCount() == 0);
}

TEST_CASE("SceneSnapshotSystem totalDataSize aggregation", "[Editor][S19]") {
    SceneSnapshotSystem sys;
    sys.init();

    SceneSnapshotFrame f1; f1.id = "s1"; f1.dataSize = 100;
    SceneSnapshotFrame f2; f2.id = "s2"; f2.dataSize = 200;
    sys.capture(f1);
    sys.capture(f2);

    REQUIRE(sys.totalDataSize() == 300);
}
