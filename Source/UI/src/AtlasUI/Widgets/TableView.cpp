#include "NF/UI/AtlasUI/Widgets/TableView.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"

namespace NF::UI::AtlasUI {

bool TableView::selectRow(int index) {
    if (index < 0 || static_cast<size_t>(index) >= m_rows.size()) {
        m_selectedRow = -1;
        return false;
    }
    if (m_selectedRow >= 0 && static_cast<size_t>(m_selectedRow) < m_rows.size()) {
        m_rows[static_cast<size_t>(m_selectedRow)].selected = false;
    }
    m_selectedRow = index;
    m_rows[static_cast<size_t>(index)].selected = true;
    if (m_onSelect) m_onSelect(static_cast<size_t>(index));
    return true;
}

void TableView::sort(size_t columnIndex, SortDirection direction) {
    m_sortColumn = columnIndex;
    m_sortDirection = direction;
    if (m_onSort) m_onSort(columnIndex, direction);
}

void TableView::measure(ILayoutContext&) {
    float totalHeight = headerHeight();
    totalHeight += static_cast<float>(m_rows.size()) * m_rowHeight;
    m_bounds.h = totalHeight;
}

void TableView::paint(IPaintContext& context) {
    if (!m_visible) return;

    context.pushClip(m_bounds);

    float y = m_bounds.y;

    // Draw header
    if (m_headerVisible && !m_columns.empty()) {
        NF::Rect headerBg = {m_bounds.x, y, m_bounds.w, m_headerRowHeight};
        context.fillRect(headerBg, Theme::ColorToken::SurfaceAlt);

        float colX = m_bounds.x;
        for (size_t c = 0; c < m_columns.size(); ++c) {
            const auto& col = m_columns[c];
            NF::Rect cellRect = {colX, y, col.width, m_headerRowHeight};
            context.drawText(insetRect(cellRect, Theme::Spacing::Small, Theme::Spacing::Tiny),
                             col.header, 0, Theme::ColorToken::Text);

            // Sort indicator
            if (col.sortable && c == m_sortColumn && m_sortDirection != SortDirection::None) {
                const char* indicator = m_sortDirection == SortDirection::Ascending ? "^" : "v";
                NF::Rect indicatorRect = {colX + col.width - 16.f, y + 4.f, 12.f, m_headerRowHeight - 8.f};
                context.drawText(indicatorRect, indicator, 0, Theme::ColorToken::Accent);
            }

            // Column separator
            NF::Rect sep = {colX + col.width - 1.f, y, 1.f, m_headerRowHeight};
            context.fillRect(sep, Theme::ColorToken::Border);

            colX += col.width;
        }

        // Header bottom border
        NF::Rect headerBorder = {m_bounds.x, y + m_headerRowHeight - 1.f, m_bounds.w, 1.f};
        context.fillRect(headerBorder, Theme::ColorToken::Border);

        y += m_headerRowHeight;
    }

    // Draw rows
    for (size_t r = 0; r < m_rows.size(); ++r) {
        const auto& row = m_rows[r];
        NF::Rect rowRect = {m_bounds.x, y, m_bounds.w, m_rowHeight};

        if (row.selected) {
            context.fillRect(rowRect, Theme::ColorToken::Selection);
        } else if (static_cast<int>(r) == m_hoveredRow) {
            context.fillRect(rowRect, Theme::ColorToken::SurfaceAlt);
        }

        float colX = m_bounds.x;
        for (size_t c = 0; c < m_columns.size() && c < row.cells.size(); ++c) {
            NF::Rect cellRect = {colX, y, m_columns[c].width, m_rowHeight};
            context.drawText(insetRect(cellRect, Theme::Spacing::Small, Theme::Spacing::Tiny),
                             row.cells[c], 0, Theme::ColorToken::Text);
            colX += m_columns[c].width;
        }

        y += m_rowHeight;
    }

    context.popClip();
}

bool TableView::handleInput(IInputContext& context) {
    if (!m_visible) return false;
    if (!rectContains(m_bounds, context.mousePosition())) {
        m_hoveredRow = -1;
        return false;
    }

    float y = m_bounds.y + headerHeight();

    // Handle header click for sorting
    if (m_headerVisible && !m_columns.empty()) {
        NF::Rect headerRect = {m_bounds.x, m_bounds.y, m_bounds.w, m_headerRowHeight};
        if (rectContains(headerRect, context.mousePosition()) && context.primaryDown()) {
            float colX = m_bounds.x;
            for (size_t c = 0; c < m_columns.size(); ++c) {
                NF::Rect colRect = {colX, m_bounds.y, m_columns[c].width, m_headerRowHeight};
                if (m_columns[c].sortable && rectContains(colRect, context.mousePosition())) {
                    SortDirection newDir = (m_sortColumn == c && m_sortDirection == SortDirection::Ascending)
                                               ? SortDirection::Descending
                                               : SortDirection::Ascending;
                    sort(c, newDir);
                    return true;
                }
                colX += m_columns[c].width;
            }
            return true;
        }
    }

    // Handle row click for selection
    m_hoveredRow = -1;
    for (size_t r = 0; r < m_rows.size(); ++r) {
        NF::Rect rowRect = {m_bounds.x, y, m_bounds.w, m_rowHeight};
        if (rectContains(rowRect, context.mousePosition())) {
            m_hoveredRow = static_cast<int>(r);
            if (context.primaryDown()) {
                selectRow(static_cast<int>(r));
                return true;
            }
            break;
        }
        y += m_rowHeight;
    }

    return m_hoveredRow >= 0 || rectContains(m_bounds, context.mousePosition());
}

} // namespace NF::UI::AtlasUI
