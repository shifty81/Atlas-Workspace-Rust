// Tests/Workspace/test_phase32_tour.cpp
// Phase 32 — Workspace Tour / Onboarding System
//
// Tests for:
//   1. TourStepKind  — enum name helpers
//   2. TourState     — enum name helpers
//   3. TourStep      — isValid; fields
//   4. TourSequence  — isValid; addStep; stepAt; stepCount
//   5. TourProgress  — isActive; fraction
//   6. TourController— load/start/next/prev/pause/resume/cancel/complete/reset
//   7. Integration   — full guided tour walk-through

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceTour.h"
#include <string>
#include <vector>

using namespace NF;

// Helper: build a valid TourStep
static TourStep makeStep(const std::string& id,
                         const std::string& title = "Step Title",
                         TourStepKind kind = TourStepKind::Tooltip) {
    TourStep s;
    s.id       = id;
    s.kind     = kind;
    s.title    = title;
    s.body     = "Step body text.";
    s.targetId = "panel.scene";
    return s;
}

// Helper: build a valid TourSequence with n steps
static TourSequence makeSequence(const std::string& id, int n) {
    TourSequence seq;
    seq.id   = id;
    seq.name = "Tour " + id;
    for (int i = 0; i < n; ++i)
        seq.addStep(makeStep("step_" + std::to_string(i), "Step " + std::to_string(i)));
    return seq;
}

// ─────────────────────────────────────────────────────────────────
// 1. TourStepKind enum
// ─────────────────────────────────────────────────────────────────
TEST_CASE("TourStepKind name helpers", "[TourStepKind]") {
    CHECK(std::string(tourStepKindName(TourStepKind::Highlight)) == "Highlight");
    CHECK(std::string(tourStepKindName(TourStepKind::Tooltip))   == "Tooltip");
    CHECK(std::string(tourStepKindName(TourStepKind::Modal))     == "Modal");
    CHECK(std::string(tourStepKindName(TourStepKind::Action))    == "Action");
    CHECK(std::string(tourStepKindName(TourStepKind::Pause))     == "Pause");
}

// ─────────────────────────────────────────────────────────────────
// 2. TourState enum
// ─────────────────────────────────────────────────────────────────
TEST_CASE("TourState name helpers", "[TourState]") {
    CHECK(std::string(tourStateName(TourState::Idle))      == "Idle");
    CHECK(std::string(tourStateName(TourState::Running))   == "Running");
    CHECK(std::string(tourStateName(TourState::Paused))    == "Paused");
    CHECK(std::string(tourStateName(TourState::Completed)) == "Completed");
    CHECK(std::string(tourStateName(TourState::Cancelled)) == "Cancelled");
}

// ─────────────────────────────────────────────────────────────────
// 3. TourStep
// ─────────────────────────────────────────────────────────────────
TEST_CASE("TourStep default is invalid", "[TourStep]") {
    TourStep s;
    CHECK_FALSE(s.isValid());
}

TEST_CASE("TourStep valid with id and title", "[TourStep]") {
    auto s = makeStep("s1");
    CHECK(s.isValid());
}

TEST_CASE("TourStep invalid without id", "[TourStep]") {
    TourStep s;
    s.title = "Title";
    CHECK_FALSE(s.isValid());
}

TEST_CASE("TourStep invalid without title", "[TourStep]") {
    TourStep s;
    s.id = "s1";
    CHECK_FALSE(s.isValid());
}

TEST_CASE("TourStep stores kind correctly", "[TourStep]") {
    auto s = makeStep("s1", "Title", TourStepKind::Modal);
    CHECK(s.kind == TourStepKind::Modal);
}

// ─────────────────────────────────────────────────────────────────
// 4. TourSequence
// ─────────────────────────────────────────────────────────────────
TEST_CASE("TourSequence default is invalid", "[TourSequence]") {
    TourSequence seq;
    CHECK_FALSE(seq.isValid());
}

TEST_CASE("TourSequence invalid with no steps", "[TourSequence]") {
    TourSequence seq;
    seq.id   = "onboard";
    seq.name = "Onboarding";
    CHECK_FALSE(seq.isValid()); // no steps
}

TEST_CASE("TourSequence valid after adding steps", "[TourSequence]") {
    auto seq = makeSequence("onboard", 3);
    CHECK(seq.isValid());
    CHECK(seq.stepCount() == 3);
}

TEST_CASE("TourSequence addStep rejects invalid step", "[TourSequence]") {
    TourSequence seq;
    seq.id = "s"; seq.name = "S";
    TourStep bad; // no id or title
    CHECK_FALSE(seq.addStep(bad));
    CHECK(seq.stepCount() == 0);
}

