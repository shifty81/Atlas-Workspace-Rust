#include "schema/SchemaValidator.h"
#include <unordered_set>

namespace atlas::schema {

bool SchemaValidator::Validate(const SchemaDefinition& schema) {
    Clear();

    if (schema.id.empty()) {
        Error("Schema ID must not be empty");
    }

    if (schema.version <= 0) {
        Error("Schema version must be > 0");
    }

    std::unordered_set<std::string> nodeIds;
    for (const auto& node : schema.nodes) {
        if (node.id.empty()) {
            Error("Node ID must not be empty");
            continue;
        }
        if (!nodeIds.insert(node.id).second) {
            Error("Duplicate node ID: " + node.id);
        }
        for (const auto& field : node.inputs) {
            if (field.name.empty()) {
                Error("Field name must not be empty in node: " + node.id);
            }
        }
        for (const auto& field : node.outputs) {
            if (field.name.empty()) {
                Error("Field name must not be empty in node: " + node.id);
            }
        }
    }

    return m_errors.empty();
}

const std::vector<SchemaError>& SchemaValidator::GetErrors() const {
    return m_errors;
}

void SchemaValidator::Clear() {
    m_errors.clear();
}

void SchemaValidator::Error(const std::string& msg) {
    m_errors.push_back({msg});
}

}
