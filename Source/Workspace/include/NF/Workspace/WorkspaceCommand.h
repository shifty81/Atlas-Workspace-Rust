#pragma once
// NF::Workspace — WorkspaceCommand: typed command descriptor.
//
// A WorkspaceCommand is the canonical unit of user-facing workspace action.
// It carries everything needed to display, invoke, and persist a command:
//   - Unique string id (e.g. "file.save", "edit.undo")
//   - Human-readable label and optional tooltip
//   - Category (for grouping in menus/toolbars)
//   - Keyboard shortcut (as an opaque string — platform interpretation is caller's)
//   - Runtime state (enabled, visible, checked)
//   - Execute handler (std::function<bool()> — returns true on success)
//   - Undo handler (optional; if provided the command is reversible)
//
// CommandState bundles the mutable display/interaction flags so they can be
// copied atomically and diffed for UI refresh.
//
// CommandCategory groups commands into named buckets (File, Edit, View, …).

#include <cstdint>
#include <functional>
#include <string>

namespace NF {

// ── Command Category ───────────────────────────────────────────────

enum class CommandCategory : uint8_t {
    File,
    Edit,
    View,
    Selection,
    Tools,
    Window,
    Help,
    Custom,
};

inline const char* commandCategoryName(CommandCategory c) {
    switch (c) {
    case CommandCategory::File:      return "File";
    case CommandCategory::Edit:      return "Edit";
    case CommandCategory::View:      return "View";
    case CommandCategory::Selection: return "Selection";
    case CommandCategory::Tools:     return "Tools";
    case CommandCategory::Window:    return "Window";
    case CommandCategory::Help:      return "Help";
    default:                         return "Custom";
    }
}

// ── Command State ──────────────────────────────────────────────────
// Mutable display/interaction flags — copied atomically for UI refresh.

struct CommandState {
    bool enabled = true;
    bool visible = true;
    bool checked = false;  // for toggle commands

    [[nodiscard]] bool operator==(const CommandState& o) const {
        return enabled == o.enabled && visible == o.visible && checked == o.checked;
    }
    [[nodiscard]] bool operator!=(const CommandState& o) const { return !(*this == o); }
};

// ── Handler types ──────────────────────────────────────────────────

using CommandHandler     = std::function<bool()>;  // returns true = success
using CommandUndoHandler = std::function<bool()>;  // returns true = success

// ── Workspace Command ──────────────────────────────────────────────

class WorkspaceCommand {
public:
    // ── Construction ──────────────────────────────────────────────

    WorkspaceCommand() = default;

    WorkspaceCommand(std::string id,
                     std::string label,
                     CommandCategory category = CommandCategory::Custom)
        : m_id(std::move(id))
        , m_label(std::move(label))
        , m_category(category)
    {}

    // ── Identity ──────────────────────────────────────────────────

    [[nodiscard]] const std::string& id()          const { return m_id; }
    [[nodiscard]] const std::string& label()       const { return m_label; }
    [[nodiscard]] const std::string& tooltip()     const { return m_tooltip; }
    [[nodiscard]] const std::string& shortcut()    const { return m_shortcut; }
    [[nodiscard]] CommandCategory    category()    const { return m_category; }
    [[nodiscard]] const std::string& iconKey()     const { return m_iconKey; }

    void setLabel(const std::string& l)       { m_label    = l; }
    void setTooltip(const std::string& t)     { m_tooltip  = t; }
    void setShortcut(const std::string& s)    { m_shortcut = s; }
    void setCategory(CommandCategory c)       { m_category = c; }
    void setIconKey(const std::string& k)     { m_iconKey  = k; }

    // ── State ─────────────────────────────────────────────────────

    [[nodiscard]] const CommandState& state()   const { return m_state; }
    [[nodiscard]] bool isEnabled()              const { return m_state.enabled; }
    [[nodiscard]] bool isVisible()              const { return m_state.visible; }
    [[nodiscard]] bool isChecked()              const { return m_state.checked; }

    void setEnabled(bool v) { m_state.enabled = v; }
    void setVisible(bool v) { m_state.visible = v; }
    void setChecked(bool v) { m_state.checked = v; }
    void setState(const CommandState& s) { m_state = s; }

    // ── Reversibility ─────────────────────────────────────────────

    [[nodiscard]] bool isReversible() const { return static_cast<bool>(m_undoHandler); }

    // ── Handlers ──────────────────────────────────────────────────

    void setHandler(CommandHandler h)     { m_handler     = std::move(h); }
    void setUndoHandler(CommandUndoHandler h) { m_undoHandler = std::move(h); }

    [[nodiscard]] bool hasHandler()     const { return static_cast<bool>(m_handler); }
    [[nodiscard]] bool hasUndoHandler() const { return static_cast<bool>(m_undoHandler); }

    // Execute the command. Returns false if disabled or handler is absent.
    bool execute() const {
        if (!m_state.enabled || !m_handler) return false;
        return m_handler();
    }

    // Execute the undo handler. Returns false if not reversible.
    bool undo() const {
        if (!m_undoHandler) return false;
        return m_undoHandler();
    }

    // ── Validity ──────────────────────────────────────────────────

    [[nodiscard]] bool isValid() const { return !m_id.empty() && !m_label.empty(); }

private:
    std::string     m_id;
    std::string     m_label;
    std::string     m_tooltip;
    std::string     m_shortcut;
    std::string     m_iconKey;
    CommandCategory m_category = CommandCategory::Custom;
    CommandState    m_state;
    CommandHandler     m_handler;
    CommandUndoHandler m_undoHandler;
};

} // namespace NF
