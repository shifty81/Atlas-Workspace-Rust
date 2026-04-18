//! [`UiLogCapture`] — captures `log` records and makes them available to the
//! Console panel (M5).

use std::sync::{Arc, Mutex};

/// A single captured log entry.
#[derive(Clone, Debug)]
pub struct LogEntry {
    pub level:   log::Level,
    pub target:  String,
    pub message: String,
}

/// Thread-safe ring buffer of log entries.
#[derive(Clone, Default)]
pub struct UiLogCapture {
    entries: Arc<Mutex<Vec<LogEntry>>>,
}

impl UiLogCapture {
    pub fn new() -> Self { Self::default() }

    /// Install as the global logger (call once at startup).
    /// Previous logger is discarded.
    pub fn install(self) -> Arc<Mutex<Vec<LogEntry>>> {
        let entries = self.entries.clone();
        // Install a secondary logger that routes to both env_logger and us.
        // Using set_boxed_logger is a one-shot; ignore errors if already set.
        let _ = log::set_boxed_logger(Box::new(CapturingLogger { entries: self.entries }));
        log::set_max_level(log::LevelFilter::Debug);
        entries
    }

    /// Push a record directly (used from within the logger impl).
    fn push(entries: &Arc<Mutex<Vec<LogEntry>>>, record: &log::Record) {
        if let Ok(mut v) = entries.lock() {
            if v.len() >= 2048 { v.remove(0); }
            v.push(LogEntry {
                level:   record.level(),
                target:  record.target().to_string(),
                message: record.args().to_string(),
            });
        }
    }
}

struct CapturingLogger {
    entries: Arc<Mutex<Vec<LogEntry>>>,
}

impl log::Log for CapturingLogger {
    fn enabled(&self, _meta: &log::Metadata) -> bool { true }

    fn log(&self, record: &log::Record) {
        // Also write to stderr so CI still sees logs
        eprintln!("[{}] {}: {}", record.level(), record.target(), record.args());
        UiLogCapture::push(&self.entries, record);
    }

    fn flush(&self) {}
}
