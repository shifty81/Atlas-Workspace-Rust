//! Atlas base error and result types.

use thiserror::Error;

/// Top-level error type for Atlas systems.
#[derive(Debug, Error)]
pub enum AtlasError {
    #[error("I/O error: {0}")]
    Io(#[from] std::io::Error),

    #[error("serialization error: {0}")]
    Serialization(String),

    #[error("not found: {0}")]
    NotFound(String),

    #[error("invalid state: {0}")]
    InvalidState(String),

    #[error("{0}")]
    Other(String),
}

/// Convenience alias.
pub type AtlasResult<T> = std::result::Result<T, AtlasError>;
