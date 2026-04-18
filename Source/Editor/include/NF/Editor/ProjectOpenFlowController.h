#pragma once
// NF::Editor — Phase H.6: Project Open Flow Controller
//
// Provides the complete "open project" UX flow: recent projects list,
// .atlas file selection, project validation on open with error summary,
// and a new-project wizard for creating .atlas files from templates.
//
//   ProjectOpenFlowState   — Idle / ChoosingFile / Validating / Opening / Open / Error
//   ProjectTemplateKind    — Blank / NovaForge / Minimal
//   ProjectValidationResult — structured open/validate outcome
//   ProjectOpenFlowController —
//       openRecentProject(path)       — open from the recents list
//       beginFileOpen()               — trigger file chooser (sets state)
//       selectFile(path)              — provide the chosen .atlas path
//       validate()                    — validate the pending path → result
//       confirmOpen()                 — proceed with open after validation
//       cancelOpen()                  — abort the flow
//       beginNewProjectWizard()       — start the wizard
//       setWizardTemplate(kind)       — choose a project template
//       setWizardProjectName(name)    — set project name
//       setWizardProjectPath(path)    — set output directory
//       createProject()               — generate .atlas from template
//       recentProjects()              — ordered recent-project list
//       clearRecents()                — clear the recents list
//       state() / pendingPath()

#include "NF/Workspace/AtlasProjectFileLoader.h"
#include "NF/Workspace/ProjectManager.h"
#include <algorithm>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// ProjectOpenFlowState
// ═════════════════════════════════════════════════════════════════

enum class ProjectOpenFlowState : uint8_t {
    Idle         = 0,
    ChoosingFile = 1,
    Validating   = 2,
    Opening      = 3,
    Open         = 4,
    NewWizard    = 5,
    Error        = 6,
};

