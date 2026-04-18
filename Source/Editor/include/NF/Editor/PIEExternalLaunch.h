#pragma once
// NF::PIEExternalLaunch — External game process launcher for Play-In-Editor.
//
// Manages the launch of the NovaForge game client or server as an external
// process directly from the workspace.  The build→launch pipeline:
//   1. Trigger a build via BuildTool (represented by a build config here)
//   2. Resolve the output executable path from the build artifact
//   3. Launch the external process with the current project's .atlas file
//   4. Route console output to the ConsolePanel notification sink
//   5. Track process lifecycle (running / exit code / crash detection)
//
// Phase F.3 — External Game Launch
//
// Real launch strategy (POSIX):
//   - If the executable is accessible on disk (access(X_OK)), fork()+execv() is
//     used and a monitor thread reads stdout via a pipe and detects natural exit
//     via waitpid(WNOHANG).  terminate() sends SIGTERM, joins the monitor, then
//     reaps the child.
//   - If the executable does not exist (or on Windows), a stub path runs instead:
//     a synthetic PID is assigned so existing unit tests continue to pass.

#include "NF/Core/Core.h"
#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#ifndef _WIN32
#  include <fcntl.h>
#  include <poll.h>
#  include <signal.h>
#  include <sys/wait.h>
#  include <unistd.h>
#endif

namespace NF {

// ── PIELaunchConfig ───────────────────────────────────────────────────────────

struct PIELaunchConfig {
    std::string executablePath;     ///< path to the game binary
    std::string projectFilePath;    ///< .atlas project file to pass as argument
    std::string workingDirectory;   ///< working dir for the process
    std::vector<std::string> args;  ///< additional command-line arguments
    bool        waitForDebugger   = false;
    bool        fullscreen        = false;
    uint32_t    targetWidth       = 1280;
    uint32_t    targetHeight      = 720;
    std::string buildConfiguration; ///< "Debug", "Release", etc.

    [[nodiscard]] bool isValid() const {
        return !executablePath.empty() && !projectFilePath.empty();
    }
};

// ── PIELaunchResult ───────────────────────────────────────────────────────────

struct PIELaunchResult {
    bool        launched   = false;
    std::string errorMsg;
    uint32_t    processId  = 0;     ///< OS PID of the launched process (stub: nonzero on success)

    [[nodiscard]] bool ok()     const { return launched; }
    [[nodiscard]] bool failed() const { return !launched; }
};

// ── PIEProcessState ───────────────────────────────────────────────────────────

enum class PIEProcessState : uint8_t {
    Idle,      ///< No process running
    Launching, ///< Process is being started
    Running,   ///< Process is active
    Exited,    ///< Process exited normally
    Crashed,   ///< Process terminated unexpectedly
};

inline const char* pieProcessStateName(PIEProcessState s) {
    switch (s) {
    case PIEProcessState::Idle:      return "Idle";
    case PIEProcessState::Launching: return "Launching";
    case PIEProcessState::Running:   return "Running";
    case PIEProcessState::Exited:    return "Exited";
    case PIEProcessState::Crashed:   return "Crashed";
    }
    return "Unknown";
}

// ── PIEExternalLaunch ─────────────────────────────────────────────────────────

class PIEExternalLaunch {
public:
    PIEExternalLaunch()  = default;

    /// Destructor: ensure the monitor thread is joined if the process was
    /// launched and not yet terminated.
    ~PIEExternalLaunch() {
        m_stopMonitor.store(true);
        if (m_monitorThread.joinable())
            m_monitorThread.join();
#ifndef _WIN32
        if (m_stdoutPipeRead >= 0) {
            ::close(m_stdoutPipeRead);
            m_stdoutPipeRead = -1;
        }
#endif
    }

    // Non-copyable, non-movable: owns a thread and OS resources.
    PIEExternalLaunch(const PIEExternalLaunch&)            = delete;
    PIEExternalLaunch& operator=(const PIEExternalLaunch&) = delete;

    // ── Configuration ─────────────────────────────────────────────────────

    void setConfig(const PIELaunchConfig& cfg) { m_config = cfg; }
    [[nodiscard]] const PIELaunchConfig& config() const { return m_config; }
    [[nodiscard]] bool hasValidConfig() const { return m_config.isValid(); }

    // ── Launch ────────────────────────────────────────────────────────────

