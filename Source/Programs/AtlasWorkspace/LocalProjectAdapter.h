#pragma once
// LocalProjectAdapter — minimal IGameProjectAdapter for generic projects.
//
// Used when the user opens or creates a non-NovaForge project from the
// workspace welcome screen.  Records the project path and lets the shell
// host the registered tools and panels without project-specific logic.
#include "NF/Workspace/IGameProjectAdapter.h"
#include <string>
#include <vector>

class LocalProjectAdapter final : public NF::IGameProjectAdapter {
public:
    LocalProjectAdapter(std::string id, std::string displayName, std::string path)
        : m_id(std::move(id))
        , m_displayName(std::move(displayName))
        , m_path(std::move(path)) {}

    std::string projectId()          const override { return m_id;          }
    std::string projectDisplayName() const override { return m_displayName; }

    bool initialize() override { return true; }
    void shutdown()   override {}

    std::vector<NF::GameplaySystemPanelDescriptor> panelDescriptors() const override {
        return {};
    }
    std::vector<std::string> contentRoots() const override { return {m_path}; }

private:
    std::string m_id;
    std::string m_displayName;
    std::string m_path;
};
