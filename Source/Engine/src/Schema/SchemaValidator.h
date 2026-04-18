#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::schema {

enum class SchemaValueType : uint8_t {
    Int,
    Float,
    Bool,
    String,
    Vec2,
    Vec3
};

struct SchemaField {
    std::string name;
    SchemaValueType type;
    bool required = true;
};

struct SchemaNodeDef {
    std::string id;
    std::vector<SchemaField> inputs;
    std::vector<SchemaField> outputs;
};

struct SchemaDefinition {
    std::string id;
    int version = 0;
    std::vector<SchemaField> inputs;
    std::vector<SchemaField> outputs;
    std::vector<SchemaNodeDef> nodes;
};

struct SchemaError {
    std::string message;
};

class SchemaValidator {
public:
    bool Validate(const SchemaDefinition& schema);
    const std::vector<SchemaError>& GetErrors() const;
    void Clear();
private:
    std::vector<SchemaError> m_errors;
    void Error(const std::string& msg);
};

}