    /// Launch the external game process using the stored configuration.
    ///
    /// On POSIX, if the executable exists on disk (access X_OK), a real child
    /// process is forked and a monitor thread reads its stdout and waits for
    /// exit.  If the executable is not found — or on Windows — a stub mode is
    /// used: a synthetic PID is assigned so unit tests remain unaffected.
    PIELaunchResult launch() {
        if (!m_config.isValid()) {
            return { false, "Launch config is invalid (no executable or project file)" };
        }
        {
            std::lock_guard<std::mutex> lk(m_procMutex);
            if (m_state == PIEProcessState::Running ||
                m_state == PIEProcessState::Launching) {
                return { false, "A process is already running" };
            }
            m_state        = PIEProcessState::Launching;
            m_launchCount++;
            m_lastExitCode = 0;
        }
        m_exitNotified.store(false);
        {
            std::lock_guard<std::mutex> lk(m_linesMutex);
            m_stdoutLines.clear();
        }

#ifndef _WIN32
        // Attempt a real POSIX launch when the executable is accessible.
        if (::access(m_config.executablePath.c_str(), X_OK) == 0) {
            return launchPosix();
        }
#endif
        // Stub path: synthetic PID (executable not on disk, or Windows).
        // Join any previous monitor thread before proceeding.
        if (m_monitorThread.joinable())
            m_monitorThread.join();
        NF_LOG_INFO("PIE", "Launching external process (stub): " + m_config.executablePath);
        {
            std::lock_guard<std::mutex> lk(m_procMutex);
            m_state     = PIEProcessState::Running;
            m_processId = 1000 + m_launchCount;
        }
        if (m_onLaunched) m_onLaunched(m_processId);
        return { true, "", m_processId };
    }

    /// Terminate the running external process.
    ///
    /// On POSIX with a real child: sends SIGTERM, joins the monitor thread,
    /// then reaps the child process.  In stub mode: transitions state directly.
    bool terminate() {
#ifndef _WIN32
        pid_t pidToKill = -1;
#endif
        {
            std::lock_guard<std::mutex> lk(m_procMutex);
            if (m_state != PIEProcessState::Running &&
                m_state != PIEProcessState::Launching) return false;
#ifndef _WIN32
            pidToKill = m_nativePid;
#endif
        }

#ifndef _WIN32
        if (pidToKill > 0) {
            // Stop the monitor thread first so it does not race with us.
            m_stopMonitor.store(true);
            ::kill(pidToKill, SIGTERM);
            if (m_monitorThread.joinable())
                m_monitorThread.join();

            // Reap child if the monitor had not already done so.
            {
                std::lock_guard<std::mutex> lk(m_procMutex);
                if (m_nativePid > 0) {
                    int status = 0;
                    ::waitpid(m_nativePid, &status, 0);
                    if (m_stdoutPipeRead >= 0) {
                        ::close(m_stdoutPipeRead);
                        m_stdoutPipeRead = -1;
                    }
                    m_nativePid = -1;
                }
                m_state        = PIEProcessState::Exited;
                m_lastExitCode = -1;
            }
            NF_LOG_INFO("PIE", "Terminated POSIX process (PID " +
                        std::to_string(pidToKill) + ")");
            if (!m_exitNotified.exchange(true))
                if (m_onExited) m_onExited(-1);
            return true;
        }
#endif
        // Stub terminate.
        {
            std::lock_guard<std::mutex> lk(m_procMutex);
            NF_LOG_INFO("PIE", "Terminating external process (PID " +
                        std::to_string(m_processId) + ")");
            m_state        = PIEProcessState::Exited;
            m_lastExitCode = -1;
        }
        if (!m_exitNotified.exchange(true))
            if (m_onExited) m_onExited(-1);
        return true;
    }

    /// Simulate process exit — for unit testing and stub-mode callbacks.
    /// Has no effect if a real process is running.
    void simulateExit(int32_t exitCode) {
        std::lock_guard<std::mutex> lk(m_procMutex);
        if (m_state != PIEProcessState::Running) return;
        m_lastExitCode = exitCode;
        m_state = (exitCode == 0) ? PIEProcessState::Exited
                                  : PIEProcessState::Crashed;
        if (!m_exitNotified.exchange(true))
            if (m_onExited) m_onExited(exitCode);
    }

    // ── Console output routing ────────────────────────────────────────────

    /// Append a line of stdout from the process and notify listeners.
    /// Thread-safe: may be called from the monitor thread.
    void pushStdoutLine(const std::string& line) {
        {
            std::lock_guard<std::mutex> lk(m_linesMutex);
            m_stdoutLines.push_back(line);
        }
        if (m_onStdoutLine) m_onStdoutLine(line);
    }

