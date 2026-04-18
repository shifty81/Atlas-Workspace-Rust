#pragma once
// NF::Editor — Workspace launch contract: launch context, CLI args, IPC,
//              and the WorkspaceLaunchService interface.
//
// Rule: no panel, subsystem, or plugin may spawn child executables directly.
//       All launches must go through WorkspaceLaunchService.
#include "NF/Workspace/WorkspaceAppRegistry.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// Windows types (HWND, DWORD, HANDLE, etc.) must be in the global namespace.
// Include windows.h here, before namespace NF, so that the types land at
// global scope even when this header is pulled in transitively.
#if defined(_WIN32)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#endif

namespace NF {

// ── WorkspaceLaunchMode ──────────────────────────────────────────
// How the child process is being started.

enum class WorkspaceLaunchMode : uint8_t {
    Hosted   = 0,  // launched by Workspace; full contract active
    Headless = 1,  // CI / server; no UI, log-only
    Preview  = 2,  // quick run-in-editor preview mode
};

inline const char* workspaceLaunchModeName(WorkspaceLaunchMode m) {
    switch (m) {
        case WorkspaceLaunchMode::Hosted:   return "hosted";
        case WorkspaceLaunchMode::Headless: return "headless";
        case WorkspaceLaunchMode::Preview:  return "preview";
        default:                            return "unknown";
    }
}

// ── WorkspaceLaunchContext ───────────────────────────────────────
// Everything Workspace provides to a child process at startup.
// This is the authoritative launch data object.

struct WorkspaceLaunchContext {
    std::string workspaceRoot;    // absolute path to workspace root
    std::string projectPath;      // absolute path to project.atlas.json
    std::string sessionId;        // unique ID for this launch session
    std::string ipcEndpoint;      // named pipe or socket path for IPC
    std::string logSinkPath;      // child process writes logs here
    WorkspaceLaunchMode mode = WorkspaceLaunchMode::Hosted;

    [[nodiscard]] bool isValid() const {
        return !workspaceRoot.empty()
            && !projectPath.empty()
            && !sessionId.empty();
    }

    // Build the canonical CLI argument list from this context.
    // Child executables must accept these arguments.
    [[nodiscard]] std::vector<std::string> toArgs() const {
        std::vector<std::string> args;
        args.push_back("--" + std::string(workspaceLaunchModeName(mode)));
        args.push_back("--workspace-root=" + workspaceRoot);
        args.push_back("--project=" + projectPath);
        args.push_back("--session-id=" + sessionId);
        if (!ipcEndpoint.empty())
            args.push_back("--ipc=" + ipcEndpoint);
        if (!logSinkPath.empty())
            args.push_back("--log=" + logSinkPath);
        return args;
    }
};

// ── WorkspaceLaunchStatus ────────────────────────────────────────

enum class WorkspaceLaunchStatus : uint8_t {
    Success            = 0,
    AppNotFound        = 1,   // not in registry
    ExecutableNotFound = 2,   // binary does not exist on disk
    InvalidContext     = 3,   // WorkspaceLaunchContext::isValid() == false
    AlreadyRunning     = 4,   // one instance already active
    SpawnFailed        = 5,   // OS-level process creation failed
};

inline const char* workspaceLaunchStatusName(WorkspaceLaunchStatus s) {
    switch (s) {
        case WorkspaceLaunchStatus::Success:             return "Success";
        case WorkspaceLaunchStatus::AppNotFound:         return "AppNotFound";
        case WorkspaceLaunchStatus::ExecutableNotFound:  return "ExecutableNotFound";
        case WorkspaceLaunchStatus::InvalidContext:      return "InvalidContext";
        case WorkspaceLaunchStatus::AlreadyRunning:      return "AlreadyRunning";
        case WorkspaceLaunchStatus::SpawnFailed:         return "SpawnFailed";
        default:                                         return "Unknown";
    }
}

// ── WorkspaceLaunchResult ────────────────────────────────────────

struct WorkspaceLaunchResult {
    WorkspaceLaunchStatus status = WorkspaceLaunchStatus::SpawnFailed;
    uint32_t              pid    = 0;   // OS process ID (0 if not running)
    std::string           errorDetail;  // human-readable error for logging

    [[nodiscard]] bool succeeded() const {
        return status == WorkspaceLaunchStatus::Success;
    }
    [[nodiscard]] bool isRunning() const { return succeeded() && pid != 0; }
};

// ── WorkspaceLaunchService ───────────────────────────────────────
// Owns all child process launch logic.
// Concrete platform implementations live in Source/Programs/AtlasWorkspace/.
// The interface lives here so it can be tested and stubbed.

class WorkspaceLaunchService {
public:
    virtual ~WorkspaceLaunchService() = default;

