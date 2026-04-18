#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "NF/Animation/Animation.h"

using namespace NF;

// ── BoneChain ────────────────────────────────────────────────────

TEST_CASE("BoneChain validity", "[Animation][S5]") {
    BoneChain chain;
    REQUIRE_FALSE(chain.isValid());

    chain.rootBone = 0;
    chain.midBone  = 1;
    chain.endBone  = 2;
    REQUIRE(chain.isValid());
}

// ── TwoJointIK ──────────────────────────────────────────────────

TEST_CASE("TwoJointIK solve updates end bone to target", "[Animation][S5]") {
    TwoJointIK ik;
    BoneChain chain{0, 1, 2};

    std::vector<Transform> pose(3);
    pose[0].position = {0.f, 0.f, 0.f};
    pose[1].position = {1.f, 0.f, 0.f};
    pose[2].position = {2.f, 0.f, 0.f};

    Vec3 target{1.5f, 0.5f, 0.f};
    Vec3 pole{0.f, 1.f, 0.f};

    bool result = ik.solve(chain, target, pole, pose);
    REQUIRE(result);
    REQUIRE(ik.solveCount() == 1);

    // End bone should be moved to target
    REQUIRE_THAT(pose[2].position.x, Catch::Matchers::WithinAbs(target.x, 0.01));
    REQUIRE_THAT(pose[2].position.y, Catch::Matchers::WithinAbs(target.y, 0.01));
}

TEST_CASE("TwoJointIK invalid chain", "[Animation][S5]") {
    TwoJointIK ik;
    BoneChain chain; // invalid
    std::vector<Transform> pose(3);
    REQUIRE_FALSE(ik.solve(chain, {1, 0, 0}, {0, 1, 0}, pose));
}

TEST_CASE("TwoJointIK out-of-range bones", "[Animation][S5]") {
    TwoJointIK ik;
    BoneChain chain{0, 1, 5}; // bone 5 doesn't exist
    std::vector<Transform> pose(3);
    REQUIRE_FALSE(ik.solve(chain, {1, 0, 0}, {0, 1, 0}, pose));
}

// ── FPSHandRig ──────────────────────────────────────────────────

TEST_CASE("FPSHandRig default state", "[Animation][S5]") {
    FPSHandRig rig;
    REQUIRE(rig.skeleton() == nullptr);
    REQUIRE_FALSE(rig.armChain(HandSide::Left).isValid());
    REQUIRE_FALSE(rig.armChain(HandSide::Right).isValid());
}

TEST_CASE("FPSHandRig arm chain setup", "[Animation][S5]") {
    FPSHandRig rig;
    BoneChain left{0, 1, 2};
    BoneChain right{3, 4, 5};
    rig.setArmChain(HandSide::Left, left);
    rig.setArmChain(HandSide::Right, right);

    REQUIRE(rig.armChain(HandSide::Left).rootBone == 0);
    REQUIRE(rig.armChain(HandSide::Right).rootBone == 3);
}

TEST_CASE("FPSHandRig applyIK solves both arms", "[Animation][S5]") {
    FPSHandRig rig;
    rig.setArmChain(HandSide::Left,  {0, 1, 2});
    rig.setArmChain(HandSide::Right, {3, 4, 5});

    std::vector<Transform> pose(6);
    pose[0].position = {-1.f, 0.f, 0.f};
    pose[1].position = {-1.f, -1.f, 0.f};
    pose[2].position = {-1.f, -2.f, 0.f};
    pose[3].position = {1.f, 0.f, 0.f};
    pose[4].position = {1.f, -1.f, 0.f};
    pose[5].position = {1.f, -2.f, 0.f};

    rig.setHandTarget(HandSide::Left,  {-0.5f, -1.5f, 0.3f});
    rig.setHandTarget(HandSide::Right, {0.5f, -1.5f, 0.3f});

    int solved = rig.applyIK(pose);
    REQUIRE(solved == 2);
}

TEST_CASE("FPSHandRig sway offset", "[Animation][S5]") {
    FPSHandRig rig;
    rig.setHandTarget(HandSide::Left,  {0.f, 0.f, 0.f});
    rig.setHandTarget(HandSide::Right, {1.f, 0.f, 0.f});
    rig.setSwayOffset({0.1f, 0.05f, 0.f});
    rig.applySwayToTargets();

    REQUIRE_THAT(rig.handTarget(HandSide::Left).x, Catch::Matchers::WithinAbs(0.1f, 0.001));
    REQUIRE_THAT(rig.handTarget(HandSide::Right).x, Catch::Matchers::WithinAbs(1.1f, 0.001));
}

