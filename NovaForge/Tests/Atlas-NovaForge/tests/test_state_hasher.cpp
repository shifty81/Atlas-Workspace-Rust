#include "../engine/sim/StateHasher.h"
#include "../engine/sim/ReplayRecorder.h"
#include <iostream>
#include <cassert>
#include <filesystem>

using namespace atlas::sim;

void test_hasher_initial_state() {
    StateHasher hasher;
    hasher.Reset(0);

    assert(hasher.CurrentTick() == 0);
    assert(hasher.History().empty());

    std::cout << "[PASS] test_hasher_initial_state" << std::endl;
}

void test_hasher_advance_tick() {
    StateHasher hasher;
    hasher.Reset(42);

    std::vector<uint8_t> state = {1, 2, 3};
    std::vector<uint8_t> inputs = {4, 5};
    hasher.AdvanceTick(1, state, inputs);

    assert(hasher.CurrentTick() == 1);
    assert(hasher.CurrentHash() != 0);
    assert(hasher.History().size() == 1);
    assert(hasher.History()[0].tick == 1);
    assert(hasher.History()[0].hash == hasher.CurrentHash());

    std::cout << "[PASS] test_hasher_advance_tick" << std::endl;
}

void test_hasher_deterministic() {
    // Two hashers with identical seeds and data must produce identical hashes
    StateHasher a, b;
    a.Reset(100);
    b.Reset(100);

    std::vector<uint8_t> state = {10, 20, 30};
    std::vector<uint8_t> inputs = {40, 50};

    for (uint64_t t = 1; t <= 10; ++t) {
        a.AdvanceTick(t, state, inputs);
        b.AdvanceTick(t, state, inputs);
    }

    assert(a.CurrentHash() == b.CurrentHash());
    assert(a.History().size() == b.History().size());
    for (size_t i = 0; i < a.History().size(); ++i) {
        assert(a.History()[i].hash == b.History()[i].hash);
    }

    std::cout << "[PASS] test_hasher_deterministic" << std::endl;
}

void test_hasher_chaining() {
    // Hash at tick N depends on hash at tick N-1
    StateHasher hasher;
    hasher.Reset(0);

    std::vector<uint8_t> state = {1};
    std::vector<uint8_t> inputs = {2};

    hasher.AdvanceTick(1, state, inputs);
    uint64_t h1 = hasher.CurrentHash();

    hasher.AdvanceTick(2, state, inputs);
    uint64_t h2 = hasher.CurrentHash();

    // Same data but different tick → different hash (chaining effect)
    assert(h1 != h2);

    std::cout << "[PASS] test_hasher_chaining" << std::endl;
}

void test_hasher_divergence_detection() {
    StateHasher a, b;
    a.Reset(0);
    b.Reset(0);

    std::vector<uint8_t> state = {1, 2, 3};
    std::vector<uint8_t> inputs = {4};

    // Identical for first 5 ticks
    for (uint64_t t = 1; t <= 5; ++t) {
        a.AdvanceTick(t, state, inputs);
        b.AdvanceTick(t, state, inputs);
    }
    assert(a.FindDivergence(b) == -1);

    // Diverge at tick 6
    a.AdvanceTick(6, state, inputs);
    std::vector<uint8_t> altState = {9, 9, 9};
    b.AdvanceTick(6, altState, inputs);

    assert(a.FindDivergence(b) == 6);

    std::cout << "[PASS] test_hasher_divergence_detection" << std::endl;
}

void test_hasher_different_seeds() {
    StateHasher a, b;
    a.Reset(1);
    b.Reset(2);

    std::vector<uint8_t> state = {1};
    std::vector<uint8_t> inputs = {2};

    a.AdvanceTick(1, state, inputs);
    b.AdvanceTick(1, state, inputs);

    // Different seeds → different hashes
    assert(a.CurrentHash() != b.CurrentHash());

    std::cout << "[PASS] test_hasher_different_seeds" << std::endl;
}

void test_hasher_empty_data() {
    StateHasher hasher;
    hasher.Reset(0);

    std::vector<uint8_t> empty;
    hasher.AdvanceTick(1, empty, empty);

    assert(hasher.CurrentTick() == 1);
    assert(hasher.History().size() == 1);

    std::cout << "[PASS] test_hasher_empty_data" << std::endl;
}

void test_hasher_raw_pointer_api() {
    StateHasher a, b;
    a.Reset(0);
    b.Reset(0);

    uint8_t state[] = {1, 2, 3};
    uint8_t inputs[] = {4, 5};

    a.AdvanceTick(1, state, 3, inputs, 2);
    b.AdvanceTick(1, std::vector<uint8_t>{1, 2, 3}, std::vector<uint8_t>{4, 5});

    assert(a.CurrentHash() == b.CurrentHash());

    std::cout << "[PASS] test_hasher_raw_pointer_api" << std::endl;
}

void test_replay_record_with_hash() {
    ReplayRecorder recorder;
    recorder.StartRecording(60, 42);

    recorder.RecordFrame(0, {1, 2}, 0xDEAD);
    recorder.RecordFrame(1, {3, 4}, 0xBEEF);
    recorder.StopRecording();

    assert(recorder.FrameCount() == 2);
    assert(recorder.FrameAtTick(0)->stateHash == 0xDEAD);
    assert(recorder.FrameAtTick(1)->stateHash == 0xBEEF);

    std::cout << "[PASS] test_replay_record_with_hash" << std::endl;
}

void test_replay_save_load_with_hash() {
    std::string tmpPath = (std::filesystem::temp_directory_path() / "atlas_hash_test_v2.rply").string();

    // Record with hashes
    {
        ReplayRecorder recorder;
        recorder.StartRecording(60, 99);
        recorder.RecordFrame(0, {10, 20}, 0x1111);
        recorder.RecordFrame(1, {30}, 0x2222);
        recorder.RecordFrame(2, {}, 0x3333);
        recorder.StopRecording();

        assert(recorder.SaveReplay(tmpPath));
    }

    // Load and verify hashes are preserved
    {
        ReplayRecorder loader;
        assert(loader.LoadReplay(tmpPath));
        assert(loader.Header().version == 3);
        assert(loader.FrameCount() == 3);
        assert(loader.FrameAtTick(0)->stateHash == 0x1111);
        assert(loader.FrameAtTick(1)->stateHash == 0x2222);
        assert(loader.FrameAtTick(2)->stateHash == 0x3333);
    }

    std::filesystem::remove(tmpPath);

    std::cout << "[PASS] test_replay_save_load_with_hash" << std::endl;
}

void test_replay_default_hash_zero() {
    ReplayRecorder recorder;
    recorder.StartRecording(30);
    recorder.RecordFrame(0, {1, 2, 3});
    recorder.StopRecording();

    // Default overload should set stateHash to 0
    assert(recorder.FrameAtTick(0)->stateHash == 0);

    std::cout << "[PASS] test_replay_default_hash_zero" << std::endl;
}

void test_hash_combine_deterministic() {
    uint8_t data[] = {1, 2, 3, 4, 5};

    uint64_t h1 = StateHasher::HashCombine(0, data, 5);
    uint64_t h2 = StateHasher::HashCombine(0, data, 5);
    assert(h1 == h2);
    assert(h1 != 0);

    // Different data → different hash
    uint8_t data2[] = {5, 4, 3, 2, 1};
    uint64_t h3 = StateHasher::HashCombine(0, data2, 5);
    assert(h1 != h3);

    std::cout << "[PASS] test_hash_combine_deterministic" << std::endl;
}