TEST_CASE("TourSequence stepAt returns correct step", "[TourSequence]") {
    auto seq = makeSequence("t", 3);
    const auto* s = seq.stepAt(1);
    REQUIRE(s != nullptr);
    CHECK(s->id == "step_1");
}

TEST_CASE("TourSequence stepAt out-of-range returns null", "[TourSequence]") {
    auto seq = makeSequence("t", 2);
    CHECK(seq.stepAt(-1) == nullptr);
    CHECK(seq.stepAt(2)  == nullptr);
}

// ─────────────────────────────────────────────────────────────────
// 5. TourProgress
// ─────────────────────────────────────────────────────────────────
TEST_CASE("TourProgress default is inactive", "[TourProgress]") {
    TourProgress p;
    CHECK_FALSE(p.isActive());
    CHECK(p.fraction() == 0.0f);
}

TEST_CASE("TourProgress isActive for valid indices", "[TourProgress]") {
    TourProgress p;
    p.totalSteps = 4;
    p.stepIndex  = 0;
    CHECK(p.isActive());
}

TEST_CASE("TourProgress fraction computation", "[TourProgress]") {
    TourProgress p;
    p.totalSteps = 4;
    p.stepIndex  = 1; // 2/4 = 0.5
    CHECK(p.fraction() == 0.5f);
}

TEST_CASE("TourProgress isActive false when stepIndex == totalSteps", "[TourProgress]") {
    TourProgress p;
    p.totalSteps = 3;
    p.stepIndex  = 3; // past-end
    CHECK_FALSE(p.isActive());
}

// ─────────────────────────────────────────────────────────────────
// 6. TourController
// ─────────────────────────────────────────────────────────────────
TEST_CASE("TourController default state is Idle", "[TourController]") {
    TourController tc;
    CHECK(tc.state() == TourState::Idle);
    CHECK_FALSE(tc.progress().isActive());
    CHECK(tc.currentStep() == nullptr);
}

TEST_CASE("TourController load valid sequence", "[TourController]") {
    TourController tc;
    CHECK(tc.load(makeSequence("onboard", 3)));
}

TEST_CASE("TourController load invalid sequence fails", "[TourController]") {
    TourController tc;
    TourSequence empty;
    CHECK_FALSE(tc.load(empty));
}

TEST_CASE("TourController start transitions to Running", "[TourController]") {
    TourController tc;
    tc.load(makeSequence("t", 3));
    CHECK(tc.start());
    CHECK(tc.state() == TourState::Running);
    CHECK(tc.progress().stepIndex == 0);
    CHECK(tc.progress().totalSteps == 3);
}

TEST_CASE("TourController start fails without loaded sequence", "[TourController]") {
    TourController tc;
    CHECK_FALSE(tc.start());
    CHECK(tc.state() == TourState::Idle);
}

TEST_CASE("TourController start fails if already running", "[TourController]") {
    TourController tc;
    tc.load(makeSequence("t", 3));
    tc.start();
    CHECK_FALSE(tc.start());
}

TEST_CASE("TourController next advances step", "[TourController]") {
    TourController tc;
    tc.load(makeSequence("t", 3));
    tc.start();
    CHECK(tc.next());
    CHECK(tc.progress().stepIndex == 1);
}

TEST_CASE("TourController next on last step completes tour", "[TourController]") {
    TourController tc;
    tc.load(makeSequence("t", 2));
    tc.start();
    tc.next(); // step 1
    tc.next(); // past last → Completed
    CHECK(tc.state() == TourState::Completed);
}

TEST_CASE("TourController next fails when not Running", "[TourController]") {
    TourController tc;
    tc.load(makeSequence("t", 3));
    CHECK_FALSE(tc.next()); // Idle
}

TEST_CASE("TourController prev goes back", "[TourController]") {
    TourController tc;
    tc.load(makeSequence("t", 3));
    tc.start();
    tc.next(); // step 1
    CHECK(tc.prev());
    CHECK(tc.progress().stepIndex == 0);
}

TEST_CASE("TourController prev fails at first step", "[TourController]") {
    TourController tc;
    tc.load(makeSequence("t", 3));
    tc.start();
    CHECK_FALSE(tc.prev());
}

TEST_CASE("TourController pause and resume", "[TourController]") {
    TourController tc;
    tc.load(makeSequence("t", 3));
    tc.start();
    CHECK(tc.pause());
    CHECK(tc.state() == TourState::Paused);
    CHECK(tc.resume());
    CHECK(tc.state() == TourState::Running);
}

