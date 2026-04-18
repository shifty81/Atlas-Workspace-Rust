#include "HeadlessGUI.h"
#ifdef ATLAS_INCLUDE_TOOLS
#include "DiagnosticsOverlay.h"
#endif
#include <sstream>
#include <algorithm>

namespace atlas::ui {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static const char* WidgetTypeName(UIWidgetType type) {
    switch (type) {
        case UIWidgetType::Panel:      return "Panel";
        case UIWidgetType::Button:     return "Button";
        case UIWidgetType::Text:       return "Text";
        case UIWidgetType::Image:      return "Image";
        case UIWidgetType::List:       return "List";
        case UIWidgetType::SlotGrid:   return "SlotGrid";
        case UIWidgetType::InputField: return "InputField";
    }
    return "Unknown";
}

static const char* ContextName(GUIContext ctx) {
    switch (ctx) {
        case GUIContext::Editor: return "Editor";
        case GUIContext::Game:   return "Game";
        case GUIContext::Server: return "Server";
    }
    return "Unknown";
}

static UICommandType CommandTypeFromString(const std::string& s) {
    if (s == "ButtonPress")       return UICommandType::ButtonPress;
    if (s == "ButtonRelease")     return UICommandType::ButtonRelease;
    if (s == "TextInput")         return UICommandType::TextInput;
    if (s == "SliderChange")      return UICommandType::SliderChange;
    if (s == "ListSelect")        return UICommandType::ListSelect;
    if (s == "FocusChange")       return UICommandType::FocusChange;
    if (s == "VisibilityToggle")  return UICommandType::VisibilityToggle;
    if (s == "DockSplit")         return UICommandType::DockSplit;
    if (s == "DockMerge")         return UICommandType::DockMerge;
    if (s == "DockResize")        return UICommandType::DockResize;
    return UICommandType::Custom;
}

// Collect all widget IDs by walking the tree from root (parentId == 0)
static void CollectWidgetIds(const UIScreen& screen, uint32_t parentId,
                             std::vector<uint32_t>& out) {
    auto children = screen.GetChildren(parentId);
    for (uint32_t id : children) {
        out.push_back(id);
        CollectWidgetIds(screen, id, out);
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void HeadlessGUI::Init(UIManager* manager) {
    m_manager = manager;
    m_commands.clear();
    RegisterBuiltinCommands();
    m_initialized = true;
}

void HeadlessGUI::Shutdown() {
    m_commands.clear();
    m_manager = nullptr;
    m_initialized = false;
}

bool HeadlessGUI::IsInitialized() const {
    return m_initialized;
}

// ---------------------------------------------------------------------------
// Command registration & query
// ---------------------------------------------------------------------------

void HeadlessGUI::RegisterCommand(
        const std::string& name,
        std::function<GUIQueryResult(const std::vector<std::string>&)> handler) {
    m_commands[name] = std::move(handler);
}

size_t HeadlessGUI::CommandCount() const {
    return m_commands.size();
}

std::vector<std::string> HeadlessGUI::AvailableCommands() const {
    std::vector<std::string> names;
    names.reserve(m_commands.size());
    for (const auto& [name, _] : m_commands) {
        names.push_back(name);
    }
    return names;
}

// ---------------------------------------------------------------------------
// Execution
// ---------------------------------------------------------------------------

GUIQueryResult HeadlessGUI::ExecuteCommand(const std::string& command) {
    if (!m_initialized || !m_manager) {
        return {false, "HeadlessGUI not initialized"};
    }

    auto tokens = TokenizeCommand(command);
    if (tokens.empty()) {
        return {false, command.empty() ? "Empty command" : "Unterminated quoted string"};
    }

    const std::string& name = tokens[0];
    auto it = m_commands.find(name);
    if (it == m_commands.end()) {
        return {false, "Unknown command: " + name};
    }

    std::vector<std::string> args(tokens.begin() + 1, tokens.end());
    return it->second(args);
}

void HeadlessGUI::Update(uint64_t /*tick*/) {
    if (!m_initialized || !m_manager) return;
    m_manager->GetCommandBus().Dispatch();
}

// ---------------------------------------------------------------------------
// Tokenizer – splits on whitespace, supports "quoted strings"
// ---------------------------------------------------------------------------

std::vector<std::string> HeadlessGUI::TokenizeCommand(const std::string& command) {
    std::vector<std::string> tokens;
    std::string current;
    bool inQuotes = false;

    for (size_t i = 0; i < command.size(); ++i) {
        char c = command[i];
        if (c == '"') {
            inQuotes = !inQuotes;
            continue;
        }
        if (c == ' ' && !inQuotes) {
            if (!current.empty()) {
                tokens.push_back(std::move(current));
                current.clear();
            }
            continue;
        }
        current += c;
    }
    if (!current.empty()) {
        tokens.push_back(std::move(current));
    }
    if (inQuotes) {
        tokens.clear();
    }
    return tokens;
}

// ---------------------------------------------------------------------------
// Built-in command registration
// ---------------------------------------------------------------------------

void HeadlessGUI::RegisterBuiltinCommands() {
    RegisterCommand("widget.list", [this](const std::vector<std::string>& args) {
        return CmdWidgetList(args);
    });
    RegisterCommand("widget.info", [this](const std::vector<std::string>& args) {
        return CmdWidgetInfo(args);
    });
    RegisterCommand("widget.count", [this](const std::vector<std::string>& args) {
        return CmdWidgetCount(args);
    });
    RegisterCommand("enqueue", [this](const std::vector<std::string>& args) {
        return CmdEnqueue(args);
    });
    RegisterCommand("help", [this](const std::vector<std::string>& args) {
        return CmdHelp(args);
    });
    RegisterCommand("status", [this](const std::vector<std::string>& args) {
        return CmdStatus(args);
    });
    RegisterCommand("diag.toggle", [this](const std::vector<std::string>& args) {
        return CmdDiagToggle(args);
    });
    RegisterCommand("diag.show", [this](const std::vector<std::string>& args) {
        return CmdDiagShow(args);
    });
    RegisterCommand("diag.hide", [this](const std::vector<std::string>& args) {
        return CmdDiagHide(args);
    });
    RegisterCommand("diag.status", [this](const std::vector<std::string>& args) {
        return CmdDiagStatus(args);
    });
}

// ---------------------------------------------------------------------------
// Built-in command implementations
// ---------------------------------------------------------------------------

GUIQueryResult HeadlessGUI::CmdWidgetList(const std::vector<std::string>& /*args*/) {
    const UIScreen& screen = m_manager->GetScreen();
    std::vector<uint32_t> ids;
    CollectWidgetIds(screen, 0, ids);
    std::sort(ids.begin(), ids.end());

    std::ostringstream out;
    out << "ID\tType\tName\tVisible\n";
    for (uint32_t id : ids) {
        const UIWidget* w = screen.GetWidget(id);
        if (!w) continue;
        out << w->id << '\t'
            << WidgetTypeName(w->type) << '\t'
            << w->name << '\t'
            << (w->visible ? "true" : "false") << '\n';
    }
    return {true, out.str()};
}

GUIQueryResult HeadlessGUI::CmdWidgetInfo(const std::vector<std::string>& args) {
    if (args.empty()) {
        return {false, "Usage: widget.info <id>"};
    }

    uint32_t id = 0;
    try { id = static_cast<uint32_t>(std::stoul(args[0])); }
    catch (...) { return {false, "Invalid widget ID"}; }

    const UIWidget* w = m_manager->GetScreen().GetWidget(id);
    if (!w) {
        return {false, "Widget not found: " + std::to_string(id)};
    }

    std::ostringstream out;
    out << "id:       " << w->id << '\n'
        << "type:     " << WidgetTypeName(w->type) << '\n'
        << "name:     " << w->name << '\n'
        << "visible:  " << (w->visible ? "true" : "false") << '\n'
        << "parent:   " << w->parentId << '\n'
        << "position: " << w->x << ", " << w->y << '\n'
        << "size:     " << w->width << " x " << w->height << '\n';
    return {true, out.str()};
}

GUIQueryResult HeadlessGUI::CmdWidgetCount(const std::vector<std::string>& /*args*/) {
    size_t count = m_manager->GetScreen().WidgetCount();
    return {true, std::to_string(count)};
}

GUIQueryResult HeadlessGUI::CmdEnqueue(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        return {false, "Usage: enqueue <TYPE> <WIDGET_ID>"};
    }

    UICommandType type = CommandTypeFromString(args[0]);

    uint32_t widgetId = 0;
    try { widgetId = static_cast<uint32_t>(std::stoul(args[1])); }
    catch (...) { return {false, "Invalid widget ID"}; }

    UICommand cmd;
    cmd.type = type;
    cmd.targetWidgetId = widgetId;
    m_manager->GetCommandBus().Enqueue(std::move(cmd));

    return {true, "Enqueued " + args[0] + " for widget " + std::to_string(widgetId)};
}

GUIQueryResult HeadlessGUI::CmdHelp(const std::vector<std::string>& /*args*/) {
    std::ostringstream out;
    out << "Available commands:\n";
    for (const auto& [name, _] : m_commands) {
        out << "  " << name << '\n';
    }
    return {true, out.str()};
}

GUIQueryResult HeadlessGUI::CmdStatus(const std::vector<std::string>& /*args*/) {
    std::ostringstream out;
    out << "context:  " << ContextName(m_manager->GetContext()) << '\n'
        << "widgets:  " << m_manager->GetScreen().WidgetCount() << '\n'
        << "pending:  " << m_manager->GetCommandBus().PendingCount() << '\n';
    return {true, out.str()};
}

GUIQueryResult HeadlessGUI::CmdDiagToggle(const std::vector<std::string>& /*args*/) {
#ifdef ATLAS_INCLUDE_TOOLS
    DiagnosticsOverlay::Toggle();
    return {true, DiagnosticsOverlay::IsEnabled() ? "on" : "off"};
#else
    return {false, "diagnostics not available (tools stripped)"};
#endif
}

GUIQueryResult HeadlessGUI::CmdDiagShow(const std::vector<std::string>& /*args*/) {
#ifdef ATLAS_INCLUDE_TOOLS
    DiagnosticsOverlay::SetEnabled(true);
    return {true, "on"};
#else
    return {false, "diagnostics not available (tools stripped)"};
#endif
}

GUIQueryResult HeadlessGUI::CmdDiagHide(const std::vector<std::string>& /*args*/) {
#ifdef ATLAS_INCLUDE_TOOLS
    DiagnosticsOverlay::SetEnabled(false);
    return {true, "off"};
#else
    return {false, "diagnostics not available (tools stripped)"};
#endif
}

GUIQueryResult HeadlessGUI::CmdDiagStatus(const std::vector<std::string>& /*args*/) {
#ifdef ATLAS_INCLUDE_TOOLS
    return {true, DiagnosticsOverlay::IsEnabled() ? "on" : "off"};
#else
    return {false, "diagnostics not available (tools stripped)"};
#endif
}

} // namespace atlas::ui