    // Launch the app described by |app| using |context|.
    // Returns a result with PID on success.
    virtual WorkspaceLaunchResult launchApp(const WorkspaceAppDescriptor& app,
                                            const WorkspaceLaunchContext& context) = 0;

    // Returns true if the app with |id| is currently running.
    virtual bool isRunning(WorkspaceAppId id) const = 0;

    // Request a graceful shutdown of the running app with |id|.
    virtual void shutdownApp(WorkspaceAppId id) = 0;

    // Returns the PID of the running app, or 0 if not running.
    virtual uint32_t pidOf(WorkspaceAppId id) const = 0;
};

// ── NullLaunchService ────────────────────────────────────────────
// No-op implementation used in headless tests and the editor layer.
// Does not spawn any processes.

class NullLaunchService final : public WorkspaceLaunchService {
public:
    WorkspaceLaunchResult launchApp(const WorkspaceAppDescriptor& app,
                                    const WorkspaceLaunchContext& context) override {
        WorkspaceLaunchResult r;
        if (!app.isValid()) {
            r.status      = WorkspaceLaunchStatus::AppNotFound;
            r.errorDetail = "Invalid descriptor for " + app.name;
            return r;
        }
        if (!context.isValid()) {
            r.status      = WorkspaceLaunchStatus::InvalidContext;
            r.errorDetail = "WorkspaceLaunchContext is incomplete";
            return r;
        }
        // Null service records the launch without actually spawning
        m_running[static_cast<uint16_t>(app.id)] = true;
        r.status = WorkspaceLaunchStatus::Success;
        r.pid    = 1;   // synthetic PID
        return r;
    }

    bool isRunning(WorkspaceAppId id) const override {
        auto it = m_running.find(static_cast<uint16_t>(id));
        return it != m_running.end() && it->second;
    }

    void shutdownApp(WorkspaceAppId id) override {
        m_running[static_cast<uint16_t>(id)] = false;
    }

    uint32_t pidOf(WorkspaceAppId id) const override {
        return isRunning(id) ? 1u : 0u;
    }

private:
    // uint16_t key maps WorkspaceAppId to running state
    std::unordered_map<uint16_t, bool> m_running;
};

// ── Win32LaunchService ───────────────────────────────────────────
// Real process-spawning implementation for Windows.
// Uses CreateProcessW to launch child executables located relative to
// a base directory (typically the directory of AtlasWorkspace.exe).
//
// Only compiled on _WIN32 — on other platforms the interface falls back
// to NullLaunchService semantics via the guarded implementation.

#if defined(_WIN32)

class Win32LaunchService final : public WorkspaceLaunchService {
public:
    // |baseDir| is the directory that contains the child executables.
    // Typically this is the directory of AtlasWorkspace.exe itself.
    explicit Win32LaunchService(std::string baseDir)
        : m_baseDir(std::move(baseDir)) {}

    ~Win32LaunchService() override {
        // Close all open process handles on destruction.
        for (auto& entry : m_processes) {
            if (entry.second.handle != INVALID_HANDLE_VALUE) {
                CloseHandle(entry.second.handle);
            }
        }
    }

