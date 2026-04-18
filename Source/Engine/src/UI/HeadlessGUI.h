#pragma once
#include "UIManager.h"
#include <string>
#include <vector>
#include <functional>
#include <map>

namespace atlas::ui {

struct GUIQueryResult {
    bool success = false;
    std::string output;
};

class HeadlessGUI {
public:
    void Init(UIManager* manager);
    void Shutdown();

    GUIQueryResult ExecuteCommand(const std::string& command);

    void RegisterCommand(const std::string& name, std::function<GUIQueryResult(const std::vector<std::string>&)> handler);

    size_t CommandCount() const;
    std::vector<std::string> AvailableCommands() const;

    bool IsInitialized() const;

    void Update(uint64_t tick);

private:
    UIManager* m_manager = nullptr;
    bool m_initialized = false;
    std::map<std::string, std::function<GUIQueryResult(const std::vector<std::string>&)>> m_commands;

    static std::vector<std::string> TokenizeCommand(const std::string& command);

    // Built-in commands
    GUIQueryResult CmdWidgetList(const std::vector<std::string>& args);
    GUIQueryResult CmdWidgetInfo(const std::vector<std::string>& args);
    GUIQueryResult CmdWidgetCount(const std::vector<std::string>& args);
    GUIQueryResult CmdEnqueue(const std::vector<std::string>& args);
    GUIQueryResult CmdHelp(const std::vector<std::string>& args);
    GUIQueryResult CmdStatus(const std::vector<std::string>& args);
    GUIQueryResult CmdDiagToggle(const std::vector<std::string>& args);
    GUIQueryResult CmdDiagShow(const std::vector<std::string>& args);
    GUIQueryResult CmdDiagHide(const std::vector<std::string>& args);
    GUIQueryResult CmdDiagStatus(const std::vector<std::string>& args);

    void RegisterBuiltinCommands();
};

} // namespace atlas::ui
