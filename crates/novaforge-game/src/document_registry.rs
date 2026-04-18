//! [`DocumentRegistry`] — tracks registered NovaForge document types and
//! open document instances.
//!
//! A *document type* is a named category of editor document (e.g. `"scene"`,
//! `"material"`, `"animation"`).  When the editor opens a file it looks up
//! the type in this registry to determine which panel to open.
//!
//! # License
//!
//! GPL v3.0 — this file is part of `novaforge-game`.

use std::collections::HashMap;

// ── DocumentTypeDescriptor ────────────────────────────────────────────────────

/// Metadata about a registered document type.
#[derive(Debug, Clone)]
pub struct DocumentTypeDescriptor {
    /// Stable identifier for this document type (e.g. `"scene"`, `"material"`).
    pub type_id: String,
    /// Human-readable display name shown in the UI.
    pub display_name: String,
    /// File extension(s) associated with this document type (e.g. `"json"`).
    pub file_extensions: Vec<String>,
    /// Panel ID that handles this document type.
    pub panel_id: String,
}

impl DocumentTypeDescriptor {
    /// Create a new descriptor.
    pub fn new(
        type_id: impl Into<String>,
        display_name: impl Into<String>,
        extensions: impl IntoIterator<Item = impl Into<String>>,
        panel_id: impl Into<String>,
    ) -> Self {
        Self {
            type_id:         type_id.into(),
            display_name:    display_name.into(),
            file_extensions: extensions.into_iter().map(Into::into).collect(),
            panel_id:        panel_id.into(),
        }
    }
}

// ── DocumentHandle ────────────────────────────────────────────────────────────

/// A reference to an open document.
#[derive(Debug, Clone)]
pub struct DocumentHandle {
    /// Unique handle ID (monotonically increasing).
    pub handle_id: u32,
    /// Document type ID (matches a [`DocumentTypeDescriptor::type_id`]).
    pub type_id: String,
    /// File path of the open document (empty for unsaved new documents).
    pub path: String,
    /// `true` if the document has unsaved changes.
    pub dirty: bool,
}

// ── DocumentRegistry ──────────────────────────────────────────────────────────

/// Tracks registered document types and currently open document handles.
pub struct DocumentRegistry {
    /// Registered document type descriptors by `type_id`.
    types: HashMap<String, DocumentTypeDescriptor>,
    /// Currently open documents, keyed by `handle_id`.
    open: HashMap<u32, DocumentHandle>,
    next_handle: u32,
}

impl DocumentRegistry {
    /// Create an empty registry.
    pub fn new() -> Self {
        Self { types: HashMap::new(), open: HashMap::new(), next_handle: 1 }
    }

    /// Create a registry pre-populated with the standard NovaForge document
    /// types (scene, material, animation, graph, data-table).
    pub fn with_novaforge_defaults() -> Self {
        let mut reg = Self::new();
        reg.register_type(DocumentTypeDescriptor::new(
            "scene",      "Scene",       ["json"],         "nf.scene"
        ));
        reg.register_type(DocumentTypeDescriptor::new(
            "material",   "Material",    ["json"],         "nf.material"
        ));
        reg.register_type(DocumentTypeDescriptor::new(
            "animation",  "Animation",   ["json", "anim"], "nf.animation"
        ));
        reg.register_type(DocumentTypeDescriptor::new(
            "graph",      "Logic Graph", ["json"],         "nf.graph"
        ));
        reg.register_type(DocumentTypeDescriptor::new(
            "data_table", "Data Table",  ["json", "csv"],  "nf.datatable"
        ));
        reg
    }

    /// Register a new document type.  Returns `false` if the `type_id` is
    /// already registered (existing entry is unchanged).
    pub fn register_type(&mut self, desc: DocumentTypeDescriptor) -> bool {
        if self.types.contains_key(&desc.type_id) { return false; }
        self.types.insert(desc.type_id.clone(), desc);
        true
    }

    /// Look up a document type by ID.
    pub fn get_type(&self, type_id: &str) -> Option<&DocumentTypeDescriptor> {
        self.types.get(type_id)
    }

