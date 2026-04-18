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

#[cfg(test)]
mod tests {
    use super::*;

    fn valid_schema() -> SchemaDefinition {
        SchemaDefinition {
            id: "MyGraph".into(),
            version: 1,
            inputs: vec![
                SchemaField { name: "x".into(), value_type: SchemaValueType::Float, required: true },
            ],
            outputs: vec![
                SchemaField { name: "result".into(), value_type: SchemaValueType::Float, required: false },
            ],
            nodes: vec![
                SchemaNodeDef { id: "node1".into(), inputs: vec![], outputs: vec![] },
            ],
        }
    }

    #[test]
    fn valid_schema_passes() {
        let mut v = SchemaValidator::new();
        assert!(v.validate(&valid_schema()));
        assert!(v.errors().is_empty());
    }

    #[test]
    fn empty_id_fails() {
        let mut v = SchemaValidator::new();
        let mut s = valid_schema();
        s.id = String::new();
        assert!(!v.validate(&s));
        assert!(!v.errors().is_empty());
    }

    #[test]
    fn negative_version_fails() {
        let mut v = SchemaValidator::new();
        let mut s = valid_schema();
        s.version = -1;
        assert!(!v.validate(&s));
    }

    #[test]
    fn duplicate_input_field_fails() {
        let mut v = SchemaValidator::new();
        let mut s = valid_schema();
        s.inputs.push(SchemaField { name: "x".into(), value_type: SchemaValueType::Int, required: false });
        assert!(!v.validate(&s));
    }

    #[test]
    fn duplicate_output_field_fails() {
        let mut v = SchemaValidator::new();
        let mut s = valid_schema();
        s.outputs.push(SchemaField { name: "result".into(), value_type: SchemaValueType::Bool, required: false });
        assert!(!v.validate(&s));
    }

    #[test]
    fn empty_node_id_fails() {
        let mut v = SchemaValidator::new();
        let mut s = valid_schema();
        s.nodes.push(SchemaNodeDef { id: String::new(), inputs: vec![], outputs: vec![] });
        assert!(!v.validate(&s));
    }

    #[test]
    fn clear_resets_errors() {
        let mut v = SchemaValidator::new();
        let mut s = valid_schema();
        s.id = String::new();
        v.validate(&s);
        assert!(!v.errors().is_empty());
        v.clear();
        assert!(v.errors().is_empty());
    }
}
