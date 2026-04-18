#pragma once
// NF::Editor — Table view v1: columnar data model with sorting and selection
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <variant>
#include <vector>

namespace NF {

// ── Column Sort Direction ─────────────────────────────────────────

enum class SortDirection : uint8_t { None, Ascending, Descending };

inline const char* sortDirectionName(SortDirection d) {
    switch (d) {
        case SortDirection::None:       return "None";
        case SortDirection::Ascending:  return "Ascending";
        case SortDirection::Descending: return "Descending";
    }
    return "Unknown";
}

// ── Table Cell Value ──────────────────────────────────────────────

using TableCellValue = std::variant<bool, int, float, std::string>;

[[nodiscard]] inline std::string tableCellToString(const TableCellValue& v) {
    return std::visit([](const auto& val) -> std::string {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, bool>)        return val ? "true" : "false";
        else if constexpr (std::is_same_v<T, int>)    return std::to_string(val);
        else if constexpr (std::is_same_v<T, float>)  return std::to_string(val);
        else if constexpr (std::is_same_v<T, std::string>) return val;
        return "";
    }, v);
}

// ── Table Column Descriptor ───────────────────────────────────────

struct TableColumnDesc {
    uint32_t    id        = 0;
    std::string header;
    float       widthPx   = 100.f;
    bool        sortable  = true;
    bool        resizable = true;
    bool        visible   = true;

    [[nodiscard]] bool isValid() const { return id != 0 && !header.empty(); }
};

// ── Table Row ─────────────────────────────────────────────────────

struct TableRow {
    uint32_t                    id = 0;
    std::vector<TableCellValue> cells;  // ordered by column index
    bool                        selected = false;

    [[nodiscard]] bool isValid() const { return id != 0; }
    [[nodiscard]] const TableCellValue* cell(size_t colIndex) const {
        return colIndex < cells.size() ? &cells[colIndex] : nullptr;
    }
};

// ── Table View V1 ─────────────────────────────────────────────────

using TableSelectCallback = std::function<void(uint32_t rowId)>;

class TableViewV1 {
public:
    static constexpr size_t MAX_COLUMNS = 32;
    static constexpr size_t MAX_ROWS    = 65536;

    bool addColumn(const TableColumnDesc& col) {
        if (!col.isValid()) return false;
        if (m_columns.size() >= MAX_COLUMNS) return false;
        for (const auto& c : m_columns) if (c.id == col.id) return false;
        m_columns.push_back(col);
        return true;
    }

    bool removeColumn(uint32_t colId) {
        size_t idx = colIndex(colId);
        if (idx == npos) return false;
        m_columns.erase(m_columns.begin() + static_cast<ptrdiff_t>(idx));
        // Remove corresponding cell from all rows
        for (auto& r : m_rows) {
            if (idx < r.cells.size())
                r.cells.erase(r.cells.begin() + static_cast<ptrdiff_t>(idx));
        }
        return true;
    }

    bool addRow(const TableRow& row) {
        if (!row.isValid()) return false;
        if (m_rows.size() >= MAX_ROWS) return false;
        for (const auto& r : m_rows) if (r.id == row.id) return false;
        m_rows.push_back(row);
        return true;
    }

    bool removeRow(uint32_t rowId) {
        for (auto it = m_rows.begin(); it != m_rows.end(); ++it) {
            if (it->id == rowId) {
                // Deselect if selected
                if (it->selected) {
                    m_selectedIds.erase(
                        std::remove(m_selectedIds.begin(), m_selectedIds.end(), rowId),
                        m_selectedIds.end());
                }
                m_rows.erase(it);
                return true;
            }
        }
        return false;
    }

    bool setCellValue(uint32_t rowId, uint32_t colId, const TableCellValue& val) {
        size_t idx = colIndex(colId);
        if (idx == npos) return false;
        auto* row = findRowMut(rowId);
        if (!row) return false;
        if (idx >= row->cells.size()) row->cells.resize(idx + 1);
        row->cells[idx] = val;
        return true;
    }

