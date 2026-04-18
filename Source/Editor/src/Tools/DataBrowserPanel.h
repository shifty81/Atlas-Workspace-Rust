#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>

namespace atlas::editor {

/**
 * @brief Represents one key–value field in a JSON data entry.
 *
 * Values are stored as strings; the @c fieldType hint tells the UI
 * whether to render a text box, a number spinner, or a checkbox.
 */
struct DataField {
    std::string key;
    std::string value;
    enum class Type { String, Number, Boolean } fieldType = Type::String;
};

/**
 * @brief One record inside a loaded data file (e.g. one entity, one asset).
 *
 * The @c id doubles as a display label in the list view.  Fields hold the
 * flat key–value pairs extracted from the JSON object.
 */
struct DataEntry {
    std::string id;
    std::vector<DataField> fields;
};

/**
 * @brief DataBrowserPanel — generic JSON data editor.
 *
 * Provides a table-style browser for any JSON data files loaded from
 * project data directories.  Categories are discovered dynamically
 * from the loaded project rather than hardcoded.
 *
 * Usage workflow:
 *   1. LoadCategory() to open a data file or set of files
 *   2. Browse / filter entries in the list view
 *   3. Select an entry to see its fields
 *   4. Edit field values
 *   5. ExportToJson() to serialise back
 *
 * The panel uses the same hand-written JSON approach as the rest of
 * the editor (no external JSON library).
 */
class DataBrowserPanel : public EditorPanel {
public:
    DataBrowserPanel();
    ~DataBrowserPanel() override = default;

    const char* Name() const override { return "Data Browser"; }
    void Draw() override;

    // ── Category / file management ───────────────────────────────

    /** Load all entries for a given category (subdirectory of data/). */
    void LoadCategory(const std::string& category);

    /** Register a data category for the browser to discover.
     *  Projects call this at startup to populate the category list. */
    void RegisterCategory(const std::string& category) {
        m_registeredCategories.push_back(category);
    }

    /** Get the list of registered categories. */
    const std::vector<std::string>& RegisteredCategories() const {
        return m_registeredCategories;
    }

    /** Get current category name. */
    const std::string& CurrentCategory() const { return m_currentCategory; }

    // ── Entry management ─────────────────────────────────────────

    size_t AddEntry(const DataEntry& entry);
    bool   RemoveEntry(size_t index);
    bool   UpdateEntryField(size_t entryIndex, size_t fieldIndex,
                            const std::string& newValue);
    size_t EntryCount() const { return m_entries.size(); }

    const DataEntry& GetEntry(size_t index) const { return m_entries[index]; }
    const std::vector<DataEntry>& Entries() const { return m_entries; }

    // ── Selection ────────────────────────────────────────────────

    void SelectEntry(int index);
    void ClearSelection();
    int  SelectedEntry() const { return m_selectedIndex; }

    // ── Filtering ────────────────────────────────────────────────

    void SetFilter(const std::string& filter);
    const std::string& Filter() const { return m_filter; }
    size_t FilteredCount() const;

    // ── Export ────────────────────────────────────────────────────

    /** Serialise current entries to a JSON string. */
    std::string ExportToJson() const;

    /** Import entries from a JSON string (lightweight parser). */
    size_t ImportFromJson(const std::string& json);

    // ── Log ──────────────────────────────────────────────────────

    const std::vector<std::string>& Log() const { return m_log; }

private:
    std::string m_currentCategory;
    std::vector<DataEntry> m_entries;
    int m_selectedIndex = -1;
    std::string m_filter;
    int m_categoryIndex = 0;

    std::vector<std::string> m_registeredCategories;
    std::vector<std::string> m_log;
    atlas::PanelState m_panelState;
    float m_scrollOffset = 0.0f;

    void log(const std::string& msg);
    bool matchesFilter(const DataEntry& entry) const;
};

} // namespace atlas::editor
