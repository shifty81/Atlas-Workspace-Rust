#pragma once

#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include <functional>
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

struct TableColumn {
    std::string header;
    float width = 100.f;
    bool sortable = false;
    bool resizable = true;
};

enum class SortDirection : uint8_t {
    None,
    Ascending,
    Descending
};

struct TableRow {
    std::vector<std::string> cells;
    bool selected = false;
};

class TableView final : public WidgetBase {
public:
    using SelectHandler = std::function<void(size_t rowIndex)>;
    using SortHandler = std::function<void(size_t columnIndex, SortDirection direction)>;

    TableView() = default;

    void setColumns(std::vector<TableColumn> columns) { m_columns = std::move(columns); }
    void addColumn(TableColumn column) { m_columns.push_back(std::move(column)); }

    void setRows(std::vector<TableRow> rows) { m_rows = std::move(rows); }
    void addRow(TableRow row) { m_rows.push_back(std::move(row)); }
    void clearRows() { m_rows.clear(); m_selectedRow = -1; }
    void clear() { m_columns.clear(); m_rows.clear(); m_selectedRow = -1; }

    void setOnSelect(SelectHandler handler) { m_onSelect = std::move(handler); }
    void setOnSort(SortHandler handler) { m_onSort = std::move(handler); }
    void setHeaderVisible(bool visible) { m_headerVisible = visible; }
    void setRowHeight(float height) { m_rowHeight = height; }

    [[nodiscard]] const std::vector<TableColumn>& columns() const { return m_columns; }
    [[nodiscard]] const std::vector<TableRow>& rows() const { return m_rows; }
    [[nodiscard]] size_t columnCount() const { return m_columns.size(); }
    [[nodiscard]] size_t rowCount() const { return m_rows.size(); }
    [[nodiscard]] int selectedRow() const { return m_selectedRow; }
    [[nodiscard]] bool headerVisible() const { return m_headerVisible; }
    [[nodiscard]] size_t sortColumn() const { return m_sortColumn; }
    [[nodiscard]] SortDirection sortDirection() const { return m_sortDirection; }

    bool selectRow(int index);
    void sort(size_t columnIndex, SortDirection direction);

    void measure(ILayoutContext& context) override;
    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

private:
    float headerHeight() const { return m_headerVisible ? m_headerRowHeight : 0.f; }

    std::vector<TableColumn> m_columns;
    std::vector<TableRow> m_rows;
    SelectHandler m_onSelect;
    SortHandler m_onSort;
    int m_selectedRow = -1;
    int m_hoveredRow = -1;
    size_t m_sortColumn = 0;
    SortDirection m_sortDirection = SortDirection::None;
    float m_rowHeight = 24.f;
    float m_headerRowHeight = 28.f;
    bool m_headerVisible = true;
};

} // namespace NF::UI::AtlasUI
