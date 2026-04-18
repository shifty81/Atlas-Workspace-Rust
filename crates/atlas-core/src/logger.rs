//! Structured logger backed by `env_logger`.
//!
//! Mirrors the C++ `atlas::Logger` with Init / Shutdown / Info / Warn / Error.

use std::sync::Once;

static INIT: Once = Once::new();

/// Facade for the Atlas logging system.
pub struct Logger;

impl Logger {
    /// Initialise the global logger.  Safe to call multiple times.
    pub fn init() {
        INIT.call_once(|| {
            env_logger::Builder::from_default_env()
                .format_timestamp_millis()
                .init();
            log::info!("[Core] Atlas Engine Core initialised");
            log::info!("[Core] Version: {}", crate::VERSION);
        });
    }

    /// Shut down the logger (no-op for env_logger; provided for API parity).
    pub fn shutdown() {
        log::info!("[Core] Atlas Engine Core shutdown");
    }

    pub fn info(msg: &str) {
        log::info!("{}", msg);
    }

    pub fn warn(msg: &str) {
        log::warn!("{}", msg);
    }

    pub fn error(msg: &str) {
        log::error!("{}", msg);
    }
}
