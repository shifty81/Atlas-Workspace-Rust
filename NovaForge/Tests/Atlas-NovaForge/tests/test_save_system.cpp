#include "../engine/sim/SaveSystem.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <filesystem>

using namespace atlas::sim;

void test_save_system_save_and_load() {
    SaveSystem saver;

    std::vector<uint8_t> ecsData = {10, 20, 30, 40, 50};
    std::vector<uint8_t> auxData = {100, 200};
    std::string metadata = "test save";

    std::string path = "/tmp/atlas_test_save.asav";

    SaveResult res = saver.Save(path, 100, 30, 42, ecsData, auxData, metadata);
    assert(res == SaveResult::Success);

    SaveSystem loader;
    res = loader.Load(path);
    assert(res == SaveResult::Success);

    assert(loader.Header().saveTick == 100);
    assert(loader.Header().tickRate == 30);
    assert(loader.Header().seed == 42);
    assert(loader.ECSData() == ecsData);
    assert(loader.AuxData() == auxData);
    assert(loader.Metadata() == metadata);

    std::filesystem::remove(path);

    std::cout << "[PASS] test_save_system_save_and_load" << std::endl;
}

void test_save_system_hash_integrity() {
    SaveSystem saver;
    std::vector<uint8_t> data = {1, 2, 3};
    std::string path = "/tmp/atlas_test_hash.asav";

    saver.Save(path, 1, 30, 0, data);

    // Corrupt the file by modifying a data byte
    {
        std::fstream f(path, std::ios::binary | std::ios::in | std::ios::out);
        // Seek past header to first data byte and corrupt it
        f.seekp(sizeof(SaveHeader));
        char corrupted = 99;
        f.write(&corrupted, 1);
    }

    SaveSystem loader;
    SaveResult res = loader.Load(path);
    assert(res == SaveResult::HashMismatch);

    std::filesystem::remove(path);

    std::cout << "[PASS] test_save_system_hash_integrity" << std::endl;
}

void test_save_system_file_not_found() {
    SaveSystem loader;
    SaveResult res = loader.Load("/tmp/nonexistent_save_file.asav");
    assert(res == SaveResult::FileNotFound);

    std::cout << "[PASS] test_save_system_file_not_found" << std::endl;
}

void test_save_system_validate() {
    SaveSystem saver;
    std::vector<uint8_t> data = {1, 2, 3};
    std::string path = "/tmp/atlas_test_validate.asav";

    saver.Save(path, 50, 60, 7, data);

    SaveResult res = saver.Validate(path);
    assert(res == SaveResult::Success);

    std::filesystem::remove(path);

    std::cout << "[PASS] test_save_system_validate" << std::endl;
}

void test_save_system_invalid_format() {
    std::string path = "/tmp/atlas_test_invalid.asav";

    // Write garbage data
    {
        std::ofstream f(path, std::ios::binary);
        uint32_t garbage = 0xDEADBEEF;
        f.write(reinterpret_cast<const char*>(&garbage), sizeof(garbage));
    }

    SaveSystem loader;
    SaveResult res = loader.Load(path);
    assert(res == SaveResult::InvalidFormat);

    std::filesystem::remove(path);

    std::cout << "[PASS] test_save_system_invalid_format" << std::endl;
}

void test_save_system_empty_data() {
    SaveSystem saver;
    std::vector<uint8_t> empty;
    std::string path = "/tmp/atlas_test_empty.asav";

    SaveResult res = saver.Save(path, 0, 30, 0, empty);
    assert(res == SaveResult::Success);

    SaveSystem loader;
    res = loader.Load(path);
    assert(res == SaveResult::Success);
    assert(loader.ECSData().empty());
    assert(loader.AuxData().empty());
    assert(loader.Metadata().empty());

    std::filesystem::remove(path);

    std::cout << "[PASS] test_save_system_empty_data" << std::endl;
}

void test_save_system_clear() {
    SaveSystem saver;
    std::vector<uint8_t> data = {1, 2, 3};
    std::string path = "/tmp/atlas_test_clear.asav";

    saver.Save(path, 1, 30, 0, data);

    SaveSystem loader;
    loader.Load(path);
    assert(!loader.ECSData().empty());

    loader.Clear();
    assert(loader.ECSData().empty());
    assert(loader.AuxData().empty());
    assert(loader.Metadata().empty());

    std::filesystem::remove(path);

    std::cout << "[PASS] test_save_system_clear" << std::endl;
}
