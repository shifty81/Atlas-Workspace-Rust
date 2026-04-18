#include "../engine/sim/ReplayRecorder.h"
#include <iostream>
#include <cassert>
#include <filesystem>

using namespace atlas::sim;

void test_replay_initial_state() {
    ReplayRecorder recorder;

    assert(recorder.State() == ReplayState::Idle);
    assert(recorder.FrameCount() == 0);
    assert(recorder.DurationTicks() == 0);
    assert(recorder.Frames().empty());

    std::cout << "[PASS] test_replay_initial_state" << std::endl;
}

void test_replay_start_recording() {
    ReplayRecorder recorder;

    recorder.StartRecording(60, 12345);
    assert(recorder.State() == ReplayState::Recording);
    assert(recorder.Header().tickRate == 60);
    assert(recorder.Header().seed == 12345);
    assert(recorder.FrameCount() == 0);

    std::cout << "[PASS] test_replay_start_recording" << std::endl;
}

void test_replay_record_frames() {
    ReplayRecorder recorder;
    recorder.StartRecording(30);

    recorder.RecordFrame(0, {1, 2, 3});
    recorder.RecordFrame(1, {4, 5});
    recorder.RecordFrame(2, {6});

    assert(recorder.FrameCount() == 3);
    assert(recorder.Header().frameCount == 3);
    assert(recorder.DurationTicks() == 2);

    const ReplayFrame* f = recorder.FrameAtTick(1);
    assert(f != nullptr);
    assert(f->inputData.size() == 2);
    assert(f->inputData[0] == 4);

    assert(recorder.FrameAtTick(99) == nullptr);

    std::cout << "[PASS] test_replay_record_frames" << std::endl;
}

void test_replay_stop_recording() {
    ReplayRecorder recorder;
    recorder.StartRecording(30);
    recorder.RecordFrame(0, {1});

    recorder.StopRecording();
    assert(recorder.State() == ReplayState::Idle);

    // Should not record after stopping
    recorder.RecordFrame(1, {2});
    assert(recorder.FrameCount() == 1);

    std::cout << "[PASS] test_replay_stop_recording" << std::endl;
}

void test_replay_save_and_load() {
    std::string tmpPath = (std::filesystem::temp_directory_path() / "atlas_replay_test.rply").string();

    // Record
    {
        ReplayRecorder recorder;
        recorder.StartRecording(60, 42);
        recorder.RecordFrame(0, {10, 20, 30});
        recorder.RecordFrame(1, {40, 50});
        recorder.RecordFrame(2, {});
        recorder.StopRecording();

        bool saved = recorder.SaveReplay(tmpPath);
        assert(saved);
    }

    // Load
    {
        ReplayRecorder loader;
        bool loaded = loader.LoadReplay(tmpPath);
        assert(loaded);

        assert(loader.State() == ReplayState::Playing);
        assert(loader.Header().tickRate == 60);
        assert(loader.Header().seed == 42);
        assert(loader.FrameCount() == 3);

        const ReplayFrame* f0 = loader.FrameAtTick(0);
        assert(f0 != nullptr);
        assert(f0->inputData.size() == 3);
        assert(f0->inputData[2] == 30);

        const ReplayFrame* f1 = loader.FrameAtTick(1);
        assert(f1 != nullptr);
        assert(f1->inputData.size() == 2);

        const ReplayFrame* f2 = loader.FrameAtTick(2);
        assert(f2 != nullptr);
        assert(f2->inputData.empty());
    }

    std::filesystem::remove(tmpPath);

    std::cout << "[PASS] test_replay_save_and_load" << std::endl;
}

void test_replay_load_invalid() {
    ReplayRecorder recorder;
    bool loaded = recorder.LoadReplay("/nonexistent/path.rply");
    assert(!loaded);
    assert(recorder.State() == ReplayState::Idle);

    std::cout << "[PASS] test_replay_load_invalid" << std::endl;
}

void test_replay_clear() {
    ReplayRecorder recorder;
    recorder.StartRecording(30);
    recorder.RecordFrame(0, {1});
    recorder.RecordFrame(1, {2});

    recorder.Clear();
    assert(recorder.State() == ReplayState::Idle);
    assert(recorder.FrameCount() == 0);

    std::cout << "[PASS] test_replay_clear" << std::endl;
}
