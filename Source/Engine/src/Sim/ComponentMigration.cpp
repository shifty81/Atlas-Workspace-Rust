#include "ComponentMigration.h"
#include <algorithm>

namespace atlas::sim {

MigrationResult RemapComponent(
    const ComponentSchema& oldSchema,
    const ComponentSchema& newSchema,
    const void* oldData,
    void* newData) {

    MigrationResult result;

    if (oldSchema.typeId != newSchema.typeId) {
        result.error = "Type ID mismatch";
        return result;
    }

    if (!oldData || !newData) {
        result.error = "Null data pointer";
        return result;
    }

    // Zero-initialize output
    std::memset(newData, 0, newSchema.totalSize);

    for (const auto& newField : newSchema.fields) {
        const FieldDesc* oldField = oldSchema.FindField(newField.id);
        if (oldField) {
            // Copy the smaller of the two sizes to avoid overflow
            uint32_t copySize = std::min(newField.size, oldField->size);
            // Bounds check: ensure offsets + size don't exceed buffer sizes
            if (newField.offset + copySize > newSchema.totalSize ||
                oldField->offset + copySize > oldSchema.totalSize) {
                result.error = "Field offset exceeds schema size";
                result.success = false;
                return result;
            }
            std::memcpy(
                static_cast<uint8_t*>(newData) + newField.offset,
                static_cast<const uint8_t*>(oldData) + oldField->offset,
                copySize);
            result.fieldsRemapped++;
        } else {
            // New field: already zero-initialized
            result.fieldsAdded++;
        }
    }

    // Count dropped fields
    for (const auto& oldField : oldSchema.fields) {
        if (!newSchema.FindField(oldField.id)) {
            result.fieldsDropped++;
        }
    }

    result.success = true;
    return result;
}

bool SchemasCompatible(const ComponentSchema& a, const ComponentSchema& b) {
    return a.typeId == b.typeId;
}

// ---- ComponentMigrationManager ----

void ComponentMigrationManager::RegisterSchema(const ComponentSchema& schema) {
    m_schemas[schema.typeId][schema.version] = schema;
}

const ComponentSchema* ComponentMigrationManager::GetLatestSchema(uint32_t typeId) const {
    auto it = m_schemas.find(typeId);
    if (it == m_schemas.end() || it->second.empty()) return nullptr;

    const ComponentSchema* latest = nullptr;
    uint32_t maxVer = 0;
    for (const auto& [ver, schema] : it->second) {
        if (ver >= maxVer) {
            maxVer = ver;
            latest = &schema;
        }
    }
    return latest;
}

const ComponentSchema* ComponentMigrationManager::GetSchema(
    uint32_t typeId, uint32_t version) const {
    auto typeIt = m_schemas.find(typeId);
    if (typeIt == m_schemas.end()) return nullptr;
    auto verIt = typeIt->second.find(version);
    if (verIt == typeIt->second.end()) return nullptr;
    return &verIt->second;
}

bool ComponentMigrationManager::NeedsMigration(
    uint32_t typeId, uint32_t fromVersion) const {
    const ComponentSchema* latest = GetLatestSchema(typeId);
    if (!latest) return false;
    return latest->version != fromVersion;
}

MigrationResult ComponentMigrationManager::MigrateToLatest(
    uint32_t typeId,
    uint32_t fromVersion,
    const void* oldData,
    void* newData) const {

    const ComponentSchema* oldSchema = GetSchema(typeId, fromVersion);
    const ComponentSchema* newSchema = GetLatestSchema(typeId);

    if (!oldSchema) {
        MigrationResult r;
        r.error = "Source schema not found";
        return r;
    }
    if (!newSchema) {
        MigrationResult r;
        r.error = "Target schema not found";
        return r;
    }
    if (oldSchema->version == newSchema->version) {
        // No migration needed; just copy (use smaller size for safety)
        uint32_t copySize = std::min(oldSchema->totalSize, newSchema->totalSize);
        std::memcpy(newData, oldData, copySize);
        MigrationResult r;
        r.success = true;
        return r;
    }

    return RemapComponent(*oldSchema, *newSchema, oldData, newData);
}

size_t ComponentMigrationManager::SchemaCount() const {
    size_t count = 0;
    for (const auto& [typeId, versions] : m_schemas) {
        count += versions.size();
    }
    return count;
}

bool ComponentMigrationManager::IsHotReloadSafe(
    uint32_t typeId, uint32_t fromVersion) const {
    const ComponentSchema* oldSchema = GetSchema(typeId, fromVersion);
    const ComponentSchema* newSchema = GetLatestSchema(typeId);
    if (!oldSchema || !newSchema) return false;
    if (oldSchema->version == newSchema->version) return true;

    // Safe if all old fields still exist in the new schema
    for (const auto& oldField : oldSchema->fields) {
        const FieldDesc* match = newSchema->FindField(oldField.id);
        if (!match) continue; // Dropping fields is acceptable
        if (match->type != oldField.type) return false; // Type change = unsafe
    }
    return true;
}

}  // namespace atlas::sim