    WorkspaceLaunchResult launchApp(const WorkspaceAppDescriptor& app,
                                    const WorkspaceLaunchContext& context) override {
        WorkspaceLaunchResult r;

        if (!app.isValid()) {
            r.status      = WorkspaceLaunchStatus::AppNotFound;
            r.errorDetail = "Invalid descriptor for: " + app.name;
            return r;
        }
        if (!context.isValid()) {
            r.status      = WorkspaceLaunchStatus::InvalidContext;
            r.errorDetail = "WorkspaceLaunchContext is incomplete";
            return r;
        }

        // Reap any previous instance that has exited so it can be relaunched.
        reapIfExited(app.id);

        if (isRunning(app.id)) {
            r.status      = WorkspaceLaunchStatus::AlreadyRunning;
            r.errorDetail = app.name + " is already running (pid="
                            + std::to_string(pidOf(app.id)) + ")";
            return r;
        }

        // Build full path to the executable.
        std::string exePath = m_baseDir.empty()
            ? app.executablePath
            : m_baseDir + "\\" + app.executablePath;

        // Check the file exists before attempting CreateProcessW.
        DWORD attrs = GetFileAttributesA(exePath.c_str());
        if (attrs == INVALID_FILE_ATTRIBUTES) {
            r.status      = WorkspaceLaunchStatus::ExecutableNotFound;
            r.errorDetail = "Executable not found: " + exePath;
            return r;
        }

        // Build command line: executable path followed by default args and
        // context args. We append them all into one quoted command string.
        // Format: "path\to\exe.exe" --arg1 --arg2 ...
        std::string cmdLine = "\"" + exePath + "\"";
        for (const auto& arg : app.defaultArgs)
            cmdLine += " " + arg;
        for (const auto& arg : context.toArgs())
            cmdLine += " " + arg;

        // Convert to wide string for Win32 API.
        // MultiByteToWideChar returns the character count *including* the null
        // terminator when cchMultiByte is -1, so allocate exactly that many
        // wchar_t cells and then pop the embedded null before use.
        int wlen = MultiByteToWideChar(CP_UTF8, 0, cmdLine.c_str(), -1, nullptr, 0);
        std::wstring wCmd(static_cast<size_t>(wlen), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, cmdLine.c_str(), -1, wCmd.data(), wlen);
        // Remove the embedded null so wCmd.size() reflects the real length.
        if (!wCmd.empty() && wCmd.back() == L'\0') wCmd.pop_back();

        STARTUPINFOW si{};
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi{};

        BOOL ok = CreateProcessW(
            nullptr,        // application name (embedded in lpCommandLine)
            wCmd.data(),    // command line (mutable buffer required by API)
            nullptr,        // process security attributes
            nullptr,        // thread security attributes
            FALSE,          // do not inherit handles
            0,              // creation flags
            nullptr,        // inherit environment
            nullptr,        // inherit current directory
            &si,
            &pi);

        if (!ok) {
            DWORD err = GetLastError();
            r.status      = WorkspaceLaunchStatus::SpawnFailed;
            r.errorDetail = "CreateProcessW failed (error=" + std::to_string(err)
                            + ") for: " + exePath;
            return r;
        }

        // Store the process handle for lifecycle tracking.
        // Close the thread handle immediately — we only need the process handle.
        CloseHandle(pi.hThread);

        ProcessEntry entry;
        entry.handle = pi.hProcess;
        entry.pid    = pi.dwProcessId;
        m_processes[static_cast<uint16_t>(app.id)] = entry;

        r.status = WorkspaceLaunchStatus::Success;
        r.pid    = pi.dwProcessId;
        return r;
    }

    bool isRunning(WorkspaceAppId id) const override {
        auto it = m_processes.find(static_cast<uint16_t>(id));
        if (it == m_processes.end()) return false;
        if (it->second.handle == INVALID_HANDLE_VALUE) return false;
        // Poll the process exit code — STILL_ACTIVE means it is alive.
        DWORD code = 0;
        if (!GetExitCodeProcess(it->second.handle, &code)) return false;
        return code == STILL_ACTIVE;
    }

    void shutdownApp(WorkspaceAppId id) override {
        auto it = m_processes.find(static_cast<uint16_t>(id));
        if (it == m_processes.end()) return;
        if (it->second.handle == INVALID_HANDLE_VALUE) return;
        // Request graceful termination.  WM_CLOSE is not available for
        // console/server processes, so TerminateProcess is used as the
        // portable fallback.  Child processes should handle CTRL_CLOSE_EVENT
        // for a cleaner shutdown path once IPC is wired.
        TerminateProcess(it->second.handle, 0);
        // Allow up to kShutdownTimeoutMs for the process to fully exit before
        // we release the handle.  This prevents zombie handle leaks on teardown.
        WaitForSingleObject(it->second.handle, kShutdownTimeoutMs);
        CloseHandle(it->second.handle);
        it->second.handle = INVALID_HANDLE_VALUE;
        it->second.pid    = 0;
    }

    uint32_t pidOf(WorkspaceAppId id) const override {
        auto it = m_processes.find(static_cast<uint16_t>(id));
        if (it == m_processes.end()) return 0;
        return isRunning(id) ? it->second.pid : 0u;
    }

private:
    // Maximum milliseconds to wait for a child process to exit after
    // TerminateProcess before closing its handle.
    static constexpr DWORD kShutdownTimeoutMs = 2000;

    struct ProcessEntry {
        HANDLE   handle = INVALID_HANDLE_VALUE;
        uint32_t pid    = 0;
    };

    // Close and remove the handle for an app that has already exited so
    // its slot in the map can be reused for a fresh launch.
    void reapIfExited(WorkspaceAppId id) {
        auto it = m_processes.find(static_cast<uint16_t>(id));
        if (it == m_processes.end()) return;
        DWORD code = STILL_ACTIVE;
        if (it->second.handle != INVALID_HANDLE_VALUE) {
            GetExitCodeProcess(it->second.handle, &code);
        }
        if (code != STILL_ACTIVE) {
            if (it->second.handle != INVALID_HANDLE_VALUE) {
                CloseHandle(it->second.handle);
            }
            m_processes.erase(it);
        }
    }

    std::string m_baseDir;
    std::unordered_map<uint16_t, ProcessEntry> m_processes;
};
#endif // _WIN32


} // namespace NF
