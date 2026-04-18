//! Delta edit store — records world edits layered on top of a PCG seed.
//!
//! Mirrors the C++ `atlas::ecs::DeltaEditStore`.

use serde::{Deserialize, Serialize};

/// Type of a single world edit.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize, Deserialize)]
#[repr(u8)]
pub enum DeltaEditType {
    AddObject    = 0,
    RemoveObject = 1,
    MoveObject   = 2,
    SetProperty  = 3,
}

impl DeltaEditType {
    pub fn name(self) -> &'static str {
        match self {
            Self::AddObject    => "AddObject",
            Self::RemoveObject => "RemoveObject",
            Self::MoveObject   => "MoveObject",
            Self::SetProperty  => "SetProperty",
        }
    }
}

/// A single recorded world edit.
#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
pub struct DeltaEdit {
    pub edit_type:      DeltaEditType,
    pub entity_id:      u32,
    pub object_type:    String,
    pub position:       [f32; 3],
    pub property_name:  String,
    pub property_value: String,
}

impl DeltaEdit {
    /// Construct an AddObject edit.
    pub fn add_object(entity_id: u32, object_type: impl Into<String>, pos: [f32; 3]) -> Self {
        Self {
            edit_type: DeltaEditType::AddObject,
            entity_id,
            object_type: object_type.into(),
            position: pos,
            property_name: String::new(),
            property_value: String::new(),
        }
    }

    /// Construct a RemoveObject edit.
    pub fn remove_object(entity_id: u32) -> Self {
        Self {
            edit_type: DeltaEditType::RemoveObject,
            entity_id,
            object_type: String::new(),
            position: [0.0; 3],
            property_name: String::new(),
            property_value: String::new(),
        }
    }

    /// Construct a MoveObject edit.
    pub fn move_object(entity_id: u32, pos: [f32; 3]) -> Self {
        Self {
            edit_type: DeltaEditType::MoveObject,
            entity_id,
            object_type: String::new(),
            position: pos,
            property_name: String::new(),
            property_value: String::new(),
        }
    }

    /// Construct a SetProperty edit.
    pub fn set_property(
        entity_id: u32,
        name: impl Into<String>,
        value: impl Into<String>,
    ) -> Self {
        Self {
            edit_type: DeltaEditType::SetProperty,
            entity_id,
            object_type: String::new(),
            position: [0.0; 3],
            property_name: name.into(),
            property_value: value.into(),
        }
    }
}

/// Persistent record of all world edits layered on top of a PCG seed.
///
/// Typical flow:
/// 1. PCG generates the world from a seed.
/// 2. Designer makes edits; each is [`record`](DeltaEditStore::record)ed.
/// 3. Save: `serialize_to_json(seed + edits)`.
/// 4. Load: `generate(seed)` then `apply_all(edits)`.
#[derive(Debug, Default, Serialize, Deserialize)]
pub struct DeltaEditStore {
    seed:  u64,
    edits: Vec<DeltaEdit>,
}

impl DeltaEditStore {
    pub fn new(seed: u64) -> Self {
        Self { seed, edits: Vec::new() }
    }

    pub fn seed(&self) -> u64 { self.seed }
    pub fn set_seed(&mut self, seed: u64) { self.seed = seed; }

    /// Append an edit.
    pub fn record(&mut self, edit: DeltaEdit) {
        self.edits.push(edit);
    }

    /// Number of recorded edits.
    pub fn count(&self) -> usize { self.edits.len() }

    /// Read-only access to all edits in recording order.
    pub fn edits(&self) -> &[DeltaEdit] { &self.edits }

    /// Remove all edits (seed is preserved).
    pub fn clear(&mut self) { self.edits.clear(); }

    /// Serialize to a JSON string.
    pub fn serialize_to_json(&self) -> Result<String, serde_json::Error> {
        serde_json::to_string_pretty(self)
    }

    /// Deserialize from a JSON string, replacing current contents.
    pub fn deserialize_from_json(&mut self, json: &str) -> Result<(), serde_json::Error> {
        let other: DeltaEditStore = serde_json::from_str(json)?;
        self.seed = other.seed;
        self.edits = other.edits;
        Ok(())
    }

    /// Save to a file.
    pub fn save_to_file(&self, path: &std::path::Path) -> std::io::Result<()> {
        if let Some(parent) = path.parent() {
            std::fs::create_dir_all(parent)?;
        }
        let json = self.serialize_to_json().map_err(|e| {
            std::io::Error::new(std::io::ErrorKind::Other, e.to_string())
        })?;
        std::fs::write(path, json)
    }

    /// Load from a file.
    pub fn load_from_file(&mut self, path: &std::path::Path) -> std::io::Result<()> {
        let json = std::fs::read_to_string(path)?;
        self.deserialize_from_json(&json)
            .map_err(|e| std::io::Error::new(std::io::ErrorKind::Other, e.to_string()))
    }
}
