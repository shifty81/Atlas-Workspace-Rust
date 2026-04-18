#![allow(dead_code)]

#[derive(Debug, Clone, PartialEq)]
pub enum SchemaValueType { Int, Float, Bool, String_, Vec2, Vec3 }

#[derive(Debug, Clone)]
pub struct SchemaField {
    pub name: String,
    pub value_type: SchemaValueType,
    pub required: bool,
}

#[derive(Debug, Clone)]
pub struct SchemaNodeDef {
    pub id: String,
    pub inputs: Vec<SchemaField>,
    pub outputs: Vec<SchemaField>,
}

#[derive(Debug, Clone)]
pub struct SchemaDefinition {
    pub id: String,
    pub version: i32,
    pub inputs: Vec<SchemaField>,
    pub outputs: Vec<SchemaField>,
    pub nodes: Vec<SchemaNodeDef>,
}

#[derive(Debug, Clone)]
pub struct SchemaError {
    pub message: String,
}

#[derive(Debug, Default)]
pub struct SchemaValidator {
    errors: Vec<SchemaError>,
}

impl SchemaValidator {
    pub fn new() -> Self { Self::default() }

    pub fn validate(&mut self, schema: &SchemaDefinition) -> bool {
        self.errors.clear();
        if schema.id.is_empty() {
            self.errors.push(SchemaError { message: "Schema id must not be empty".into() });
        }
        if schema.version < 0 {
            self.errors.push(SchemaError { message: "Schema version must be >= 0".into() });
        }
        let mut seen = std::collections::HashSet::new();
        for f in &schema.inputs {
            if !seen.insert(f.name.clone()) {
                self.errors.push(SchemaError { message: format!("Duplicate input field: {}", f.name) });
            }
        }
        seen.clear();
        for f in &schema.outputs {
            if !seen.insert(f.name.clone()) {
                self.errors.push(SchemaError { message: format!("Duplicate output field: {}", f.name) });
            }
        }
        for node in &schema.nodes {
            if node.id.is_empty() {
                self.errors.push(SchemaError { message: "Node id must not be empty".into() });
            }
        }
        self.errors.is_empty()
    }

    pub fn errors(&self) -> &[SchemaError] { &self.errors }
    pub fn clear(&mut self) { self.errors.clear(); }
}