    /// Find the document type that handles a given file extension.
    pub fn type_for_extension(&self, ext: &str) -> Option<&DocumentTypeDescriptor> {
        self.types.values().find(|d| {
            d.file_extensions.iter().any(|e| e == ext)
        })
    }

    /// Open a document of the given type at the given path.  Returns the
    /// assigned handle ID.
    pub fn open_document(&mut self, type_id: &str, path: &str) -> Result<u32, String> {
        if !self.types.contains_key(type_id) {
            return Err(format!("unknown document type '{type_id}'"));
        }
        let id = self.next_handle;
        self.next_handle += 1;
        self.open.insert(id, DocumentHandle {
            handle_id: id,
            type_id:   type_id.to_string(),
            path:      path.to_string(),
            dirty:     false,
        });
        Ok(id)
    }

    /// Close a document by handle ID.  Returns `false` if not found.
    pub fn close_document(&mut self, handle_id: u32) -> bool {
        self.open.remove(&handle_id).is_some()
    }

    /// Mark a document as dirty (unsaved changes).
    pub fn mark_dirty(&mut self, handle_id: u32) -> bool {
        if let Some(doc) = self.open.get_mut(&handle_id) {
            doc.dirty = true;
            true
        } else {
            false
        }
    }

    /// Get an open document by handle.
    pub fn get_open(&self, handle_id: u32) -> Option<&DocumentHandle> {
        self.open.get(&handle_id)
    }

    /// Number of registered document types.
    pub fn type_count(&self) -> usize { self.types.len() }

    /// Number of currently open documents.
    pub fn open_count(&self) -> usize { self.open.len() }
}

impl Default for DocumentRegistry {
    fn default() -> Self { Self::new() }
}

// ── Tests ────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn novaforge_defaults_registers_standard_types() {
        let reg = DocumentRegistry::with_novaforge_defaults();
        assert!(reg.get_type("scene").is_some());
        assert!(reg.get_type("material").is_some());
        assert!(reg.get_type("animation").is_some());
        assert!(reg.get_type("graph").is_some());
        assert!(reg.get_type("data_table").is_some());
        assert_eq!(reg.type_count(), 5);
    }

    #[test]
    fn register_type_duplicate_returns_false() {
        let mut reg = DocumentRegistry::new();
        let desc = DocumentTypeDescriptor::new("scene", "Scene", ["json"], "panel.scene");
        assert!(reg.register_type(desc.clone()));
        assert!(!reg.register_type(desc));
        assert_eq!(reg.type_count(), 1);
    }

    #[test]
    fn type_for_extension_finds_correct_type() {
        let reg = DocumentRegistry::with_novaforge_defaults();
        let t = reg.type_for_extension("anim").unwrap();
        assert_eq!(t.type_id, "animation");
    }

    #[test]
    fn open_and_close_document() {
        let mut reg = DocumentRegistry::with_novaforge_defaults();
        let handle = reg.open_document("scene", "worlds/test.json").unwrap();
        assert_eq!(reg.open_count(), 1);
        assert!(reg.close_document(handle));
        assert_eq!(reg.open_count(), 0);
    }

    #[test]
    fn open_unknown_type_returns_error() {
        let mut reg = DocumentRegistry::new();
        assert!(reg.open_document("no_such_type", "x.json").is_err());
    }

    #[test]
    fn mark_dirty_sets_flag() {
        let mut reg = DocumentRegistry::with_novaforge_defaults();
        let h = reg.open_document("scene", "a.json").unwrap();
        assert!(!reg.get_open(h).unwrap().dirty);
        assert!(reg.mark_dirty(h));
        assert!(reg.get_open(h).unwrap().dirty);
    }

    #[test]
    fn close_nonexistent_handle_returns_false() {
        let mut reg = DocumentRegistry::new();
        assert!(!reg.close_document(999));
    }

    #[test]
    fn handle_ids_are_monotonically_increasing() {
        let mut reg = DocumentRegistry::with_novaforge_defaults();
        let h1 = reg.open_document("scene", "a.json").unwrap();
        let h2 = reg.open_document("scene", "b.json").unwrap();
        assert!(h2 > h1);
    }
}
