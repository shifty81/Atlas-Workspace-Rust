// SPDX-License-Identifier: GPL-3.0-only
// NovaForge document base — port of NovaForge::NovaForgeDocument.
// Copyright (C) NovaForge contributors. GNU General Public License v3.0.

// ── Document type taxonomy ────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum NovaForgeDocumentType {
    ItemDefinition = 0,
    StructureArchetype,
    BiomeDefinition,
    PlanetArchetype,
    FactionDefinition,
    MissionDefinition,
    ProgressionRules,
    CharacterRules,
    EconomyRules,
    CraftingDefinition,
    PcgRuleset,
    WorldDocument,
    LevelInstance,
    EntityTemplate,
    AssetDocument,
    MaterialDocument,
    AnimationDocument,
    EncounterTemplate,
    SpawnProfile,
    PcgContext,
    PcgPreset,
    VisualLogicGraph,
}

pub fn document_type_name(t: NovaForgeDocumentType) -> &'static str {
    match t {
        NovaForgeDocumentType::ItemDefinition     => "ItemDefinition",
        NovaForgeDocumentType::StructureArchetype => "StructureArchetype",
        NovaForgeDocumentType::BiomeDefinition    => "BiomeDefinition",
        NovaForgeDocumentType::PlanetArchetype    => "PlanetArchetype",
        NovaForgeDocumentType::FactionDefinition  => "FactionDefinition",
        NovaForgeDocumentType::MissionDefinition  => "MissionDefinition",
        NovaForgeDocumentType::ProgressionRules   => "ProgressionRules",
        NovaForgeDocumentType::CharacterRules     => "CharacterRules",
        NovaForgeDocumentType::EconomyRules       => "EconomyRules",
        NovaForgeDocumentType::CraftingDefinition => "CraftingDefinition",
        NovaForgeDocumentType::PcgRuleset         => "PCGRuleset",
        NovaForgeDocumentType::WorldDocument      => "WorldDocument",
        NovaForgeDocumentType::LevelInstance      => "LevelInstance",
        NovaForgeDocumentType::EntityTemplate     => "EntityTemplate",
        NovaForgeDocumentType::AssetDocument      => "AssetDocument",
        NovaForgeDocumentType::MaterialDocument   => "MaterialDocument",
        NovaForgeDocumentType::AnimationDocument  => "AnimationDocument",
        NovaForgeDocumentType::EncounterTemplate  => "EncounterTemplate",
        NovaForgeDocumentType::SpawnProfile       => "SpawnProfile",
        NovaForgeDocumentType::PcgContext         => "PCGContext",
        NovaForgeDocumentType::PcgPreset          => "PCGPreset",
        NovaForgeDocumentType::VisualLogicGraph   => "VisualLogicGraph",
    }
}

// ── Validation ────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum ValidationSeverity {
    Info    = 0,
    Warning = 1,
    Error   = 2,
}

#[derive(Debug, Clone)]
pub struct ValidationMessage {
    pub field:    String,
    pub message:  String,
    pub severity: ValidationSeverity,
}

#[derive(Debug, Clone)]
pub struct ValidationResult {
    pub valid:  bool,
    pub errors: Vec<ValidationMessage>,
}

impl ValidationResult {
    pub fn is_valid(&self) -> bool {
        !self.errors.iter().any(|m| m.severity == ValidationSeverity::Error)
    }
}

// ── NovaForgeDocument ────────────────────────────────────────────────────

pub struct NovaForgeDocument {
    doc_type:     NovaForgeDocumentType,
    file_path:    String,
    display_name: String,
    dirty:        bool,
}

impl NovaForgeDocument {
    pub fn new(doc_type: NovaForgeDocumentType, file_path: impl Into<String>) -> Self {
        Self {
            doc_type,
            file_path: file_path.into(),
            display_name: String::new(),
            dirty: false,
        }
    }