TEST_CASE("TourController pause fails when not Running", "[TourController]") {
    TourController tc;
    CHECK_FALSE(tc.pause());
}

TEST_CASE("TourController resume fails when not Paused", "[TourController]") {
    TourController tc;
    tc.load(makeSequence("t", 3));
    tc.start();
    CHECK_FALSE(tc.resume()); // Running, not Paused
}

TEST_CASE("TourController cancel from Running", "[TourController]") {
    TourController tc;
    tc.load(makeSequence("t", 3));
    tc.start();
    CHECK(tc.cancel());
    CHECK(tc.state() == TourState::Cancelled);
}

TEST_CASE("TourController cancel from Paused", "[TourController]") {
    TourController tc;
    tc.load(makeSequence("t", 3));
    tc.start();
    tc.pause();
    CHECK(tc.cancel());
    CHECK(tc.state() == TourState::Cancelled);
}

TEST_CASE("TourController cancel fails when Idle", "[TourController]") {
    TourController tc;
    CHECK_FALSE(tc.cancel());
}

TEST_CASE("TourController currentStep returns active step", "[TourController]") {
    TourController tc;
    tc.load(makeSequence("t", 3));
    tc.start();
    const auto* s = tc.currentStep();
    REQUIRE(s != nullptr);
    CHECK(s->id == "step_0");
}

TEST_CASE("TourController currentStep null when Idle", "[TourController]") {
    TourController tc;
    tc.load(makeSequence("t", 3));
    CHECK(tc.currentStep() == nullptr);
}

TEST_CASE("TourController reset returns to Idle", "[TourController]") {
    TourController tc;
    tc.load(makeSequence("t", 3));
    tc.start();
    tc.cancel();
    tc.reset();
    CHECK(tc.state() == TourState::Idle);
    CHECK_FALSE(tc.progress().isActive());
}

TEST_CASE("TourController observer fires on state change", "[TourController]") {
    TourController tc;
    tc.load(makeSequence("t", 3));
    TourState last = TourState::Idle;
    tc.addObserver([&](TourState s, const TourProgress&){ last = s; });
    tc.start();
    CHECK(last == TourState::Running);
    tc.pause();
    CHECK(last == TourState::Paused);
}

TEST_CASE("TourController observer fires on next", "[TourController]") {
    TourController tc;
    tc.load(makeSequence("t", 3));
    int calls = 0;
    tc.addObserver([&](TourState, const TourProgress&){ ++calls; });
    tc.start(); // +1
    tc.next();  // +1
    CHECK(calls == 2);
}

TEST_CASE("TourController clearObservers removes observers", "[TourController]") {
    TourController tc;
    tc.load(makeSequence("t", 3));
    int calls = 0;
    tc.addObserver([&](TourState, const TourProgress&){ ++calls; });
    tc.clearObservers();
    tc.start();
    CHECK(calls == 0);
}

// ─────────────────────────────────────────────────────────────────
// 7. Integration
// ─────────────────────────────────────────────────────────────────
TEST_CASE("Tour integration: full walk-through", "[TourIntegration]") {
    TourController tc;
    auto seq = makeSequence("welcome", 4);
    tc.load(seq);
    tc.start();
    CHECK(tc.state() == TourState::Running);

    std::vector<int> stepsSeen;
    tc.addObserver([&](TourState, const TourProgress& p){
        if (p.isActive()) stepsSeen.push_back(p.stepIndex);
    });

    tc.next(); // step 1
    tc.next(); // step 2
    tc.next(); // step 3
    tc.next(); // complete
    CHECK(tc.state() == TourState::Completed);
}

TEST_CASE("Tour integration: pause mid-tour then resume", "[TourIntegration]") {
    TourController tc;
    tc.load(makeSequence("mid", 5));
    tc.start();
    tc.next();
    tc.next();
    tc.pause();
    CHECK(tc.state() == TourState::Paused);
    CHECK(tc.progress().stepIndex == 2);
    tc.resume();
    CHECK(tc.state() == TourState::Running);
    CHECK(tc.progress().stepIndex == 2);
}

TEST_CASE("Tour integration: progress fraction increases", "[TourIntegration]") {
    TourController tc;
    tc.load(makeSequence("frac", 4));
    tc.start();
    CHECK(tc.progress().fraction() == 0.25f);
    tc.next();
    CHECK(tc.progress().fraction() == 0.5f);
    tc.next();
    CHECK(tc.progress().fraction() == 0.75f);
}
