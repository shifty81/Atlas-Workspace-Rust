//! Editor panel modules (M6).

pub mod asset_browser;
pub mod console;
pub mod outliner;
pub mod properties;
pub mod viewport;

pub use asset_browser::AssetBrowserPanel;
pub use console::ConsolePanel;
pub use outliner::{OutlinerPanel, OutlinerEvent};
pub use properties::PropertiesPanel;
pub use viewport::ViewportPanel;
