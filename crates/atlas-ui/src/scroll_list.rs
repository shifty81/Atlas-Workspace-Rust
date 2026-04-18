//! [`ScrollList`] — virtual scroll list for efficiently displaying large item
//! collections (M14).
//!
//! Tracks a window of visible rows within a total item count so that only the
//! rows inside the viewport need to be rendered.  Stateless with respect to
//! the rendering backend — callers decide how to draw each row.

/// A virtual scroll state for a list of `total_items` rows.
#[derive(Debug, Clone)]
pub struct ScrollList {
    /// Total number of items in the list.
    pub total_items: usize,
    /// Number of rows that fit in the visible viewport at once.
    pub visible_rows: usize,
    /// Current scroll offset (index of the first visible row).
    pub offset: usize,
    /// Height of a single row in logical pixels.
    pub row_height: f32,
}

impl ScrollList {
    /// Create a new scroll list.
    ///
    /// * `total_items`  — number of items in the backing collection.
    /// * `visible_rows` — how many rows fit in the viewport.
    /// * `row_height`   — height in logical pixels of one row (for pixel-level APIs).
    pub fn new(total_items: usize, visible_rows: usize, row_height: f32) -> Self {
        Self {
            total_items,
            visible_rows,
            offset: 0,
            row_height,
        }
    }

    /// The range of item indices that are currently visible.
    ///
    /// Returns an empty range when the list is empty.
    pub fn visible_range(&self) -> std::ops::Range<usize> {
        if self.total_items == 0 { return 0..0; }
        let start = self.offset.min(self.max_offset());
        let end   = (start + self.visible_rows).min(self.total_items);
        start..end
    }

    /// Maximum valid offset (the last position where the viewport does not
    /// extend past the end of the list).
    pub fn max_offset(&self) -> usize {
        if self.total_items <= self.visible_rows {
            0
        } else {
            self.total_items - self.visible_rows
        }
    }

    /// Scroll down by `rows` rows (clamped to the end of the list).
    pub fn scroll_down(&mut self, rows: usize) {
        self.offset = (self.offset + rows).min(self.max_offset());
    }

    /// Scroll up by `rows` rows (clamped to the start of the list).
    pub fn scroll_up(&mut self, rows: usize) {
        self.offset = self.offset.saturating_sub(rows);
    }

    /// Jump to the start of the list.
    pub fn scroll_to_top(&mut self) {
        self.offset = 0;
    }

    /// Jump to the end of the list.
    pub fn scroll_to_bottom(&mut self) {
        self.offset = self.max_offset();
    }

    /// Ensure `item_index` is within the visible range, scrolling as little as
    /// possible.  Does nothing if the index is already visible.
    pub fn scroll_to_item(&mut self, item_index: usize) {
        if item_index >= self.total_items { return; }
        if item_index < self.offset {
            self.offset = item_index;
        } else if item_index >= self.offset + self.visible_rows {
            self.offset = (item_index + 1).saturating_sub(self.visible_rows);
        }
        self.offset = self.offset.min(self.max_offset());
    }

    /// Update the total item count (e.g. when the backing list changes).
    /// Clamps the offset if it would now be out of range.
    pub fn set_total_items(&mut self, count: usize) {
        self.total_items = count;
        self.offset = self.offset.min(self.max_offset());
    }

    /// Update the visible row count (e.g. when the panel is resized).
    pub fn set_visible_rows(&mut self, rows: usize) {
        self.visible_rows = rows.max(1);
        self.offset = self.offset.min(self.max_offset());
    }

    /// Total content height in logical pixels.
    pub fn content_height(&self) -> f32 {
        self.total_items as f32 * self.row_height
    }

    /// Viewport height in logical pixels.
    pub fn viewport_height(&self) -> f32 {
        self.visible_rows as f32 * self.row_height
    }

    /// Scroll position as a fraction `[0.0, 1.0]`.  Returns `0.0` when the
    /// list fits entirely within the viewport.
    pub fn scroll_fraction(&self) -> f32 {
        let max = self.max_offset();
        if max == 0 { return 0.0; }
        self.offset as f32 / max as f32
    }

