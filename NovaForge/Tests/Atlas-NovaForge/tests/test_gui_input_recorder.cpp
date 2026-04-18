#include "../engine/ui/GUIInputRecorder.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdio>
#include <cstring>

using namespace atlas::ui;

void test_recorder_initial_state() {
    GUIInputRecorder recorder;
    assert(recorder.State() == GUIRecordState::Idle);
    assert(recorder.EventCount() == 0);
    assert(recorder.PlaybackPosition() == 0);

    std::cout << "[PASS] test_recorder_initial_state" << std::endl;
}

void test_recorder_start_stop_recording() {
    GUIInputRecorder recorder;
    recorder.StartRecording(10);
    assert(recorder.State() == GUIRecordState::Recording);

    recorder.StopRecording();
    assert(recorder.State() == GUIRecordState::Idle);
    assert(recorder.Log().startTick == 10);
    assert(recorder.Log().endTick == 10);

    std::cout << "[PASS] test_recorder_start_stop_recording" << std::endl;
}

void test_recorder_record_events() {
    GUIInputRecorder recorder;
    recorder.StartRecording(1);

    UICommand cmd1;
    cmd1.type = UICommandType::ButtonPress;
    cmd1.targetWidgetId = 42;
    cmd1.tick = 5;
    recorder.RecordEvent(cmd1);

    UICommand cmd2;
    cmd2.type = UICommandType::TextInput;
    cmd2.targetWidgetId = 99;
    cmd2.tick = 10;
    cmd2.valueString = "hello";
    recorder.RecordEvent(cmd2);

    assert(recorder.EventCount() == 2);

    recorder.StopRecording();
    assert(recorder.Log().endTick == 10);
    assert(recorder.Log().events[0].type == UICommandType::ButtonPress);
    assert(recorder.Log().events[0].targetWidgetId == 42);
    assert(recorder.Log().events[1].valueString == "hello");

    std::cout << "[PASS] test_recorder_record_events" << std::endl;
}

void test_recorder_record_only_when_recording() {
    GUIInputRecorder recorder;

    UICommand cmd;
    cmd.type = UICommandType::ButtonPress;
    cmd.tick = 1;
    recorder.RecordEvent(cmd);

    assert(recorder.EventCount() == 0);

    std::cout << "[PASS] test_recorder_record_only_when_recording" << std::endl;
}

void test_recorder_playback() {
    GUIInputRecorder recorder;
    recorder.StartRecording(0);

    UICommand cmd1;
    cmd1.type = UICommandType::ButtonPress;
    cmd1.targetWidgetId = 1;
    cmd1.tick = 2;
    recorder.RecordEvent(cmd1);

    UICommand cmd2;
    cmd2.type = UICommandType::ButtonRelease;
    cmd2.targetWidgetId = 1;
    cmd2.tick = 4;
    recorder.RecordEvent(cmd2);

    recorder.StopRecording();

    UICommandBus bus;
    recorder.StartPlayback(&bus, 100);
    assert(recorder.State() == GUIRecordState::Playing);

    // At tick 100, events with offset 0 should fire (none yet, offset=2 and offset=4)
    recorder.AdvancePlayback(100);
    assert(bus.PendingCount() == 0);

    // At tick 102 (offset=2 from start), first event fires
    recorder.AdvancePlayback(102);
    assert(bus.PendingCount() == 1);

    // At tick 104 (offset=4), second event fires
    recorder.AdvancePlayback(104);
    assert(bus.PendingCount() == 2);

    recorder.StopPlayback();
    assert(recorder.State() == GUIRecordState::Idle);

    std::cout << "[PASS] test_recorder_playback" << std::endl;
}

void test_recorder_save_load() {
    const std::string path = "/tmp/atlas_gui_recorder_test.agui";
    std::remove(path.c_str());

    {
        GUIInputRecorder recorder;
        recorder.StartRecording(5);

        UICommand cmd;
        cmd.type = UICommandType::SliderChange;
        cmd.targetWidgetId = 77;
        cmd.tick = 10;
        cmd.valueFloat = 0.75f;
        cmd.valueString = "volume";
        recorder.RecordEvent(cmd);

        recorder.StopRecording();
        assert(recorder.SaveLog(path));
    }

    {
        GUIInputRecorder loader;
        assert(loader.LoadLog(path));
        assert(loader.State() == GUIRecordState::Idle);
        assert(loader.EventCount() == 1);
        assert(loader.Log().startTick == 5);
        assert(loader.Log().endTick == 10);

        auto& evt = loader.Log().events[0];
        assert(evt.type == UICommandType::SliderChange);
        assert(evt.targetWidgetId == 77);
        assert(evt.tick == 10);
        assert(evt.valueFloat > 0.74f && evt.valueFloat < 0.76f);
        assert(evt.valueString == "volume");
    }

    std::remove(path.c_str());
    std::cout << "[PASS] test_recorder_save_load" << std::endl;
}

void test_recorder_load_invalid_file() {
    const std::string path = "/tmp/atlas_gui_invalid_test.agui";
    std::remove(path.c_str());

    // Write garbage
    {
        std::ofstream f(path, std::ios::binary);
        uint32_t garbage = 0xDEADBEEF;
        f.write(reinterpret_cast<const char*>(&garbage), sizeof(garbage));
    }

    GUIInputRecorder loader;
    assert(!loader.LoadLog(path));

    std::remove(path.c_str());
    std::cout << "[PASS] test_recorder_load_invalid_file" << std::endl;
}

void test_recorder_load_nonexistent_file() {
    GUIInputRecorder loader;
    assert(!loader.LoadLog("/tmp/atlas_gui_does_not_exist.agui"));

    std::cout << "[PASS] test_recorder_load_nonexistent_file" << std::endl;
}