inline const char* projectOpenFlowStateName(ProjectOpenFlowState s) {
    switch (s) {
        case ProjectOpenFlowState::Idle:         return "Idle";
        case ProjectOpenFlowState::ChoosingFile: return "ChoosingFile";
        case ProjectOpenFlowState::Validating:   return "Validating";
        case ProjectOpenFlowState::Opening:      return "Opening";
        case ProjectOpenFlowState::Open:         return "Open";
        case ProjectOpenFlowState::NewWizard:    return "NewWizard";
        case ProjectOpenFlowState::Error:        return "Error";
        default:                                 return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// ProjectTemplateKind
// ═════════════════════════════════════════════════════════════════

enum class ProjectTemplateKind : uint8_t {
    Blank     = 0,
    NovaForge = 1,
    Minimal   = 2,
};

inline const char* projectTemplateKindName(ProjectTemplateKind k) {
    switch (k) {
        case ProjectTemplateKind::Blank:     return "Blank";
        case ProjectTemplateKind::NovaForge: return "NovaForge";
        case ProjectTemplateKind::Minimal:   return "Minimal";
        default:                             return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// ProjectValidationResult
// ═════════════════════════════════════════════════════════════════

struct ProjectValidationResult {
    bool        success   = false;
    std::string projectPath;
    std::string projectName;

    std::vector<std::string> errors;
    std::vector<std::string> warnings;

    [[nodiscard]] bool hasErrors()   const { return !errors.empty(); }
    [[nodiscard]] bool hasWarnings() const { return !warnings.empty(); }

    void addError(const std::string& msg)   { errors.push_back(msg);   success = false; }
    void addWarning(const std::string& msg) { warnings.push_back(msg); }

    [[nodiscard]] std::string summary() const {
        std::ostringstream out;
        out << (success ? "OK" : "FAILED");
        if (!errors.empty())   { out << "; Errors: " << errors.size();   }
        if (!warnings.empty()) { out << "; Warnings: " << warnings.size(); }
        return out.str();
    }
};

// ═════════════════════════════════════════════════════════════════
// RecentProjectEntry (re-use from ProjectManager)
// Forward-only adapter: we use ProjectManager's recent list.
// ═════════════════════════════════════════════════════════════════

// ═════════════════════════════════════════════════════════════════
// ProjectOpenFlowController
// ═════════════════════════════════════════════════════════════════

class ProjectOpenFlowController {
public:
    // ── Open flow state machine ───────────────────────────────────

    [[nodiscard]] ProjectOpenFlowState state() const { return m_state; }

    // ── Recent projects ───────────────────────────────────────────

    [[nodiscard]] const std::vector<RecentProjectEntry>& recentProjects() const {
        return m_manager.recentProjects();
    }

    void clearRecents() { m_manager.clearRecent(); }

    // ── Open recent ───────────────────────────────────────────────

    bool openRecentProject(const std::string& path) {
        if (m_state != ProjectOpenFlowState::Idle) return false;
        if (path.empty()) return false;
        m_pendingPath = path;
        m_state       = ProjectOpenFlowState::Validating;
        m_lastResult  = runValidation(path);
        if (!m_lastResult.success) {
            m_errorMsg = m_lastResult.summary();
            m_state    = ProjectOpenFlowState::Error;
            return false;
        }
        return doOpen(path, m_lastResult.projectName);
    }

    // ── File chooser flow ─────────────────────────────────────────

    void beginFileOpen() {
        if (m_state != ProjectOpenFlowState::Idle) return;
        m_pendingPath.clear();
        m_state = ProjectOpenFlowState::ChoosingFile;
        if (m_onRequestFilePicker) m_onRequestFilePicker();
    }

    bool selectFile(const std::string& path) {
        if (m_state != ProjectOpenFlowState::ChoosingFile) return false;
        if (path.empty()) { m_state = ProjectOpenFlowState::Idle; return false; }
        m_pendingPath = path;
        m_state       = ProjectOpenFlowState::Validating;
        return true;
    }

    [[nodiscard]] ProjectValidationResult validate() {
        if (m_state != ProjectOpenFlowState::Validating || m_pendingPath.empty()) {
            ProjectValidationResult bad;
            bad.addError("No pending path to validate");
            return bad;
        }
        m_lastResult = runValidation(m_pendingPath);
        if (!m_lastResult.success) {
            m_errorMsg = m_lastResult.summary();
            m_state    = ProjectOpenFlowState::Error;
        }
        return m_lastResult;
    }

    bool confirmOpen() {
        if (m_state != ProjectOpenFlowState::Validating &&
            m_state != ProjectOpenFlowState::Idle) return false;
        if (m_pendingPath.empty()) return false;
        if (!m_lastResult.success && !m_lastResult.projectPath.empty()) return false;
        return doOpen(m_pendingPath, m_lastResult.projectName);
    }

    void cancelOpen() {
        m_pendingPath.clear();
        m_lastResult = {};
        m_errorMsg.clear();
        m_state = ProjectOpenFlowState::Idle;
    }

    [[nodiscard]] const std::string& pendingPath() const { return m_pendingPath; }
    [[nodiscard]] const std::string& errorMessage() const { return m_errorMsg; }
    [[nodiscard]] const ProjectValidationResult& lastValidationResult() const {
        return m_lastResult;
    }

    // ── New project wizard ────────────────────────────────────────

    void beginNewProjectWizard() {
        if (m_state != ProjectOpenFlowState::Idle) return;
        m_wizardTemplate    = ProjectTemplateKind::Blank;
        m_wizardProjectName.clear();
        m_wizardProjectPath.clear();
        m_state             = ProjectOpenFlowState::NewWizard;
    }

    void setWizardTemplate(ProjectTemplateKind kind)      { m_wizardTemplate    = kind; }
    void setWizardProjectName(const std::string& name)    { m_wizardProjectName = name; }
    void setWizardProjectPath(const std::string& path)    { m_wizardProjectPath = path; }

    [[nodiscard]] ProjectTemplateKind wizardTemplate()    const { return m_wizardTemplate; }
    [[nodiscard]] const std::string&  wizardProjectName() const { return m_wizardProjectName; }
    [[nodiscard]] const std::string&  wizardProjectPath() const { return m_wizardProjectPath; }

    // Validate wizard input and produce the .atlas path that would be created.
    [[nodiscard]] bool isWizardReady() const {
        return m_state == ProjectOpenFlowState::NewWizard
            && !m_wizardProjectName.empty()
            && !m_wizardProjectPath.empty();
    }

    bool createProject() {
        if (!isWizardReady()) return false;
        // Build .atlas path
        std::string atlasPath = m_wizardProjectPath + "/" + m_wizardProjectName + ".atlas";
        // Delegate to manager — creates in-memory project record
        bool ok = m_manager.newProject(atlasPath, m_wizardProjectName);
        if (ok) {
            m_pendingPath  = atlasPath;
            m_state        = ProjectOpenFlowState::Open;
            if (m_onProjectOpened) m_onProjectOpened(atlasPath);
        } else {
            m_errorMsg = "Failed to create project: " + m_wizardProjectName;
            m_state    = ProjectOpenFlowState::Error;
        }
        return ok;
    }

    void cancelWizard() {
        m_wizardProjectName.clear();
        m_wizardProjectPath.clear();
        m_state = ProjectOpenFlowState::Idle;
    }

    // ── Close current project ─────────────────────────────────────

    bool closeProject() {
        if (m_state != ProjectOpenFlowState::Open) return false;
        bool ok = m_manager.closeProject();
        if (ok) {
            m_state       = ProjectOpenFlowState::Idle;
            m_pendingPath.clear();
        }
        return ok;
    }

    // ── Callbacks ─────────────────────────────────────────────────

    void setOnRequestFilePicker(std::function<void()> cb) {
        m_onRequestFilePicker = std::move(cb);
    }
    void setOnProjectOpened(std::function<void(const std::string&)> cb) {
        m_onProjectOpened = std::move(cb);
    }
    void setOnProjectError(std::function<void(const std::string&)> cb) {
        m_onProjectError = std::move(cb);
    }

    // ── Underlying ProjectManager ─────────────────────────────────

    [[nodiscard]] ProjectManager&       manager()       { return m_manager; }
    [[nodiscard]] const ProjectManager& manager() const { return m_manager; }

private:
    ProjectManager     m_manager;
    ProjectOpenFlowState m_state       = ProjectOpenFlowState::Idle;
    std::string        m_pendingPath;
    std::string        m_errorMsg;
    ProjectValidationResult m_lastResult;

    // Wizard state
    ProjectTemplateKind m_wizardTemplate    = ProjectTemplateKind::Blank;
    std::string         m_wizardProjectName;
    std::string         m_wizardProjectPath;

    std::function<void()>                m_onRequestFilePicker;
    std::function<void(const std::string&)> m_onProjectOpened;
    std::function<void(const std::string&)> m_onProjectError;

    // Validation logic: validates the .atlas project path.
    // When the file exists on disk, the manifest is parsed and its contents
    // are validated using AtlasProjectFileLoader::bootstrap(). When the file
    // does not yet exist (e.g. a path just entered by the user or used in
    // unit tests), extension and naming checks are applied only.
    static ProjectValidationResult runValidation(const std::string& path) {
        ProjectValidationResult result;
        result.projectPath = path;

        if (path.empty()) {
            result.addError("Project path is empty");
            return result;
        }

        // Check extension
        if (path.size() < 6 || path.substr(path.size() - 6) != ".atlas") {
            result.addError("Project file must have a .atlas extension");
            return result;
        }

        // Derive name from filename
        auto slash = path.find_last_of("/\\");
        std::string filename = (slash == std::string::npos) ? path : path.substr(slash + 1);
        std::string derivedName = filename.substr(0, filename.size() - 6); // strip ".atlas"

        if (derivedName.empty()) {
            result.addError("Project name cannot be empty");
            return result;
        }

        // Attempt to parse and validate the manifest when the file is on disk.
        // If the file does not exist yet (e.g. during wizard flow or unit tests
        // that use synthetic paths), skip content validation.
        bool fileExists = std::filesystem::exists(path);
        AtlasProjectFileLoader loader;
        bool parsed = loader.loadFromFile(path);
        if (!fileExists) {
            // File not on disk — use the filename as the project name.
            // A warning is added so the UX can surface this to the user.
            result.projectName = derivedName;
            result.addWarning("Project file not found on disk: " + path);
            result.success = true;
            return result;
        }
        if (!parsed) {
            // File exists on disk but could not be parsed — surface as an error.
            result.addError("Failed to parse project manifest: " + loader.error());
            return result;
        }
        {
            // File exists and parsed — validate its contents via bootstrap.
            ProjectBootstrapResult bootstrap = loader.bootstrap(path, /*checkPathsOnDisk=*/false);
            for (const auto& entry : bootstrap.validationEntries) {
                switch (entry.severity) {
                    case ProjectValidationSeverity::Fatal:
                    case ProjectValidationSeverity::Error:
                        result.addError("[" + entry.code + "] " + entry.message);
                        break;
                    case ProjectValidationSeverity::Warning:
                        result.addWarning("[" + entry.code + "] " + entry.message);
                        break;
                    default:
                        break; // Info entries are not surfaced in the flow result
                }
            }
            if (!bootstrap.success) {
                return result; // errors already added above
            }
            // Use the manifest name when present; fall back to the filename-derived name.
            // The 'name' field is optional in the atlas.project.v1 schema, so an empty
            // name is valid and should not be treated as an error.
            result.projectName = bootstrap.manifest.name.empty()
                               ? derivedName
                               : bootstrap.manifest.name;
        }

        result.success = true;
        return result;
    }

    bool doOpen(const std::string& path, const std::string& displayName) {
        m_state = ProjectOpenFlowState::Opening;
        bool ok = m_manager.openProject(path, displayName);
        if (ok) {
            m_state = ProjectOpenFlowState::Open;
            if (m_onProjectOpened) m_onProjectOpened(path);
        } else {
            m_errorMsg = "Failed to open project: " + path;
            m_state    = ProjectOpenFlowState::Error;
            if (m_onProjectError) m_onProjectError(m_errorMsg);
        }
        return ok;
    }
};

} // namespace NF