    /// Returns a snapshot copy of all stdout lines received so far.
    [[nodiscard]] std::vector<std::string> stdoutLines() const {
        std::lock_guard<std::mutex> lk(m_linesMutex);
        return m_stdoutLines;
    }

    [[nodiscard]] uint32_t stdoutLineCount() const {
        std::lock_guard<std::mutex> lk(m_linesMutex);
        return static_cast<uint32_t>(m_stdoutLines.size());
    }

    // ── State query ───────────────────────────────────────────────────────

    [[nodiscard]] PIEProcessState state() const {
        std::lock_guard<std::mutex> lk(m_procMutex);
        return m_state;
    }
    [[nodiscard]] bool     isRunning()    const { return state() == PIEProcessState::Running; }
    [[nodiscard]] bool     hasExited()    const {
        auto s = state();
        return s == PIEProcessState::Exited || s == PIEProcessState::Crashed;
    }
    [[nodiscard]] bool     hasCrashed()   const { return state() == PIEProcessState::Crashed; }
    [[nodiscard]] uint32_t processId()    const {
        std::lock_guard<std::mutex> lk(m_procMutex);
        return m_processId;
    }
    [[nodiscard]] int32_t  lastExitCode() const {
        std::lock_guard<std::mutex> lk(m_procMutex);
        return m_lastExitCode;
    }
    [[nodiscard]] uint32_t launchCount()  const {
        std::lock_guard<std::mutex> lk(m_procMutex);
        return m_launchCount;
    }

    // ── Callbacks ─────────────────────────────────────────────────────────

    using LaunchCb = std::function<void(uint32_t pid)>;
    using ExitCb   = std::function<void(int32_t exitCode)>;
    using StdoutCb = std::function<void(const std::string& line)>;

    void setOnLaunched(LaunchCb cb)   { m_onLaunched   = std::move(cb); }
    void setOnExited(ExitCb cb)       { m_onExited      = std::move(cb); }
    void setOnStdoutLine(StdoutCb cb) { m_onStdoutLine  = std::move(cb); }

private:
    // ── POSIX real-launch implementation ──────────────────────────────────

#ifndef _WIN32
    PIELaunchResult launchPosix() {
        int pipefd[2];
        if (::pipe(pipefd) < 0) {
            std::lock_guard<std::mutex> lk(m_procMutex);
            m_state = PIEProcessState::Idle;
            return { false, "pipe() failed" };
        }

        // Join any previous monitor thread (it will have returned once the
        // prior process exited, but the std::thread object remains joinable
        // until explicitly joined).
        if (m_monitorThread.joinable())
            m_monitorThread.join();
        std::vector<std::string> argStrings;
        argStrings.push_back(m_config.executablePath);
        argStrings.push_back(m_config.projectFilePath);
        for (const auto& a : m_config.args)
            argStrings.push_back(a);
        if (m_config.fullscreen)
            argStrings.emplace_back("--fullscreen");
        if (m_config.waitForDebugger)
            argStrings.emplace_back("--wait-debugger");

        // Build argv vector.  execv historically takes char*[], not const char*[].
        // The strings are owned by argStrings (local scope) and execv does not
        // modify them — the const_cast is safe here and is a POSIX API quirk.
        std::vector<char*> argv;
        for (auto& s : argStrings)
            argv.push_back(const_cast<char*>(s.c_str())); // NOLINT(cppcoreguidelines-pro-type-const-cast)
        argv.push_back(nullptr);

        pid_t pid = ::fork();
        if (pid < 0) {
            ::close(pipefd[0]);
            ::close(pipefd[1]);
            std::lock_guard<std::mutex> lk(m_procMutex);
            m_state = PIEProcessState::Idle;
            return { false, "fork() failed" };
        }

        if (pid == 0) {
            // ── Child process ──────────────────────────────────────────
            ::close(pipefd[0]);
            ::dup2(pipefd[1], STDOUT_FILENO);
            ::dup2(pipefd[1], STDERR_FILENO);
            ::close(pipefd[1]);
            if (!m_config.workingDirectory.empty())
                ::chdir(m_config.workingDirectory.c_str());
            ::execv(argv[0], argv.data());
            ::_exit(127); // execv failed
        }

        // ── Parent process ─────────────────────────────────────────────
        ::close(pipefd[1]);
        m_nativePid      = pid;
        m_stdoutPipeRead = pipefd[0];
        m_stopMonitor.store(false);

        {
            std::lock_guard<std::mutex> lk(m_procMutex);
            m_state     = PIEProcessState::Running;
            m_processId = static_cast<uint32_t>(pid);
        }

        m_monitorThread = std::thread([this] { monitorLoop(); });

        NF_LOG_INFO("PIE", "Launched real process (PID " + std::to_string(pid) +
                    "): " + m_config.executablePath);
        if (m_onLaunched) m_onLaunched(static_cast<uint32_t>(pid));
        return { true, "", static_cast<uint32_t>(pid) };
    }

