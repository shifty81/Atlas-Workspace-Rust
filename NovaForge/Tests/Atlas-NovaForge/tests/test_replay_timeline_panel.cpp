#include "../editor/panels/ReplayTimelinePanel.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace atlas::editor;
using atlas::sim::ReplayFrame;

void test_replay_timeline_empty() {
    ReplayTimelinePanel panel;

    assert(std::string(panel.Name()) == "Replay Timeline");
    assert(panel.GetCurrentTick() == 0);
    assert(panel.TotalTicks() == 0);
    assert(panel.TickRate() == 0);
    assert(panel.CurrentFrame() == nullptr);
    assert(panel.Markers().empty());
    assert(!panel.HasDivergence());
    assert(panel.DivergenceTick() == -1);
    assert(!panel.HasInjectedInputs());

    std::cout << "[PASS] test_replay_timeline_empty" << std::endl;
}

void test_replay_timeline_load() {
    ReplayTimelinePanel panel;

    std::vector<ReplayFrame> frames(10);
    for (uint32_t i = 0; i < 10; ++i) {
        frames[i].tick = i;
        frames[i].stateHash = i * 100;
    }

    panel.LoadReplay(frames, 60);
    assert(panel.TotalTicks() == 10);
    assert(panel.TickRate() == 60);
    assert(panel.GetCurrentTick() == 0);
    assert(panel.CurrentFrame() != nullptr);
    assert(panel.CurrentFrame()->tick == 0);

    std::cout << "[PASS] test_replay_timeline_load" << std::endl;
}

void test_replay_timeline_scrub() {
    ReplayTimelinePanel panel;

    std::vector<ReplayFrame> frames(20);
    for (uint32_t i = 0; i < 20; ++i) {
        frames[i].tick = i;
    }

    panel.LoadReplay(frames, 30);

    panel.SetCurrentTick(10);
    assert(panel.GetCurrentTick() == 10);
    assert(panel.CurrentFrame() != nullptr);
    assert(panel.CurrentFrame()->tick == 10);

    // Clamp to last tick
    panel.SetCurrentTick(999);
    assert(panel.GetCurrentTick() == 19);

    // Scrub to zero
    panel.SetCurrentTick(0);
    assert(panel.GetCurrentTick() == 0);

    std::cout << "[PASS] test_replay_timeline_scrub" << std::endl;
}

void test_replay_timeline_markers() {
    ReplayTimelinePanel panel;

    std::vector<ReplayFrame> frames(100);
    for (uint32_t i = 0; i < 100; ++i) frames[i].tick = i;
    panel.LoadReplay(frames, 60);

    TimelineMarker m1;
    m1.tick = 10;
    m1.label = "Start";
    m1.type = MarkerType::Bookmark;

    TimelineMarker m2;
    m2.tick = 50;
    m2.label = "Mid";
    m2.type = MarkerType::Branch;

    TimelineMarker m3;
    m3.tick = 90;
    m3.label = "End";
    m3.type = MarkerType::Injection;

    panel.AddMarker(m1);
    panel.AddMarker(m2);
    panel.AddMarker(m3);
    assert(panel.Markers().size() == 3);

    // Range query
    auto range = panel.MarkersInRange(10, 50);
    assert(range.size() == 2);

    // Remove
    panel.RemoveMarker(50);
    assert(panel.Markers().size() == 2);

    std::cout << "[PASS] test_replay_timeline_markers" << std::endl;
}

void test_replay_timeline_compare_identical() {
    ReplayTimelinePanel panel;

    std::vector<ReplayFrame> frames(5);
    for (uint32_t i = 0; i < 5; ++i) {
        frames[i].tick = i;
        frames[i].stateHash = i * 10;
        frames[i].inputData = {static_cast<uint8_t>(i)};
    }

    panel.LoadReplay(frames, 30);
    auto result = panel.CompareWith(frames);

    assert(result.divergeTick == -1);
    assert(std::abs(result.matchPercentage - 100.0) < 0.01);
    assert(!panel.HasDivergence());
    assert(panel.DivergenceTick() == -1);

    std::cout << "[PASS] test_replay_timeline_compare_identical" << std::endl;
}

void test_replay_timeline_compare_divergent() {
    ReplayTimelinePanel panel;

    std::vector<ReplayFrame> framesA(5);
    std::vector<ReplayFrame> framesB(5);
    for (uint32_t i = 0; i < 5; ++i) {
        framesA[i].tick = i;
        framesA[i].stateHash = i * 10;
        framesA[i].inputData = {static_cast<uint8_t>(i)};
        framesB[i].tick = i;
        framesB[i].stateHash = i * 10;
        framesB[i].inputData = {static_cast<uint8_t>(i)};
    }

    // Diverge at tick 3
    framesB[3].stateHash = 999;

    panel.LoadReplay(framesA, 30);
    auto result = panel.CompareWith(framesB);

    assert(result.divergeTick == 3);
    assert(result.matchPercentage < 100.0);
    assert(panel.HasDivergence());
    assert(panel.DivergenceTick() == 3);

    std::cout << "[PASS] test_replay_timeline_compare_divergent" << std::endl;
}

void test_replay_timeline_inject_input() {
    ReplayTimelinePanel panel;

    std::vector<ReplayFrame> frames(5);
    for (uint32_t i = 0; i < 5; ++i) {
        frames[i].tick = i;
    }

    panel.LoadReplay(frames, 30);
    assert(!panel.HasInjectedInputs());

    std::vector<uint8_t> input = {0xAA, 0xBB};
    panel.InjectInput(2, input);
    assert(panel.HasInjectedInputs());

    panel.SetCurrentTick(2);
    const auto* frame = panel.CurrentFrame();
    assert(frame != nullptr);
    assert(frame->inputData.size() == 2);
    assert(frame->inputData[0] == 0xAA);
    assert(frame->inputData[1] == 0xBB);

    std::cout << "[PASS] test_replay_timeline_inject_input" << std::endl;
}

void test_replay_timeline_branch() {
    ReplayTimelinePanel panel;

    std::vector<ReplayFrame> frames(10);
    for (uint32_t i = 0; i < 10; ++i) {
        frames[i].tick = i;
        frames[i].stateHash = i * 100;
    }

    panel.LoadReplay(frames, 60);
    auto branch = panel.BranchAt(4);

    assert(branch.size() == 5); // ticks 0..4
    assert(branch.front().tick == 0);
    assert(branch.back().tick == 4);

    std::cout << "[PASS] test_replay_timeline_branch" << std::endl;
}
