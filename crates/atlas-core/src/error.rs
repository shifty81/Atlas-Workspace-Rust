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

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn display_serialization() {
        let e = AtlasError::Serialization("bad json".into());
        assert!(e.to_string().contains("serialization"));
    }

    #[test]
    fn display_not_found() {
        let e = AtlasError::NotFound("asset.png".into());
        assert!(e.to_string().contains("not found"));
    }

    #[test]
    fn display_invalid_state() {
        let e = AtlasError::InvalidState("wrong phase".into());
        assert!(e.to_string().contains("invalid state"));
    }

    #[test]
    fn display_other() {
        let e = AtlasError::Other("custom error".into());
        assert!(e.to_string().contains("custom error"));
    }

    #[test]
    fn from_io_error() {
        let io = std::io::Error::new(std::io::ErrorKind::NotFound, "file missing");
        let e = AtlasError::from(io);
        assert!(matches!(e, AtlasError::Io(_)));
        assert!(e.to_string().contains("I/O"));
    }
}