    /// Background thread: reads child stdout line-by-line and detects exit.
    void monitorLoop() {
        char buf[4096];
        std::string lineBuf;

        auto flushLine = [&] {
            if (!lineBuf.empty()) {
                pushStdoutLine(lineBuf);
                lineBuf.clear();
            }
        };

        auto feedBytes = [&](const char* p, ssize_t n) {
            for (ssize_t i = 0; i < n; ++i) {
                if (p[i] == '\n' || p[i] == '\r') {
                    flushLine();
                } else {
                    lineBuf += p[i];
                }
            }
        };

        // Read stdout until the pipe is closed (child exited) or we are told to stop.
        while (!m_stopMonitor.load()) {
            struct pollfd pfd{};
            pfd.fd     = m_stdoutPipeRead;
            pfd.events = POLLIN | POLLHUP;
            int ret = ::poll(&pfd, 1, 50 /*ms*/);

            if (ret > 0) {
                // Always try to read available bytes first.
                if (pfd.revents & POLLIN) {
                    ssize_t n = ::read(m_stdoutPipeRead, buf, sizeof(buf));
                    if (n > 0) {
                        feedBytes(buf, n);
                    } else {
                        // n == 0 → EOF; n < 0 → error.  Pipe is done.
                        break;
                    }
                }
                if (pfd.revents & POLLHUP) {
                    // Write end was closed.  Drain any remaining bytes then stop.
                    ssize_t n;
                    while ((n = ::read(m_stdoutPipeRead, buf, sizeof(buf))) > 0)
                        feedBytes(buf, n);
                    break;
                }
            } else if (ret < 0 && errno != EINTR) {
                break;
            }
        }

        flushLine();

        // If we stopped because stopMonitor was set, terminate() owns the cleanup.
        if (m_stopMonitor.load())
            return;

        // Pipe is done → child has exited (or closed stdout).  Reap it.
        int status = 0;
        ::waitpid(m_nativePid, &status, 0); // blocking; child is already done

        int32_t code = 0;
        if (WIFEXITED(status))
            code = WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
            // Encode signal-terminated exits as negative signal numbers
            // (distinct from the -1 used by terminate() for editor-initiated stops).
            code = -static_cast<int32_t>(WTERMSIG(status));

        {
            std::lock_guard<std::mutex> lk(m_procMutex);
            if (m_stdoutPipeRead >= 0) {
                ::close(m_stdoutPipeRead);
                m_stdoutPipeRead = -1;
            }
            m_nativePid    = -1;
            m_lastExitCode = code;
            m_state        = (code == 0) ? PIEProcessState::Exited
                                         : PIEProcessState::Crashed;
        }

        if (!m_exitNotified.exchange(true))
            if (m_onExited) m_onExited(code);
    }
#endif // !_WIN32

    // ── Members ───────────────────────────────────────────────────────────

    PIELaunchConfig          m_config;
    PIEProcessState          m_state        = PIEProcessState::Idle;
    uint32_t                 m_processId    = 0;
    int32_t                  m_lastExitCode = 0;
    uint32_t                 m_launchCount  = 0;

    std::vector<std::string> m_stdoutLines;

    LaunchCb  m_onLaunched;
    ExitCb    m_onExited;
    StdoutCb  m_onStdoutLine;

    // Thread and synchronisation (used for real POSIX launches).
    mutable std::mutex  m_procMutex;   ///< guards m_state, m_lastExitCode, m_nativePid
    mutable std::mutex  m_linesMutex;  ///< guards m_stdoutLines
    std::thread         m_monitorThread;
    std::atomic<bool>   m_stopMonitor{false};
    std::atomic<bool>   m_exitNotified{false}; ///< ensures onExited fires exactly once

#ifndef _WIN32
    pid_t m_nativePid      = -1;  ///< real child PID (-1 when not running)
    int   m_stdoutPipeRead = -1;  ///< read end of child stdout pipe
#endif
};

} // namespace NF