// ── AnimationBlendGraph ─────────────────────────────────────────

TEST_CASE("AnimationBlendGraph add and find nodes", "[Animation][S5]") {
    AnimationBlendGraph graph;
    AnimationClip idle("idle", 1.f);
    AnimationClip walk("walk", 0.8f);

    int idx0 = graph.addNode("idle", &idle, 0.5f);
    int idx1 = graph.addNode("walk", &walk, 0.5f);

    REQUIRE(idx0 == 0);
    REQUIRE(idx1 == 1);
    REQUIRE(graph.nodeCount() == 2);
    REQUIRE(graph.findNode("idle") == 0);
    REQUIRE(graph.findNode("walk") == 1);
    REQUIRE(graph.findNode("run") == -1);
}

TEST_CASE("AnimationBlendGraph setWeight clamps", "[Animation][S5]") {
    AnimationBlendGraph graph;
    AnimationClip clip("c", 1.f);
    graph.addNode("c", &clip, 0.5f);

    graph.setWeight(0, 1.5f);
    REQUIRE(graph.node(0)->weight == 1.f);

    graph.setWeight(0, -0.5f);
    REQUIRE(graph.node(0)->weight == 0.f);
}

TEST_CASE("AnimationBlendGraph update advances time", "[Animation][S5]") {
    AnimationBlendGraph graph;
    AnimationClip clip("c", 2.f);
    graph.addNode("c", &clip, 1.f, true);

    graph.update(1.5f);
    REQUIRE_THAT(graph.node(0)->localTime, Catch::Matchers::WithinAbs(1.5f, 0.001));

    // Looping: wraps around
    graph.update(1.0f);
    REQUIRE_THAT(graph.node(0)->localTime, Catch::Matchers::WithinAbs(0.5f, 0.001));
}

TEST_CASE("AnimationBlendGraph max nodes", "[Animation][S5]") {
    AnimationBlendGraph graph;
    for (int i = 0; i < AnimationBlendGraph::kMaxNodes; ++i)
        REQUIRE(graph.addNode("n" + std::to_string(i), nullptr) >= 0);
    REQUIRE(graph.addNode("overflow", nullptr) == -1);
}

TEST_CASE("AnimationBlendGraph clear", "[Animation][S5]") {
    AnimationBlendGraph graph;
    graph.addNode("a", nullptr);
    graph.addNode("b", nullptr);
    REQUIRE(graph.nodeCount() == 2);
    graph.clear();
    REQUIRE(graph.nodeCount() == 0);
}

// ── CharacterGroundingSystem ────────────────────────────────────

TEST_CASE("CharacterGroundingSystem default state", "[Animation][S5]") {
    CharacterGroundingSystem gs;
    REQUIRE(gs.groundHeight() == 0.f);
    REQUIRE_THAT(gs.maxAdjustment(), Catch::Matchers::WithinAbs(0.5f, 0.001));
    REQUIRE(gs.leftFootBone() == -1);
    REQUIRE(gs.rightFootBone() == -1);
}

TEST_CASE("CharacterGroundingSystem apply adjusts feet", "[Animation][S5]") {
    CharacterGroundingSystem gs;
    gs.setGroundHeight(1.0f);
    gs.setFootBones(0, 1);

    std::vector<Transform> pose(2);
    pose[0].position = {0.f, 0.f, 0.f};
    pose[1].position = {0.f, 0.f, 0.f};

    gs.apply(pose, 0.8f);  // character 0.2 below ground

    REQUIRE_THAT(pose[0].position.y, Catch::Matchers::WithinAbs(0.2f, 0.001));
    REQUIRE_THAT(pose[1].position.y, Catch::Matchers::WithinAbs(0.2f, 0.001));
}

TEST_CASE("CharacterGroundingSystem clamps to maxAdjustment", "[Animation][S5]") {
    CharacterGroundingSystem gs;
    gs.setGroundHeight(10.f);
    gs.setMaxAdjustment(0.3f);
    gs.setFootBones(0, 1);

    std::vector<Transform> pose(2);
    gs.apply(pose, 0.f);  // 10 units difference, clamped to 0.3

    REQUIRE_THAT(pose[0].position.y, Catch::Matchers::WithinAbs(0.3f, 0.001));
}

TEST_CASE("CharacterGroundingSystem needsGrounding", "[Animation][S5]") {
    CharacterGroundingSystem gs;
    gs.setGroundHeight(1.f);

    REQUIRE(gs.needsGrounding(0.5f));
    REQUIRE_FALSE(gs.needsGrounding(1.0f));
    REQUIRE_FALSE(gs.needsGrounding(1.005f, 0.01f));
}
