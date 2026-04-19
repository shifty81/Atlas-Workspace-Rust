// SPDX-License-Identifier: GPL-3.0-only
// NovaForge document save pipeline — port of NovaForge::DocumentSavePipeline.
// Copyright (C) NovaForge contributors. GNU General Public License v3.0.

use std::fs;
use std::path::Path;

use crate::document::{NovaForgeDocument, ValidationMessage, ValidationSeverity};

// ── SaveResultStatus ──────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum SaveResultStatus {
    Ok               = 0,
    NoDocument       = 1,
    ValidationFailed = 2,
    WriteError       = 3,
}

// ── SaveResult ────────────────────────────────────────────────────────────

#[derive(Debug, Clone)]
pub struct SaveResult {
    pub status:   SaveResultStatus,
    pub messages: Vec<ValidationMessage>,
}

impl SaveResult {
    pub fn ok(&self) -> bool {
        self.status == SaveResultStatus::Ok
    }

    pub fn has_errors(&self) -> bool {
        self.messages.iter().any(|m| m.severity == ValidationSeverity::Error)
    }
}

// ── DocumentSavePipeline ──────────────────────────────────────────────────

pub struct DocumentSavePipeline {
    notify_callback: Option<Box<dyn Fn(&str, &str)>>,
}

impl DocumentSavePipeline {
    pub fn new() -> Self {
        Self { notify_callback: None }
    }

    pub fn set_notification_callback(&mut self, cb: impl Fn(&str, &str) + 'static) {
        self.notify_callback = Some(Box::new(cb));
    }

    /// Validate → write → clear dirty.
    pub fn save(&self, doc: &mut NovaForgeDocument) -> SaveResult {
        // 1. Validate
        let validation = doc.validate();
        let messages: Vec<ValidationMessage> = validation.errors.clone();

        let has_errors = messages.iter().any(|m| m.severity == ValidationSeverity::Error);
        if has_errors {
            self.notify("error", "[DocumentSavePipeline] Validation errors blocked save");
            return SaveResult {
                status:   SaveResultStatus::ValidationFailed,
                messages,
            };
        }

        // 2. Write document to file path
        if !self.write_document(doc) {
            self.notify("error", &format!("[DocumentSavePipeline] Write failed for '{}'", doc.file_path()));
            return SaveResult {
                status:   SaveResultStatus::WriteError,
                messages,
            };
        }

        // 3. Clear dirty
        doc.clear_dirty();

        self.notify("info", &format!("[DocumentSavePipeline] Saved '{}'", doc.file_path()));
        SaveResult {
            status:   SaveResultStatus::Ok,
            messages,
        }
    }

    /// Reload document from disk (calls revert on the document).
    pub fn revert(&self, doc: &mut NovaForgeDocument) -> bool {
        let ok = doc.revert();
        if ok {
            self.notify("info", &format!("[DocumentSavePipeline] Reverted '{}'", doc.file_path()));
        } else {
            self.notify("error", "[DocumentSavePipeline] Revert failed");
        }
        ok
    }

    /// Validate without writing.
    pub fn validate_only(&self, doc: &NovaForgeDocument) -> Vec<ValidationMessage> {
        let result = doc.validate();
        for m in &result.errors {
            match m.severity {
                ValidationSeverity::Error   => self.notify("error",   &format!("[Validation] {}: {}", m.field, m.message)),
                ValidationSeverity::Warning => self.notify("warning", &format!("[Validation] {}: {}", m.field, m.message)),
                ValidationSeverity::Info    => {}
            }
        }
        result.errors
    }

    // ── Private ───────────────────────────────────────────────────────────

    fn write_document(&self, doc: &NovaForgeDocument) -> bool {
        let file_path = doc.file_path();
        if file_path.is_empty() {
            return false;
        }
        let path = Path::new(file_path);

        // Ensure parent directory exists
        if let Some(parent) = path.parent() {
            if !parent.as_os_str().is_empty() {
                if let Err(_) = fs::create_dir_all(parent) {
                    return false;
                }
            }
        }

        let type_name = crate::document::document_type_name(doc.doc_type());
        let json = format!(
            "{{\"schema\":\"novaforge.document.v1\",\"type\":\"{}\",\"displayName\":\"{}\"}}",
            type_name,
            doc.display_name()
        );

        fs::write(path, json).is_ok()
    }

    fn notify(&self, severity: &str, message: &str) {
        if let Some(cb) = &self.notify_callback {
            cb(severity, message);
        }
    }
}