    [[nodiscard]] const TableCellValue* getCellValue(uint32_t rowId, uint32_t colId) const {
        size_t idx = colIndex(colId);
        if (idx == npos) return nullptr;
        const auto* row = findRow(rowId);
        if (!row) return nullptr;
        return row->cell(idx);
    }

    // Selection
    bool selectRow(uint32_t rowId, bool exclusive = true) {
        auto* row = findRowMut(rowId);
        if (!row) return false;
        if (exclusive) {
            for (auto& r : m_rows) r.selected = false;
            m_selectedIds.clear();
        }
        row->selected = true;
        m_selectedIds.push_back(rowId);
        ++m_selectCount;
        if (m_onSelect) m_onSelect(rowId);
        return true;
    }

    void clearSelection() {
        for (auto& r : m_rows) r.selected = false;
        m_selectedIds.clear();
    }

    // Sorting
    void sort(uint32_t colId, SortDirection dir) {
        size_t idx = colIndex(colId);
        if (idx == npos || dir == SortDirection::None) {
            m_sortColId  = 0;
            m_sortDir    = SortDirection::None;
            return;
        }
        m_sortColId = colId;
        m_sortDir   = dir;

        std::stable_sort(m_rows.begin(), m_rows.end(),
            [idx, dir](const TableRow& a, const TableRow& b) {
                const auto* ca = a.cell(idx);
                const auto* cb = b.cell(idx);
                if (!ca || !cb) return false;
                std::string sa = tableCellToString(*ca);
                std::string sb = tableCellToString(*cb);
                return dir == SortDirection::Ascending ? sa < sb : sa > sb;
            });
    }

    // Column visibility
    bool setColumnVisible(uint32_t colId, bool visible) {
        for (auto& c : m_columns) {
            if (c.id == colId) { c.visible = visible; return true; }
        }
        return false;
    }

    bool setColumnWidth(uint32_t colId, float widthPx) {
        for (auto& c : m_columns) {
            if (c.id == colId) { c.widthPx = widthPx > 10.f ? widthPx : 10.f; return true; }
        }
        return false;
    }

    const TableRow* findRow(uint32_t id) const {
        for (const auto& r : m_rows) if (r.id == id) return &r;
        return nullptr;
    }

    void setOnSelect(TableSelectCallback cb) { m_onSelect = std::move(cb); }

    [[nodiscard]] size_t columnCount()    const { return m_columns.size();    }
    [[nodiscard]] size_t rowCount()       const { return m_rows.size();       }
    [[nodiscard]] size_t selectedCount()  const { return m_selectedIds.size(); }
    [[nodiscard]] size_t selectCount()    const { return m_selectCount;        }
    [[nodiscard]] uint32_t sortColId()    const { return m_sortColId;          }
    [[nodiscard]] SortDirection sortDir() const { return m_sortDir;            }
    [[nodiscard]] const std::vector<TableColumnDesc>& columns() const { return m_columns; }
    [[nodiscard]] const std::vector<TableRow>&        rows()    const { return m_rows;    }
    [[nodiscard]] const std::vector<uint32_t>& selectedIds()   const { return m_selectedIds; }

private:
    static constexpr size_t npos = static_cast<size_t>(-1);

    size_t colIndex(uint32_t colId) const {
        for (size_t i = 0; i < m_columns.size(); ++i)
            if (m_columns[i].id == colId) return i;
        return npos;
    }

    TableRow* findRowMut(uint32_t id) {
        for (auto& r : m_rows) if (r.id == id) return &r;
        return nullptr;
    }

    std::vector<TableColumnDesc> m_columns;
    std::vector<TableRow>        m_rows;
    std::vector<uint32_t>        m_selectedIds;
    TableSelectCallback          m_onSelect;
    uint32_t                     m_sortColId  = 0;
    SortDirection                m_sortDir    = SortDirection::None;
    size_t                       m_selectCount = 0;
};

} // namespace NF
