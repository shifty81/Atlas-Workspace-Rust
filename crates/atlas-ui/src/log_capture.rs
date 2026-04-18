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

#[cfg(test)]
mod tests {
    use super::*;

    // Helper macro: binds format_args BEFORE calling the builder to ensure the
    // temporary string lives for the whole block (not just the method chain).
    macro_rules! push_entry {
        ($entries:expr, $level:expr, $msg:expr) => {{
            let _s = $msg.to_string();
            let _args = format_args!("{_s}");
            let mut _b = log::Record::builder();
            _b.level($level).target("test").args(_args);
            let _record = _b.build();
            UiLogCapture::push($entries, &_record);
        }};
    }

    macro_rules! push_entry_target {
        ($entries:expr, $level:expr, $target:expr, $msg:expr) => {{
            let _s = $msg.to_string();
            let _args = format_args!("{_s}");
            let mut _b = log::Record::builder();
            _b.level($level).target($target).args(_args);
            let _record = _b.build();
            UiLogCapture::push($entries, &_record);
        }};
    }

    #[test]
    fn new_capture_is_empty() {
        let cap = UiLogCapture::new();
        assert!(cap.entries.lock().unwrap().is_empty());
    }

    #[test]
    fn push_adds_entry() {
        let cap = UiLogCapture::new();
        push_entry!(&cap.entries, log::Level::Info, "hello");
        let entries = cap.entries.lock().unwrap();
        assert_eq!(entries.len(), 1);
        assert_eq!(entries[0].message, "hello");
        assert_eq!(entries[0].level, log::Level::Info);
    }

    #[test]
    fn push_preserves_level() {
        let cap = UiLogCapture::new();
        push_entry!(&cap.entries, log::Level::Warn, "w");
        push_entry!(&cap.entries, log::Level::Error, "e");
        let entries = cap.entries.lock().unwrap();
        assert_eq!(entries[0].level, log::Level::Warn);
        assert_eq!(entries[1].level, log::Level::Error);
    }

    #[test]
    fn ring_buffer_caps_at_2048() {
        let cap = UiLogCapture::new();
        for i in 0..2050_usize {
            push_entry!(&cap.entries, log::Level::Debug, format!("msg-{i}"));
        }
        let entries = cap.entries.lock().unwrap();
        // Ring buffer should not grow beyond 2048.
        assert!(entries.len() <= 2048);
    }

    #[test]
    fn ring_buffer_drops_oldest() {
        let cap = UiLogCapture::new();
        // Fill to capacity then push one more.
        for i in 0..2048_usize {
            push_entry!(&cap.entries, log::Level::Debug, format!("msg-{i}"));
        }
        push_entry!(&cap.entries, log::Level::Info, "newest");
        let entries = cap.entries.lock().unwrap();
        assert_eq!(entries.last().unwrap().message, "newest");
        // First entry should no longer be "msg-0".
        assert_ne!(entries[0].message, "msg-0");
    }

    #[test]
    fn clone_shares_buffer() {
        let cap1 = UiLogCapture::new();
        let cap2 = cap1.clone();
        push_entry!(&cap1.entries, log::Level::Info, "shared");
        let entries = cap2.entries.lock().unwrap();
        assert_eq!(entries.len(), 1);
    }

    #[test]
    fn log_entry_clone() {
        let e = LogEntry {
            level: log::Level::Error,
            target: "t".to_string(),
            message: "m".to_string(),
        };
        let e2 = e.clone();
        assert_eq!(e2.level, log::Level::Error);
        assert_eq!(e2.message, "m");
    }

    #[test]
    fn target_is_captured() {
        let cap = UiLogCapture::new();
        push_entry_target!(&cap.entries, log::Level::Debug, "my::module", "msg");
        let entries = cap.entries.lock().unwrap();
        assert_eq!(entries[0].target, "my::module");
    }
}
