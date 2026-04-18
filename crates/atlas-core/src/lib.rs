//! # Atlas Core
//!
//! Foundation types and systems for the Atlas Workspace.
//!
//! Provides:
//! - [`logger`] — structured logging with file output
//! - [`string_id`] — compile-time and runtime string interning
//! - [`version`] — versioning constants
//! - [`error`] — base error types

pub mod error;
pub mod logger;
pub mod string_id;
pub mod version;

pub use error::{AtlasError, AtlasResult};
pub use logger::Logger;
pub use string_id::StringId;
pub use version::VERSION;