impl Default for DocumentSavePipeline {
    fn default() -> Self { Self::new() }
}

// ── Tests ─────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use crate::document::{NovaForgeDocument, NovaForgeDocumentType};
    use std::sync::{Arc, Mutex};

    fn make_pipeline() -> DocumentSavePipeline {
        DocumentSavePipeline::new()
    }

    #[test]
    fn save_ok_with_valid_doc_and_path() {
        let tmp = std::env::temp_dir().join("nf_save_test_ok.json");
        let path_str = tmp.to_str().unwrap().to_string();
        let mut doc = NovaForgeDocument::new(NovaForgeDocumentType::ItemDefinition, &path_str);
        doc.mark_dirty();

        let pipeline = make_pipeline();
        let result = pipeline.save(&mut doc);

        assert!(result.ok(), "expected Ok, got {:?}", result.status);
        assert!(!doc.is_dirty());
        assert!(tmp.exists());
        let _ = std::fs::remove_file(&tmp);
    }

    #[test]
    fn save_write_error_on_empty_path() {
        let mut doc = NovaForgeDocument::new(NovaForgeDocumentType::ItemDefinition, "");
        let pipeline = make_pipeline();
        let result = pipeline.save(&mut doc);
        assert_eq!(result.status, SaveResultStatus::WriteError);
    }

    #[test]
    fn revert_returns_true() {
        let mut doc = NovaForgeDocument::new(NovaForgeDocumentType::ItemDefinition, "x.json");
        doc.mark_dirty();
        let pipeline = make_pipeline();
        assert!(pipeline.revert(&mut doc));
        assert!(!doc.is_dirty());
    }

    #[test]
    fn validate_only_returns_messages() {
        let doc = NovaForgeDocument::new(NovaForgeDocumentType::WorldDocument, "world.json");
        let pipeline = make_pipeline();
        let msgs = pipeline.validate_only(&doc);
        assert!(msgs.is_empty(), "default doc should have no validation messages");
    }

    #[test]
    fn notification_fired_on_save() {
        let log: Arc<Mutex<Vec<String>>> = Arc::new(Mutex::new(Vec::new()));
        let log2 = log.clone();
        let mut pipeline = make_pipeline();
        pipeline.set_notification_callback(move |sev, msg| {
            log2.lock().unwrap().push(format!("[{}] {}", sev, msg));
        });

        let tmp = std::env::temp_dir().join("nf_save_test_notify.json");
        let path_str = tmp.to_str().unwrap().to_string();
        let mut doc = NovaForgeDocument::new(NovaForgeDocumentType::ItemDefinition, &path_str);
        let _ = pipeline.save(&mut doc);

        let entries = log.lock().unwrap();
        assert!(!entries.is_empty());
        let _ = std::fs::remove_file(&tmp);
    }

    #[test]
    fn notification_fired_on_write_error() {
        let log: Arc<Mutex<Vec<String>>> = Arc::new(Mutex::new(Vec::new()));
        let log2 = log.clone();
        let mut pipeline = make_pipeline();
        pipeline.set_notification_callback(move |sev, _| {
            log2.lock().unwrap().push(sev.to_string());
        });

        let mut doc = NovaForgeDocument::new(NovaForgeDocumentType::ItemDefinition, "");
        let _ = pipeline.save(&mut doc);

        let entries = log.lock().unwrap();
        assert!(entries.iter().any(|e| e == "error"));
    }

    #[test]
    fn save_creates_parent_dirs() {
        let tmp = std::env::temp_dir()
            .join("nf_save_test_subdir")
            .join("nested")
            .join("doc.json");
        let path_str = tmp.to_str().unwrap().to_string();
        let mut doc = NovaForgeDocument::new(NovaForgeDocumentType::LevelInstance, &path_str);
        let pipeline = make_pipeline();
        let result = pipeline.save(&mut doc);
        assert!(result.ok());
        assert!(tmp.exists());
        let _ = std::fs::remove_file(&tmp);
        let _ = std::fs::remove_dir_all(std::env::temp_dir().join("nf_save_test_subdir"));
    }

    #[test]
    fn save_result_ok_method() {
        let r = SaveResult { status: SaveResultStatus::Ok, messages: vec![] };
        assert!(r.ok());
        let r2 = SaveResult { status: SaveResultStatus::WriteError, messages: vec![] };
        assert!(!r2.ok());
    }
}