    /// Whether the list is currently scrolled to the very bottom.
    pub fn is_at_bottom(&self) -> bool {
        self.offset >= self.max_offset()
    }

    /// Whether the list is currently scrolled to the very top.
    pub fn is_at_top(&self) -> bool {
        self.offset == 0
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn list(total: usize, visible: usize) -> ScrollList {
        ScrollList::new(total, visible, 20.0)
    }

    #[test]
    fn new_starts_at_top() {
        let sl = list(100, 10);
        assert_eq!(sl.offset, 0);
        assert!(sl.is_at_top());
    }

    #[test]
    fn visible_range_initial() {
        let sl = list(100, 10);
        assert_eq!(sl.visible_range(), 0..10);
    }

    #[test]
    fn visible_range_empty_list() {
        let sl = list(0, 10);
        assert_eq!(sl.visible_range(), 0..0);
    }

    #[test]
    fn visible_range_smaller_than_viewport() {
        let sl = list(5, 10);
        assert_eq!(sl.visible_range(), 0..5);
    }

    #[test]
    fn scroll_down_clamps() {
        let mut sl = list(20, 10);
        sl.scroll_down(100);
        assert_eq!(sl.offset, 10); // max_offset = 20-10 = 10
        assert!(sl.is_at_bottom());
    }

    #[test]
    fn scroll_up_clamps() {
        let mut sl = list(20, 10);
        sl.scroll_down(5);
        sl.scroll_up(100);
        assert_eq!(sl.offset, 0);
        assert!(sl.is_at_top());
    }

    #[test]
    fn scroll_to_top_and_bottom() {
        let mut sl = list(50, 10);
        sl.scroll_to_bottom();
        assert!(sl.is_at_bottom());
        sl.scroll_to_top();
        assert!(sl.is_at_top());
    }

    #[test]
    fn scroll_to_item_brings_into_view_below() {
        let mut sl = list(100, 10);
        sl.scroll_to_item(25);
        assert!(sl.visible_range().contains(&25));
    }

    #[test]
    fn scroll_to_item_brings_into_view_above() {
        let mut sl = list(100, 10);
        sl.scroll_down(30);
        sl.scroll_to_item(5);
        assert_eq!(sl.offset, 5);
    }

    #[test]
    fn scroll_to_item_already_visible_noop() {
        let mut sl = list(100, 10);
        sl.scroll_down(5);
        let before = sl.offset;
        sl.scroll_to_item(7); // inside 5..15
        assert_eq!(sl.offset, before);
    }

    #[test]
    fn set_total_items_clamps_offset() {
        let mut sl = list(100, 10);
        sl.scroll_to_bottom(); // offset = 90
        sl.set_total_items(15); // max_offset becomes 5
        assert!(sl.offset <= 5);
    }

    #[test]
    fn set_visible_rows_clamps_offset() {
        let mut sl = list(100, 10);
        sl.scroll_down(90);
        sl.set_visible_rows(50); // now max_offset = 50
        assert!(sl.offset <= 50);
    }

    #[test]
    fn content_and_viewport_height() {
        let sl = list(100, 10);
        assert!((sl.content_height() - 2000.0).abs() < f32::EPSILON);
        assert!((sl.viewport_height() - 200.0).abs() < f32::EPSILON);
    }

    #[test]
    fn scroll_fraction_extremes() {
        let mut sl = list(100, 10);
        assert!((sl.scroll_fraction() - 0.0).abs() < f32::EPSILON);
        sl.scroll_to_bottom();
        assert!((sl.scroll_fraction() - 1.0).abs() < f32::EPSILON);
    }

    #[test]
    fn scroll_fraction_fits_entirely() {
        let sl = list(5, 10); // all items fit
        assert!((sl.scroll_fraction() - 0.0).abs() < f32::EPSILON);
    }
}