    pub fn doc_type(&self)     -> NovaForgeDocumentType { self.doc_type }
    pub fn file_path(&self)    -> &str                  { &self.file_path }
    pub fn display_name(&self) -> &str                  { &self.display_name }

    pub fn set_display_name(&mut self, name: impl Into<String>) {
        self.display_name = name.into();
    }

    pub fn is_dirty(&self)  -> bool { self.dirty }
    pub fn mark_dirty(&mut self)    { self.dirty = true; }
    pub fn clear_dirty(&mut self)   { self.dirty = false; }

    /// Override to provide document-type-specific validation.
    pub fn validate(&self) -> ValidationResult {
        ValidationResult { valid: true, errors: Vec::new() }
    }

    /// Validate → on_save() → clear dirty.
    pub fn save(&mut self) -> bool {
        let result = self.validate();
        if !result.is_valid() {
            return false;
        }
        self.on_save();
        self.clear_dirty();
        true
    }

    /// on_revert() → clear dirty.
    pub fn revert(&mut self) -> bool {
        self.on_revert();
        self.clear_dirty();
        true
    }

    /// Always succeeds for the base type.
    pub fn apply(&self) -> bool { true }

    // ── Overridable hooks ─────────────────────────────────────────────────

    fn on_save(&self)   {}
    fn on_revert(&self) {}
}

// ── Tests ─────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    fn make_doc() -> NovaForgeDocument {
        NovaForgeDocument::new(NovaForgeDocumentType::ItemDefinition, "items/sword.json")
    }

    #[test]
    fn new_returns_correct_type() {
        let doc = make_doc();
        assert_eq!(doc.doc_type(), NovaForgeDocumentType::ItemDefinition);
    }

    #[test]
    fn new_stores_file_path() {
        let doc = make_doc();
        assert_eq!(doc.file_path(), "items/sword.json");
    }

    #[test]
    fn dirty_starts_false() {
        let doc = make_doc();
        assert!(!doc.is_dirty());
    }

    #[test]
    fn mark_dirty_sets_flag() {
        let mut doc = make_doc();
        doc.mark_dirty();
        assert!(doc.is_dirty());
    }

    #[test]
    fn clear_dirty_clears_flag() {
        let mut doc = make_doc();
        doc.mark_dirty();
        doc.clear_dirty();
        assert!(!doc.is_dirty());
    }

    #[test]
    fn validate_returns_valid() {
        let doc = make_doc();
        let result = doc.validate();
        assert!(result.valid);
        assert!(result.is_valid());
        assert!(result.errors.is_empty());
    }

    #[test]
    fn save_clears_dirty() {
        let mut doc = make_doc();
        doc.mark_dirty();
        let ok = doc.save();
        assert!(ok);
        assert!(!doc.is_dirty());
    }

    #[test]
    fn revert_clears_dirty() {
        let mut doc = make_doc();
        doc.mark_dirty();
        let ok = doc.revert();
        assert!(ok);
        assert!(!doc.is_dirty());
    }

    #[test]
    fn document_type_name_roundtrip() {
        assert_eq!(document_type_name(NovaForgeDocumentType::PcgRuleset), "PCGRuleset");
        assert_eq!(document_type_name(NovaForgeDocumentType::VisualLogicGraph), "VisualLogicGraph");
    }

    #[test]
    fn validation_result_is_valid_ignores_warnings() {
        let result = ValidationResult {
            valid: true,
            errors: vec![ValidationMessage {
                field:    "x".into(),
                message:  "warn".into(),
                severity: ValidationSeverity::Warning,
            }],
        };
        assert!(result.is_valid());
    }

    #[test]
    fn validation_result_not_valid_with_error() {
        let result = ValidationResult {
            valid: false,
            errors: vec![ValidationMessage {
                field:    "path".into(),
                message:  "missing".into(),
                severity: ValidationSeverity::Error,
            }],
        };
        assert!(!result.is_valid());
    }
}
