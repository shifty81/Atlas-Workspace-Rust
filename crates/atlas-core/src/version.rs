//! Workspace versioning constants.

/// Human-readable version string.
pub const VERSION: &str = env!("CARGO_PKG_VERSION");

/// Version components.
pub const VERSION_MAJOR: u32 = 0;
pub const VERSION_MINOR: u32 = 1;
pub const VERSION_PATCH: u32 = 0;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn version_string_is_non_empty() {
        assert!(!VERSION.is_empty());
    }

    #[test]
    fn version_components_are_zero_or_positive() {
        // Just verifying the constants compile and are accessible.
        let _s = format!("{}.{}.{}", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    }
}
