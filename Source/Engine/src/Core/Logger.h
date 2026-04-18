#pragma once
#include <string>
#include <fstream>
#include <functional>
#include <mutex>
#include <ostream>

namespace atlas {

class Logger {
public:
    /// Callback type for log sinks.  Receives the fully-formatted log line.
    using SinkCallback = std::function<void(const std::string&)>;

    static void Init();
    static void Shutdown();
    static void Info(const std::string& msg);
    static void Warn(const std::string& msg);
    static void Error(const std::string& msg);

    /// Register a callback that receives every log line.
    static void SetSink(SinkCallback sink);

private:
    static void Write(const std::string& line, std::ostream& console);

    static std::ofstream s_logFile;
    static std::mutex s_mutex;
    static SinkCallback s_sink;
};

}
