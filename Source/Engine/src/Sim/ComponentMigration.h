#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <unordered_map>

namespace atlas::sim {

/// Describes a single field within a component layout
struct FieldDesc {
    uint32_t id = 0;       // Stable field identifier (never reused)
    uint32_t offset = 0;   // Byte offset within component
    uint32_t size = 0;     // Size in bytes
    enum class Type : uint8_t {
        Int32, Uint32, Float, Double, Bool, Vec2, Vec3, Vec4, Custom
    };
    Type type = Type::Int32;
    std::string name;
};

/// Schema describing a component's memory layout at a given version
struct ComponentSchema {
    uint32_t typeId = 0;
    uint32_t version = 0;
    uint32_t totalSize = 0;
    std::vector<FieldDesc> fields;

    const FieldDesc* FindField(uint32_t fieldId) const {
        for (const auto& f : fields) {
            if (f.id == fieldId) return &f;
        }
        return nullptr;
    }
};

/// Result of a migration operation
struct MigrationResult {
    bool success = false;
    uint32_t fieldsRemapped = 0;
    uint32_t fieldsAdded = 0;
    uint32_t fieldsDropped = 0;
    std::string error;
};

/// Remaps component data from an old schema to a new schema
/// Matching fields (by id) are copied; new fields are zero-initialized;
/// removed fields are dropped.
MigrationResult RemapComponent(
    const ComponentSchema& oldSchema,
    const ComponentSchema& newSchema,
    const void* oldData,
    void* newData);

/// Checks whether two schemas are compatible (same typeId)
bool SchemasCompatible(const ComponentSchema& a, const ComponentSchema& b);

/// Manages schema versions and performs world-level migration
class ComponentMigrationManager {
public:
    /// Register a schema version for a component type
    void RegisterSchema(const ComponentSchema& schema);

    /// Get the latest schema for a type
    const ComponentSchema* GetLatestSchema(uint32_t typeId) const;

    /// Get a specific version of a schema
    const ComponentSchema* GetSchema(uint32_t typeId, uint32_t version) const;

    /// Check if migration is needed between two versions
    bool NeedsMigration(uint32_t typeId, uint32_t fromVersion) const;

    /// Migrate a component blob from one version to the latest
    MigrationResult MigrateToLatest(
        uint32_t typeId,
        uint32_t fromVersion,
        const void* oldData,
        void* newData) const;

    /// Number of registered schemas
    size_t SchemaCount() const;

    /// Check if a hot reload migration is safe (no incompatible changes)
    bool IsHotReloadSafe(uint32_t typeId, uint32_t fromVersion) const;

private:
    // typeId -> (version -> schema)
    std::unordered_map<uint32_t, std::unordered_map<uint32_t, ComponentSchema>> m_schemas;
};

}  // namespace atlas::sim
