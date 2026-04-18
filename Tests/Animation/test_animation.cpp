#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "NF/Animation/Animation.h"

using Catch::Matchers::WithinAbs;

// ── Skeleton ─────────────────────────────────────────────────────

TEST_CASE("Skeleton add and find bones", "[Animation][Skeleton]") {
    NF::Skeleton skel("TestSkeleton");

    auto root = skel.addBone("Root", -1);
    auto spine = skel.addBone("Spine", root);
    auto head = skel.addBone("Head", spine);

    REQUIRE(skel.boneCount() == 3);
    REQUIRE(skel.name() == "TestSkeleton");

    REQUIRE(skel.findBone("Root") == 0);
    REQUIRE(skel.findBone("Spine") == 1);
    REQUIRE(skel.findBone("Head") == 2);
    REQUIRE(skel.findBone("Missing") == -1);

    REQUIRE(skel.bone(0).parentIndex == -1);
    REQUIRE(skel.bone(1).parentIndex == 0);
    REQUIRE(skel.bone(2).parentIndex == 1);
}

TEST_CASE("Skeleton empty", "[Animation][Skeleton]") {
    NF::Skeleton skel;
    REQUIRE(skel.boneCount() == 0);
}

// ── Animation Channel ────────────────────────────────────────────

TEST_CASE("AnimationChannel sample single key", "[Animation][Channel]") {
    NF::AnimationChannel channel;
    channel.boneIndex = 0;
    channel.keys.push_back({0.f, {{1, 2, 3}}});

    auto result = channel.sample(0.5f);
    REQUIRE_THAT(result.position.x, WithinAbs(1.f, 1e-5));
    REQUIRE_THAT(result.position.y, WithinAbs(2.f, 1e-5));
    REQUIRE_THAT(result.position.z, WithinAbs(3.f, 1e-5));
}

TEST_CASE("AnimationChannel sample interpolation", "[Animation][Channel]") {
    NF::AnimationChannel channel;
    channel.boneIndex = 0;
    NF::Transform t0; t0.position = {0, 0, 0}; t0.scale = {1, 1, 1};
    NF::Transform t1; t1.position = {10, 0, 0}; t1.scale = {1, 1, 1};
    channel.keys.push_back({0.f, t0});
    channel.keys.push_back({1.f, t1});

    auto mid = channel.sample(0.5f);
    REQUIRE_THAT(mid.position.x, WithinAbs(5.f, 1e-4));
}

TEST_CASE("AnimationChannel sample before first key", "[Animation][Channel]") {
    NF::AnimationChannel channel;
    channel.boneIndex = 0;
    NF::Transform t0; t0.position = {5, 0, 0};
    channel.keys.push_back({1.f, t0});

    auto result = channel.sample(0.f);
    REQUIRE_THAT(result.position.x, WithinAbs(5.f, 1e-5));
}

TEST_CASE("AnimationChannel sample after last key", "[Animation][Channel]") {
    NF::AnimationChannel channel;
    channel.boneIndex = 0;
    NF::Transform t0; t0.position = {5, 0, 0};
    NF::Transform t1; t1.position = {10, 0, 0};
    channel.keys.push_back({0.f, t0});
    channel.keys.push_back({1.f, t1});

    auto result = channel.sample(2.f);
    REQUIRE_THAT(result.position.x, WithinAbs(10.f, 1e-5));
}

// ── Animation Clip ───────────────────────────────────────────────

TEST_CASE("AnimationClip sample fills pose", "[Animation][Clip]") {
    NF::AnimationClip clip("Walk", 2.f);
    REQUIRE(clip.name() == "Walk");
    REQUIRE_THAT(clip.duration(), WithinAbs(2.f, 1e-5));

    auto& ch = clip.addChannel(0);
    NF::Transform t0; t0.position = {0, 0, 0};
    NF::Transform t1; t1.position = {10, 0, 0};
    ch.keys.push_back({0.f, t0});
    ch.keys.push_back({2.f, t1});

    REQUIRE(clip.channelCount() == 1);

    std::vector<NF::Transform> pose(1);
    clip.sample(1.f, pose);
    REQUIRE_THAT(pose[0].position.x, WithinAbs(5.f, 1e-4));
}

// ── Animation State Machine ──────────────────────────────────────

TEST_CASE("AnimationStateMachine default state", "[Animation][StateMachine]") {
    NF::AnimationStateMachine sm;
    NF::AnimationClip idleClip("Idle", 1.f);
    NF::AnimationClip walkClip("Walk", 0.5f);

    sm.addState("Idle", &idleClip, true);
    sm.addState("Walk", &walkClip, true);

    REQUIRE(sm.currentState() == "Idle");
    REQUIRE(sm.stateCount() == 2);
    REQUIRE(sm.currentClip() == &idleClip);
}

TEST_CASE("AnimationStateMachine transitions", "[Animation][StateMachine]") {
    NF::AnimationStateMachine sm;
    NF::AnimationClip idleClip("Idle", 1.f);
    NF::AnimationClip walkClip("Walk", 0.5f);

    sm.addState("Idle", &idleClip, true);
    sm.addState("Walk", &walkClip, true);

    bool shouldWalk = false;
    sm.addTransition("Idle", "Walk", [&]() { return shouldWalk; });

    sm.update(0.1f);
    REQUIRE(sm.currentState() == "Idle"); // condition not met

    shouldWalk = true;
    sm.update(0.1f);
    REQUIRE(sm.currentState() == "Walk"); // transitioned
    REQUIRE_THAT(sm.stateTime(), WithinAbs(0.f, 1e-5)); // reset on transition
}

TEST_CASE("AnimationStateMachine time advances", "[Animation][StateMachine]") {
    NF::AnimationStateMachine sm;
    NF::AnimationClip clip("Test", 2.f);

    sm.addState("Test", &clip, false, 1.f);

    sm.update(0.5f);
    REQUIRE_THAT(sm.stateTime(), WithinAbs(0.5f, 1e-4));

    sm.update(0.3f);
    REQUIRE_THAT(sm.stateTime(), WithinAbs(0.8f, 1e-4));
}

TEST_CASE("AnimationStateMachine speed multiplier", "[Animation][StateMachine]") {
    NF::AnimationStateMachine sm;
    NF::AnimationClip clip("Fast", 2.f);

    sm.addState("Fast", &clip, false, 2.f); // 2x speed

    sm.update(0.5f);
    // At 2x speed, 0.5s real = 1.0s animation
    REQUIRE_THAT(sm.stateTime(), WithinAbs(1.f, 1e-4));
}

TEST_CASE("AnimationStateMachine sample pose", "[Animation][StateMachine]") {
    NF::AnimationClip clip("Move", 1.f);
    auto& ch = clip.addChannel(0);
    NF::Transform t0; t0.position = {0, 0, 0};
    NF::Transform t1; t1.position = {10, 0, 0};
    ch.keys.push_back({0.f, t0});
    ch.keys.push_back({1.f, t1});

    NF::AnimationStateMachine sm;
    sm.addState("Move", &clip, false);

    sm.update(0.5f);

    std::vector<NF::Transform> pose(1);
    sm.sampleCurrentPose(pose);
    REQUIRE_THAT(pose[0].position.x, WithinAbs(5.f, 1e-4));
}
