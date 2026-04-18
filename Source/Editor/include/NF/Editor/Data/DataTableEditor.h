#pragma once
// NF::Editor — Data table editor for game data
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

enum class DataTableColumnType : uint8_t {
    Bool, Int, Float, String, Enum, Asset, Curve
};

inline const char* dataTableColumnTypeName(DataTableColumnType t) {
    switch (t) {
        case DataTableColumnType::Bool:   return "Bool";
        case DataTableColumnType::Int:    return "Int";
        case DataTableColumnType::Float:  return "Float";
        case DataTableColumnType::String: return "String";
        case DataTableColumnType::Enum:   return "Enum";
        case DataTableColumnType::Asset:  return "Asset";
        case DataTableColumnType::Curve:  return "Curve";
    }
    return "Unknown";
}

enum class DataTableImportFormat : uint8_t {
    CSV, JSON, XML, Binary, GoogleSheets
};

inline const char* dataTableImportFormatName(DataTableImportFormat f) {
    switch (f) {
        case DataTableImportFormat::CSV:          return "CSV";
        case DataTableImportFormat::JSON:         return "JSON";
        case DataTableImportFormat::XML:          return "XML";
        case DataTableImportFormat::Binary:       return "Binary";
        case DataTableImportFormat::GoogleSheets: return "GoogleSheets";
    }
    return "Unknown";
}

enum class DataTableEditState : uint8_t {
    Clean, Modified, Importing, Exporting, Error
};

inline const char* dataTableEditStateName(DataTableEditState s) {
    switch (s) {
        case DataTableEditState::Clean:     return "Clean";
        case DataTableEditState::Modified:  return "Modified";
        case DataTableEditState::Importing: return "Importing";
        case DataTableEditState::Exporting: return "Exporting";
        case DataTableEditState::Error:     return "Error";
    }
    return "Unknown";
}

class DataTableColumn {
public:
    explicit DataTableColumn(const std::string& name, DataTableColumnType type)
        : m_name(name), m_type(type) {}

    void setRequired(bool v)      { m_required  = v; }
    void setDefaultValue(const std::string& val) { m_defaultValue = val; }
    void setVisible(bool v)       { m_visible   = v; }
    void setWidth(uint16_t w)     { m_width     = w; }

    [[nodiscard]] const std::string& name()         const { return m_name;         }
    [[nodiscard]] DataTableColumnType type()        const { return m_type;         }
    [[nodiscard]] bool               isRequired()   const { return m_required;     }
    [[nodiscard]] const std::string& defaultValue() const { return m_defaultValue; }
    [[nodiscard]] bool               isVisible()    const { return m_visible;      }
    [[nodiscard]] uint16_t           width()        const { return m_width;        }

private:
    std::string          m_name;
    DataTableColumnType  m_type;
    std::string          m_defaultValue;
    uint16_t             m_width    = 100;
    bool                 m_required = false;
    bool                 m_visible  = true;
};

class DataTableEditor {
public:
    static constexpr size_t MAX_COLUMNS = 64;
    static constexpr size_t MAX_ROWS    = 65536;

    [[nodiscard]] bool addColumn(const DataTableColumn& col) {
        for (auto& c : m_columns) if (c.name() == col.name()) return false;
        if (m_columns.size() >= MAX_COLUMNS) return false;
        m_columns.push_back(col);
        return true;
    }

    [[nodiscard]] bool removeColumn(const std::string& name) {
        for (auto it = m_columns.begin(); it != m_columns.end(); ++it) {
            if (it->name() == name) { m_columns.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] DataTableColumn* findColumn(const std::string& name) {
        for (auto& c : m_columns) if (c.name() == name) return &c;
        return nullptr;
    }

    void setRowCount(size_t n)                { m_rowCount  = n; }
    void setState(DataTableEditState s)       { m_state     = s; }
    void setImportFormat(DataTableImportFormat f){ m_importFormat = f; }
    void setTableName(const std::string& n)   { m_tableName = n; }

    [[nodiscard]] const std::string&  tableName()    const { return m_tableName;    }
    [[nodiscard]] DataTableEditState  state()        const { return m_state;        }
    [[nodiscard]] DataTableImportFormat importFormat() const { return m_importFormat; }
    [[nodiscard]] size_t              columnCount()  const { return m_columns.size(); }
    [[nodiscard]] size_t              rowCount()     const { return m_rowCount;     }

    [[nodiscard]] bool isModified()   const { return m_state == DataTableEditState::Modified; }
    [[nodiscard]] size_t visibleColumnCount() const {
        size_t c = 0; for (auto& col : m_columns) if (col.isVisible()) ++c; return c;
    }
    [[nodiscard]] size_t requiredColumnCount() const {
        size_t c = 0; for (auto& col : m_columns) if (col.isRequired()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(DataTableColumnType t) const {
        size_t c = 0; for (auto& col : m_columns) if (col.type() == t) ++c; return c;
    }

private:
    std::vector<DataTableColumn> m_columns;
    std::string                  m_tableName;
    DataTableEditState           m_state        = DataTableEditState::Clean;
    DataTableImportFormat        m_importFormat = DataTableImportFormat::CSV;
    size_t                       m_rowCount     = 0;
};

} // namespace NF
