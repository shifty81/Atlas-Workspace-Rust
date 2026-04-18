#include "GamePackagerPanel.h"
#include <sstream>

namespace atlas::editor {

void GamePackagerPanel::Draw() {
    m_drawList.Clear();

    // Background
    m_drawList.DrawRect({0, 0, 500, 300}, {30, 30, 30, 255});

    // Title
    m_drawList.DrawRect({0, 0, 500, 24}, {50, 50, 50, 255});
    m_drawList.DrawText({4, 4, 200, 20}, "Game Packager", {220, 220, 220, 255});

    // Settings summary
    m_drawList.DrawText({4, 28, 490, 16}, SettingsSummary(), {200, 200, 200, 255});

    // Build status
    if (m_building) {
        m_drawList.DrawText({4, 50, 300, 16}, "Building: " + m_currentStage, {255, 200, 100, 255});
        int32_t barW = static_cast<int32_t>(m_currentProgress * 400);
        m_drawList.DrawRect({4, 70, 400, 12}, {60, 60, 60, 255});
        m_drawList.DrawRect({4, 70, barW, 12}, {80, 180, 80, 255});
    } else if (m_lastReport.result == production::PackageResult::Success) {
        m_drawList.DrawText({4, 50, 400, 16}, "Build succeeded", {100, 255, 100, 255});
    } else if (!m_lastReport.errorMessage.empty()) {
        m_drawList.DrawText({4, 50, 490, 16}, "Error: " + m_lastReport.errorMessage, {255, 80, 80, 255});
    }
}

std::string GamePackagerPanel::SettingsSummary() const {
    std::ostringstream oss;
    oss << "Target: ";
    switch (m_settings.target) {
        case BuildTarget::Client: oss << "Client"; break;
        case BuildTarget::Server: oss << "Server"; break;
    }
    oss << ", Mode: ";
    switch (m_settings.mode) {
        case BuildMode::Debug:       oss << "Debug"; break;
        case BuildMode::Development: oss << "Development"; break;
        case BuildMode::Release:     oss << "Release"; break;
    }
    oss << ", Output: " << m_settings.outputPath;
    return oss.str();
}

production::PackageReport GamePackagerPanel::Build() {
    m_building = true;
    m_currentStage = "";
    m_currentProgress = 0.0f;

    // Convert panel settings to packager config
    production::PackageConfig config;
    config.target = (m_settings.target == BuildTarget::Client)
                    ? production::PackageTarget::Client
                    : production::PackageTarget::Server;

    switch (m_settings.mode) {
        case BuildMode::Debug:       config.profile = production::ProfileType::Debug; break;
        case BuildMode::Development: config.profile = production::ProfileType::Development; break;
        case BuildMode::Release:     config.profile = production::ProfileType::Release; break;
    }

    config.outputDir = m_settings.outputPath;
    config.sourceDir = m_settings.sourceDir;
    config.includeMods = m_settings.includeMods;
    config.stripEditorData = m_settings.stripEditorData;
    config.singleExecutable = m_settings.singleExe;

    // Wire up progress callback
    m_packager.SetProgressCallback(
        [this](const std::string& stage, float progress) {
            m_currentStage = stage;
            m_currentProgress = progress;
        });

    m_lastReport = m_packager.Package(config);
    m_building = false;
    return m_lastReport;
}

bool GamePackagerPanel::IsPackaging() const {
    auto s = m_status.currentStep;
    return s == PackageStep::Validate || s == PackageStep::CookAssets ||
           s == PackageStep::Compile  || s == PackageStep::Bundle;
}

void GamePackagerPanel::StartPackage() {
    if (m_settings.outputPath.empty()) {
        m_status.currentStep = PackageStep::Failed;
        m_status.hasErrors = true;
        m_status.log.push_back("Empty output path");
        return;
    }
    m_status.currentStep = PackageStep::Validate;
    m_status.hasErrors = false;
    m_status.log.push_back("Started packaging");
}

void GamePackagerPanel::AdvanceStep() {
    switch (m_status.currentStep) {
        case PackageStep::Validate:
            m_status.currentStep = PackageStep::CookAssets;
            m_status.log.push_back("Validation passed");
            break;
        case PackageStep::CookAssets:
            m_status.currentStep = PackageStep::Compile;
            m_status.log.push_back("Assets cooked");
            break;
        case PackageStep::Compile:
            m_status.currentStep = PackageStep::Bundle;
            m_status.log.push_back("Compilation finished");
            break;
        case PackageStep::Bundle:
            m_status.currentStep = PackageStep::Complete;
            m_status.log.push_back("Bundle created at " + m_settings.outputPath);
            break;
        default:
            break;
    }
}

void GamePackagerPanel::CancelPackage() {
    m_status.currentStep = PackageStep::Idle;
    m_status.log.push_back("Packaging cancelled");
}

std::string GamePackagerPanel::StepToString(PackageStep step) {
    switch (step) {
        case PackageStep::Idle:       return "Idle";
        case PackageStep::Validate:   return "Validate";
        case PackageStep::CookAssets: return "Cook Assets";
        case PackageStep::Compile:    return "Compile";
        case PackageStep::Bundle:     return "Bundle";
        case PackageStep::Complete:   return "Complete";
        case PackageStep::Failed:     return "Failed";
    }
    return "Unknown";
}

std::string GamePackagerPanel::TargetToString(BuildTarget target) {
    switch (target) {
        case BuildTarget::Client: return "Client";
        case BuildTarget::Server: return "Server";
    }
    return "Unknown";
}

std::string GamePackagerPanel::ModeToString(BuildMode mode) {
    switch (mode) {
        case BuildMode::Debug:       return "Debug";
        case BuildMode::Development: return "Development";
        case BuildMode::Release:     return "Release";
    }
    return "Unknown";
}

}
