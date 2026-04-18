#pragma once
// NF::Editor — DataTableDocument: schema-driven data table document model.
//
// Phase G.8 — Data Editor full tool wiring.
//
// A DataTableDocument is the authoring model for a single data table asset:
//   - Column schema (name, type, default, enum options)
//   - Row table (cells indexed by column id)
//   - Per-field validation (type check + callback)
//   - Dirty tracking, save/load, CSV import/export stubs
//
// DataEditorTool owns one DataTableDocument at a time.

#include "NF/Core/Core.h"
#include <cstdint>
#include <fstream>
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace NF {

// ── Identifiers ────────────────────────────────────────────────────────────────

using DataColumnId = uint32_t;
using DataRowId    = uint32_t;

static constexpr DataColumnId kInvalidDataColumnId = 0u;
static constexpr DataRowId    kInvalidDataRowId    = 0u;

// ── Column type ────────────────────────────────────────────────────────────────

enum class DataColumnType : uint8_t {
    String,
    Int,
    Float,
    Bool,
    Enum,     ///< value must be one of enumOptions
};

inline const char* dataColumnTypeName(DataColumnType t) {
    switch (t) {
    case DataColumnType::String: return "String";
    case DataColumnType::Int:    return "Int";
    case DataColumnType::Float:  return "Float";
    case DataColumnType::Bool:   return "Bool";
    case DataColumnType::Enum:   return "Enum";
    }
    return "Unknown";
}

// ── DataColumn ─────────────────────────────────────────────────────────────────

struct DataColumn {
    DataColumnId             id           = kInvalidDataColumnId;
    std::string              name;
    DataColumnType           type         = DataColumnType::String;
    std::string              defaultValue;
    std::vector<std::string> enumOptions; ///< valid options when type == Enum
    bool                     required     = false;
};

// ── Validation result ──────────────────────────────────────────────────────────

enum class DataCellValidation : uint8_t { Ok, TypeMismatch, InvalidEnum, Empty };

inline const char* dataCellValidationName(DataCellValidation v) {
    switch (v) {
    case DataCellValidation::Ok:          return "Ok";
    case DataCellValidation::TypeMismatch: return "TypeMismatch";
    case DataCellValidation::InvalidEnum: return "InvalidEnum";
    case DataCellValidation::Empty:       return "Empty";
    }
    return "Unknown";
}

struct DataCellValidationResult {
    DataCellValidation status  = DataCellValidation::Ok;
    std::string        message;
    [[nodiscard]] bool ok() const { return status == DataCellValidation::Ok; }
};

// ── DataRow ────────────────────────────────────────────────────────────────────

struct DataRow {
    DataRowId                            id = kInvalidDataRowId;
    std::map<DataColumnId, std::string>  cells; ///< columnId → string value
};

// ── DataTableDocument ──────────────────────────────────────────────────────────

class DataTableDocument {
public:
    DataTableDocument() = default;
    explicit DataTableDocument(const std::string& tableName) : m_tableName(tableName) {}

    // ── Identity ───────────────────────────────────────────────────────────────

    [[nodiscard]] const std::string& tableName()  const { return m_tableName; }
    [[nodiscard]] const std::string& assetPath()  const { return m_assetPath; }
    void setTableName(const std::string& n) { m_tableName = n; markDirty(); }
    void setAssetPath(const std::string& p) { m_assetPath = p; }

    // ── Dirty tracking ─────────────────────────────────────────────────────────

    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void markDirty()  { m_dirty = true; }
    void clearDirty() { m_dirty = false; }

    // ── Column management ──────────────────────────────────────────────────────

    DataColumnId addColumn(const std::string& name, DataColumnType type,
                            const std::string& defaultValue = "") {
        DataColumnId id = ++m_nextColumnId;
        DataColumn col;
        col.id           = id;
        col.name         = name;
        col.type         = type;
        col.defaultValue = defaultValue;

        // Pre-populate existing rows with default value
        for (auto& row : m_rows) {
            row.cells[id] = defaultValue;
        }

        m_columns.push_back(std::move(col));
        markDirty();
        return id;
    }

    bool removeColumn(DataColumnId id) {
        for (auto it = m_columns.begin(); it != m_columns.end(); ++it) {
            if (it->id == id) {
                // Remove cell data from all rows
                for (auto& row : m_rows) row.cells.erase(id);
                m_columns.erase(it);
                markDirty();
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] uint32_t columnCount() const {
        return static_cast<uint32_t>(m_columns.size());
    }

    [[nodiscard]] const DataColumn* findColumn(DataColumnId id) const {
        for (const auto& c : m_columns) if (c.id == id) return &c;
        return nullptr;
    }

    [[nodiscard]] DataColumn* findColumn(DataColumnId id) {
        for (auto& c : m_columns) if (c.id == id) return &c;
        return nullptr;
    }

    [[nodiscard]] DataColumnId findColumnByName(const std::string& name) const {
        for (const auto& c : m_columns) if (c.name == name) return c.id;
        return kInvalidDataColumnId;
    }

    bool addEnumOption(DataColumnId colId, const std::string& option) {
        auto* col = findColumn(colId);
        if (!col || col->type != DataColumnType::Enum) return false;
        col->enumOptions.push_back(option);
        markDirty();
        return true;
    }

    [[nodiscard]] const std::vector<DataColumn>& columns() const { return m_columns; }

    // ── Row management ─────────────────────────────────────────────────────────

    DataRowId addRow() {
        DataRowId id = ++m_nextRowId;
        DataRow row;
        row.id = id;

        // Initialize cells with column defaults
        for (const auto& col : m_columns) {
            row.cells[col.id] = col.defaultValue;
        }

        m_rows.push_back(std::move(row));
        markDirty();
        return id;
    }

    bool removeRow(DataRowId id) {
        for (auto it = m_rows.begin(); it != m_rows.end(); ++it) {
            if (it->id == id) {
                m_rows.erase(it);
                markDirty();
                return true;
            }
        }
        return false;
    }

    DataRowId duplicateRow(DataRowId sourceId) {
        const auto* src = findRow(sourceId);
        if (!src) return kInvalidDataRowId;

        DataRowId id = ++m_nextRowId;
        DataRow copy = *src;
        copy.id = id;
        m_rows.push_back(std::move(copy));
        markDirty();
        return id;
    }

    [[nodiscard]] uint32_t rowCount() const {
        return static_cast<uint32_t>(m_rows.size());
    }

    [[nodiscard]] const DataRow* findRow(DataRowId id) const {
        for (const auto& r : m_rows) if (r.id == id) return &r;
        return nullptr;
    }

    [[nodiscard]] DataRow* findRow(DataRowId id) {
        for (auto& r : m_rows) if (r.id == id) return &r;
        return nullptr;
    }

    [[nodiscard]] const std::vector<DataRow>& rows() const { return m_rows; }

    // ── Cell access ────────────────────────────────────────────────────────────

    bool setCell(DataRowId rowId, DataColumnId colId, const std::string& value) {
        auto* row = findRow(rowId);
        if (!row) return false;
        if (!findColumn(colId)) return false;
        row->cells[colId] = value;
        markDirty();
        return true;
    }

    [[nodiscard]] std::string getCell(DataRowId rowId, DataColumnId colId,
                                       const std::string& defaultVal = "") const {
        if (const auto* row = findRow(rowId)) {
            auto it = row->cells.find(colId);
            if (it != row->cells.end()) return it->second;
        }
        return defaultVal;
    }

    // ── Validation ─────────────────────────────────────────────────────────────

    [[nodiscard]] DataCellValidationResult validateCell(DataRowId rowId,
                                                         DataColumnId colId) const {
        const auto* col = findColumn(colId);
        if (!col) return { DataCellValidation::TypeMismatch, "Unknown column" };

        std::string value = getCell(rowId, colId);

        if (col->required && value.empty()) {
            return { DataCellValidation::Empty, col->name + " is required" };
        }

        if (col->type == DataColumnType::Enum && !value.empty()) {
            bool found = false;
            for (const auto& opt : col->enumOptions) {
                if (opt == value) { found = true; break; }
            }
            if (!found) {
                return { DataCellValidation::InvalidEnum,
                         "'" + value + "' is not a valid option for " + col->name };
            }
        }

        return { DataCellValidation::Ok, {} };
    }

    [[nodiscard]] uint32_t validateAll() const {
        uint32_t errorCount = 0;
        for (const auto& row : m_rows) {
            for (const auto& col : m_columns) {
                if (!validateCell(row.id, col.id).ok()) ++errorCount;
            }
        }
        return errorCount;
    }

    // ── CSV import/export ──────────────────────────────────────────────────────

    [[nodiscard]] std::string exportCsv() const {
        std::string out;
        // Header row
        for (size_t i = 0; i < m_columns.size(); ++i) {
            if (i > 0) out += ",";
            out += m_columns[i].name;
        }
        out += "\n";
        // Data rows
        for (const auto& row : m_rows) {
            for (size_t i = 0; i < m_columns.size(); ++i) {
                if (i > 0) out += ",";
                auto it = row.cells.find(m_columns[i].id);
                if (it != row.cells.end()) {
                    // Wrap in quotes if the value contains a comma or quote.
                    const std::string& v = it->second;
                    if (v.find(',') != std::string::npos ||
                        v.find('"') != std::string::npos) {
                        out += '"';
                        for (char c : v) {
                            if (c == '"') out += '"'; // escape embedded quotes
                            out += c;
                        }
                        out += '"';
                    } else {
                        out += v;
                    }
                }
            }
            out += "\n";
        }
        return out;
    }

    /// Parse a CSV string and populate rows/cells.
    /// The first row is treated as a header that defines (or maps to) columns.
    /// If a header name matches an existing column, cells are mapped to it;
    /// otherwise a new String column is created.
    bool importCsv(const std::string& csv) {
        if (csv.empty()) return false;

        std::vector<std::string> csvLines;
        {
            std::string line;
            for (char c : csv) {
                if (c == '\n') {
                    csvLines.push_back(line);
                    line.clear();
                } else if (c != '\r') {
                    line += c;
                }
            }
            if (!line.empty()) csvLines.push_back(line);
        }

        if (csvLines.empty()) return false;

        // Parse a single CSV field: handles quoted fields and embedded commas.
        auto parseRow = [](const std::string& rowStr) -> std::vector<std::string> {
            std::vector<std::string> fields;
            std::string field;
            bool inQuotes = false;
            for (size_t k = 0; k < rowStr.size(); ++k) {
                char c = rowStr[k];
                if (inQuotes) {
                    if (c == '"') {
                        if (k + 1 < rowStr.size() && rowStr[k + 1] == '"') {
                            field += '"'; ++k; // escaped quote
                        } else {
                            inQuotes = false;
                        }
                    } else {
                        field += c;
                    }
                } else {
                    if (c == '"') {
                        inQuotes = true;
                    } else if (c == ',') {
                        fields.push_back(field);
                        field.clear();
                    } else {
                        field += c;
                    }
                }
            }
            fields.push_back(field);
            return fields;
        };

        // Header row → column mapping
        auto headers = parseRow(csvLines[0]);
        std::vector<DataColumnId> colIds;
        colIds.reserve(headers.size());
        for (const auto& h : headers) {
            DataColumnId id = findColumnByName(h);
            if (id == kInvalidDataColumnId) {
                // Create a new String column for this CSV header.
                id = addColumn(h, DataColumnType::String);
            }
            colIds.push_back(id);
        }

        // Data rows
        for (size_t r = 1; r < csvLines.size(); ++r) {
            if (csvLines[r].empty()) continue;
            auto fields = parseRow(csvLines[r]);
            DataRowId rowId = addRow();
            for (size_t f = 0; f < fields.size() && f < colIds.size(); ++f) {
                setCell(rowId, colIds[f], fields[f]);
            }
        }

        markDirty();
        return true;
    }

    // ── Save / load ────────────────────────────────────────────────────────────

    /// Mark the table as persisted (clears dirty flag).
    /// Returns false if no path is set.
    /// Note: this is a stub save — no actual file I/O is performed.
    bool save(const std::string& path = "") {
        if (!path.empty()) m_assetPath = path;
        if (m_assetPath.empty()) return false;
        clearDirty();
        return true;
    }

    /// Deserialize the table from a JSON string previously produced by serialize().
    /// This is a minimal reader: it restores column schema and row data.
    bool load(const std::string& json) {
        if (json.empty()) return false;

        // Helper: find the value of a JSON string key.
        // E.g. for '"name":"Foo"' findJsonString("name") returns "Foo".
        auto findJsonString = [&](const std::string& key) -> std::string {
            std::string needle = "\"" + key + "\":\"";
            auto pos = json.find(needle);
            if (pos == std::string::npos) return {};
            pos += needle.size();
            std::string val;
            for (; pos < json.size() && json[pos] != '"'; ++pos) {
                if (json[pos] == '\\' && pos + 1 < json.size()) {
                    ++pos; val += json[pos];
                } else {
                    val += json[pos];
                }
            }
            return val;
        };

        // Restore table name.
        std::string name = findJsonString("table");
        if (!name.empty()) m_tableName = name;

        // Restore columns from "columns" JSON array.
        // Format: "columns":[{"id":1,"name":"Col","type":"String","default":""},...]
        {
            auto pos = json.find("\"columns\":[");
            if (pos != std::string::npos) {
                pos += 11;
                size_t end = json.find(']', pos);
                if (end != std::string::npos) {
                    m_columns.clear();
                    m_nextColumnId = 0;
                    // Parse each {...} object in the array.
                    while (pos < end) {
                        auto objStart = json.find('{', pos);
                        if (objStart == std::string::npos || objStart >= end) break;
                        auto objEnd = json.find('}', objStart);
                        if (objEnd == std::string::npos || objEnd > end) break;
                        std::string obj = json.substr(objStart, objEnd - objStart + 1);
                        DataColumn col;
                        // Parse "id"
                        auto idPos = obj.find("\"id\":");
                        if (idPos != std::string::npos) {
                            col.id = static_cast<DataColumnId>(
                                std::stoul(obj.substr(idPos + 5)));
                            if (col.id > m_nextColumnId) m_nextColumnId = col.id;
                        }
                        // Parse "name"
                        auto nPos = obj.find("\"name\":\"");
                        if (nPos != std::string::npos) {
                            nPos += 8;
                            while (nPos < obj.size() && obj[nPos] != '"') col.name += obj[nPos++];
                        }
                        // Parse "type"
                        auto tPos = obj.find("\"type\":\"");
                        if (tPos != std::string::npos) {
                            tPos += 8;
                            std::string typeName;
                            while (tPos < obj.size() && obj[tPos] != '"') typeName += obj[tPos++];
                            if      (typeName == "Int")    col.type = DataColumnType::Int;
                            else if (typeName == "Float")  col.type = DataColumnType::Float;
                            else if (typeName == "Bool")   col.type = DataColumnType::Bool;
                            else if (typeName == "Enum")   col.type = DataColumnType::Enum;
                            else                           col.type = DataColumnType::String;
                        }
                        // Parse "default"
                        auto dPos = obj.find("\"default\":\"");
                        if (dPos != std::string::npos) {
                            dPos += 11;
                            while (dPos < obj.size() && obj[dPos] != '"') col.defaultValue += obj[dPos++];
                        }
                        if (col.id != kInvalidDataColumnId && !col.name.empty())
                            m_columns.push_back(col);
                        pos = objEnd + 1;
                    }
                }
            }
        }

        // Restore rows from "rows" JSON array.
        // Format: "rows":[{"id":1,"cells":{"1":"val","2":"val"}},...]
        {
            auto pos = json.find("\"rows\":[");
            if (pos != std::string::npos) {
                pos += 8;
                size_t arrayEnd = json.find(']', pos);
                if (arrayEnd != std::string::npos) {
                    m_rows.clear();
                    m_nextRowId = 0;
                    while (pos < arrayEnd) {
                        auto objStart = json.find('{', pos);
                        if (objStart == std::string::npos || objStart >= arrayEnd) break;
                        // Find matching closing brace (accounting for nested cells object)
                        int depth = 0;
                        size_t objEnd = objStart;
                        for (; objEnd < json.size(); ++objEnd) {
                            if (json[objEnd] == '{') ++depth;
                            else if (json[objEnd] == '}') { if (--depth == 0) break; }
                        }
                        if (objEnd >= json.size()) break;
                        std::string obj = json.substr(objStart, objEnd - objStart + 1);
                        DataRow row;
                        // Parse "id"
                        auto idPos = obj.find("\"id\":");
                        if (idPos != std::string::npos) {
                            row.id = static_cast<DataRowId>(
                                std::stoul(obj.substr(idPos + 5)));
                            if (row.id > m_nextRowId) m_nextRowId = row.id;
                        }
                        // Parse cells: "cells":{"colId":"value",...}
                        auto cellsPos = obj.find("\"cells\":{");
                        if (cellsPos != std::string::npos) {
                            cellsPos += 9;
                            size_t cellsEnd = obj.find('}', cellsPos);
                            if (cellsEnd != std::string::npos) {
                                size_t cp = cellsPos;
                                while (cp < cellsEnd) {
                                    // Find next key
                                    auto kq = obj.find('"', cp);
                                    if (kq == std::string::npos || kq >= cellsEnd) break;
                                    auto kq2 = obj.find('"', kq + 1);
                                    if (kq2 == std::string::npos) break;
                                    std::string colIdStr = obj.substr(kq + 1, kq2 - kq - 1);
                                    // Find value
                                    auto vq = obj.find('"', kq2 + 2);
                                    if (vq == std::string::npos) break;
                                    auto vq2 = obj.find('"', vq + 1);
                                    if (vq2 == std::string::npos) break;
                                    std::string val = obj.substr(vq + 1, vq2 - vq - 1);
                                    try {
                                        DataColumnId cid = static_cast<DataColumnId>(
                                            std::stoul(colIdStr));
                                        row.cells[cid] = val;
                                    } catch (...) {}
                                    cp = vq2 + 1;
                                }
                            }
                        }
                        if (row.id != kInvalidDataRowId)
                            m_rows.push_back(row);
                        pos = objEnd + 1;
                    }
                }
            }
        }

        clearDirty();
        return true;
    }

    /// Produce a complete JSON serialization of the table schema and rows.
    [[nodiscard]] std::string serialize() const {
        std::string out = "{\n";
        // Escape a JSON string value.
        auto escapeJson = [](const std::string& s) -> std::string {
            std::string r;
            for (char c : s) {
                if      (c == '"')  r += "\\\"";
                else if (c == '\\') r += "\\\\";
                else if (c == '\n') r += "\\n";
                else if (c == '\r') r += "\\r";
                else if (c == '\t') r += "\\t";
                else                r += c;
            }
            return r;
        };

        out += "  \"table\": \"" + escapeJson(m_tableName) + "\",\n";
        out += "  \"columns\": [\n";
        for (size_t i = 0; i < m_columns.size(); ++i) {
            const auto& col = m_columns[i];
            out += "    {\"id\":" + std::to_string(col.id);
            out += ",\"name\":\"" + escapeJson(col.name) + "\"";
            out += ",\"type\":\"" + std::string(dataColumnTypeName(col.type)) + "\"";
            out += ",\"default\":\"" + escapeJson(col.defaultValue) + "\"";
            out += ",\"required\":" + std::string(col.required ? "true" : "false");
            out += "}";
            if (i + 1 < m_columns.size()) out += ",";
            out += "\n";
        }
        out += "  ],\n";
        out += "  \"rows\": [\n";
        for (size_t i = 0; i < m_rows.size(); ++i) {
            const auto& row = m_rows[i];
            out += "    {\"id\":" + std::to_string(row.id);
            out += ",\"cells\":{";
            bool first = true;
            for (const auto& [colId, val] : row.cells) {
                if (!first) out += ",";
                out += "\"" + std::to_string(colId) + "\":\"" + escapeJson(val) + "\"";
                first = false;
            }
            out += "}}";
            if (i + 1 < m_rows.size()) out += ",";
            out += "\n";
        }
        out += "  ]\n";
        out += "}\n";
        return out;
    }

private:
    std::string  m_tableName;
    std::string  m_assetPath;
    bool         m_dirty = false;

    DataColumnId          m_nextColumnId = 0u;
    DataRowId             m_nextRowId    = 0u;
    std::vector<DataColumn> m_columns;
    std::vector<DataRow>    m_rows;
};

} // namespace NF
