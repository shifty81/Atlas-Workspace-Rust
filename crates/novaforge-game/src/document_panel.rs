// SPDX-License-Identifier: GPL-3.0-only
// NovaForge document panel types — port of NovaForge IDocumentPanel / PanelUndoStack.
// Copyright (C) NovaForge contributors. GNU General Public License v3.0.

// ── DocumentPanelValidationSeverity ──────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
#[repr(u8)]
pub enum DocumentPanelValidationSeverity {
    Info    = 0,
    Warning = 1,
    Error   = 2,
}

// ── DocumentPanelValidationMessage ───────────────────────────────────────

#[derive(Debug, Clone)]
pub struct DocumentPanelValidationMessage {
    pub field:    String,
    pub message:  String,
    pub severity: DocumentPanelValidationSeverity,
}

// ── PanelUndoEntry ────────────────────────────────────────────────────────

pub struct PanelUndoEntry {
    pub label:   String,
    pub do_fn:   Box<dyn Fn() -> bool>,
    pub undo_fn: Box<dyn Fn() -> bool>,
}

// ── PanelUndoStack ────────────────────────────────────────────────────────

pub struct PanelUndoStack {
    stack:  Vec<PanelUndoEntry>,
    cursor: usize,
}

impl PanelUndoStack {
    pub fn new() -> Self {
        Self { stack: Vec::new(), cursor: 0 }
    }

    /// Push a new entry, trimming any redo future.
    pub fn push(&mut self, entry: PanelUndoEntry) {
        self.stack.truncate(self.cursor);
        self.stack.push(entry);
        self.cursor = self.stack.len();
    }

    pub fn undo(&mut self) -> bool {
        if self.cursor == 0 {
            return false;
        }
        self.cursor -= 1;
        (self.stack[self.cursor].undo_fn)()
    }

    pub fn redo(&mut self) -> bool {
        if self.cursor >= self.stack.len() {
            return false;
        }
        let ok = (self.stack[self.cursor].do_fn)();
        if ok {
            self.cursor += 1;
        }
        ok
    }

    pub fn can_undo(&self) -> bool { self.cursor > 0 }
    pub fn can_redo(&self) -> bool { self.cursor < self.stack.len() }
    pub fn undo_depth(&self) -> usize { self.cursor }
    pub fn redo_depth(&self) -> usize { self.stack.len() - self.cursor }

    pub fn clear(&mut self) {
        self.stack.clear();
        self.cursor = 0;
    }
}

impl Default for PanelUndoStack {
    fn default() -> Self { Self::new() }
}

// ── Tests ─────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::{Arc, Mutex};

    fn counter_entry(label: &str, do_log: Arc<Mutex<Vec<&'static str>>>, undo_log: Arc<Mutex<Vec<&'static str>>>, do_tag: &'static str, undo_tag: &'static str) -> PanelUndoEntry {
        PanelUndoEntry {
            label: label.to_string(),
            do_fn: Box::new(move || { do_log.lock().unwrap().push(do_tag); true }),
            undo_fn: Box::new(move || { undo_log.lock().unwrap().push(undo_tag); true }),
        }
    }

    fn simple_entry(label: &str) -> PanelUndoEntry {
        PanelUndoEntry {
            label: label.to_string(),
            do_fn: Box::new(|| true),
            undo_fn: Box::new(|| true),
        }
    }

    #[test]
    fn push_single_entry() {
        let mut stack = PanelUndoStack::new();
        stack.push(simple_entry("a"));
        assert_eq!(stack.undo_depth(), 1);
        assert_eq!(stack.redo_depth(), 0);
    }

    #[test]
    fn undo_moves_cursor() {
        let mut stack = PanelUndoStack::new();
        stack.push(simple_entry("a"));
        let ok = stack.undo();
        assert!(ok);
        assert_eq!(stack.undo_depth(), 0);
        assert_eq!(stack.redo_depth(), 1);
    }

    #[test]
    fn redo_after_undo() {
        let mut stack = PanelUndoStack::new();
        stack.push(simple_entry("a"));
        stack.undo();
        let ok = stack.redo();
        assert!(ok);
        assert_eq!(stack.undo_depth(), 1);
        assert_eq!(stack.redo_depth(), 0);
    }

    #[test]
    fn can_undo_can_redo() {
        let mut stack = PanelUndoStack::new();
        assert!(!stack.can_undo());
        assert!(!stack.can_redo());
        stack.push(simple_entry("a"));
        assert!(stack.can_undo());
        assert!(!stack.can_redo());
        stack.undo();
        assert!(!stack.can_undo());
        assert!(stack.can_redo());
    }

    #[test]
    fn clear_resets_stack() {
        let mut stack = PanelUndoStack::new();
        stack.push(simple_entry("a"));
        stack.push(simple_entry("b"));
        stack.clear();
        assert_eq!(stack.undo_depth(), 0);
        assert_eq!(stack.redo_depth(), 0);
        assert!(!stack.can_undo());
        assert!(!stack.can_redo());
    }

    #[test]
    fn push_trims_redo_future() {
        let mut stack = PanelUndoStack::new();
        stack.push(simple_entry("a"));
        stack.push(simple_entry("b"));
        stack.undo(); // cursor=1, redo future = b
        stack.push(simple_entry("c")); // should trim b
        assert_eq!(stack.undo_depth(), 2); // a + c
        assert_eq!(stack.redo_depth(), 0);
    }

    #[test]
    fn undo_returns_false_when_empty() {
        let mut stack = PanelUndoStack::new();
        assert!(!stack.undo());
    }

    #[test]
    fn redo_returns_false_when_no_future() {
        let mut stack = PanelUndoStack::new();
        stack.push(simple_entry("a"));
        assert!(!stack.redo()); // nothing to redo yet
    }

    #[test]
    fn undo_calls_undo_fn() {
        let do_log: Arc<Mutex<Vec<&'static str>>> = Arc::new(Mutex::new(Vec::new()));
        let undo_log: Arc<Mutex<Vec<&'static str>>> = Arc::new(Mutex::new(Vec::new()));
        let entry = counter_entry("e", do_log.clone(), undo_log.clone(), "do", "undo");
        let mut stack = PanelUndoStack::new();
        stack.push(entry);
        stack.undo();
        assert_eq!(*undo_log.lock().unwrap(), vec!["undo"]);
    }
}
