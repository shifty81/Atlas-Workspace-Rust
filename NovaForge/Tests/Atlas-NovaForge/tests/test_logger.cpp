#include "../engine/core/Logger.h"
#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>

using namespace atlas;

void test_logger_creates_log_directory() {
    std::filesystem::remove_all("logs");

    Logger::Init();
    Logger::Shutdown();

    assert(std::filesystem::exists("logs"));
    assert(std::filesystem::is_directory("logs"));

    std::filesystem::remove_all("logs");

    std::cout << "[PASS] test_logger_creates_log_directory" << std::endl;
}

void test_logger_writes_to_file() {
    std::filesystem::remove_all("logs");

    Logger::Init();
    Logger::Info("file output test info");
    Logger::Warn("file output test warn");
    Logger::Error("file output test error");
    Logger::Shutdown();

    std::string logPath = (std::filesystem::path("logs") / "atlas.log").string();
    assert(std::filesystem::exists(logPath));

    std::ifstream f(logPath);
    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();

    assert(content.find("[INFO]") != std::string::npos);
    assert(content.find("Logger initialized") != std::string::npos);
    assert(content.find("file output test info") != std::string::npos);
    assert(content.find("[WARN]") != std::string::npos);
    assert(content.find("file output test warn") != std::string::npos);
    assert(content.find("[ERROR]") != std::string::npos);
    assert(content.find("file output test error") != std::string::npos);

    std::filesystem::remove_all("logs");

    std::cout << "[PASS] test_logger_writes_to_file" << std::endl;
}
