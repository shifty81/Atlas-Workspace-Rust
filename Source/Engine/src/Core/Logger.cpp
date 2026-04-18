#include "Logger.h"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace atlas {

std::ofstream Logger::s_logFile;
std::mutex Logger::s_mutex;
Logger::SinkCallback Logger::s_sink;

static std::string Timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void Logger::Init() {
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        std::filesystem::create_directories("logs");
        std::string logPath = (std::filesystem::path("logs") / "atlas.log").string();
        s_logFile.open(logPath, std::ios::out | std::ios::app);
        if (!s_logFile.is_open()) {
            std::cerr << "[ERROR] Failed to open log file: " << logPath << std::endl;
        }
    }
    Info("Logger initialized");
}

void Logger::Shutdown() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (s_logFile.is_open()) {
        s_logFile.flush();
        s_logFile.close();
    }
}

void Logger::Info(const std::string& msg) {
    std::string line = "[INFO] " + Timestamp() + " " + msg;
    Write(line, std::cout);
}

void Logger::Warn(const std::string& msg) {
    std::string line = "[WARN] " + Timestamp() + " " + msg;
    Write(line, std::cout);
}

void Logger::Error(const std::string& msg) {
    std::string line = "[ERROR] " + Timestamp() + " " + msg;
    Write(line, std::cerr);
}

void Logger::SetSink(SinkCallback sink) {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_sink = std::move(sink);
}

void Logger::Write(const std::string& line, std::ostream& console) {
    SinkCallback sinkCopy;
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        console << line << std::endl;
        if (s_logFile.is_open()) {
            s_logFile << line << std::endl;
        }
        sinkCopy = s_sink;
    }
    // Invoke the sink outside the lock to avoid deadlocks if the callback
    // tries to call Logger methods.
    if (sinkCopy) {
        sinkCopy(line);
    }
}

}
