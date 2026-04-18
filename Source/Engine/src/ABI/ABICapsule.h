#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace atlas::abi {

/// ABI version identifier
struct ABIVersion {
    uint32_t major = 0;
    uint32_t minor = 0;

    bool operator==(const ABIVersion& other) const;
    bool operator!=(const ABIVersion& other) const;
    bool operator<(const ABIVersion& other) const;

    /// "atlas_abi_v{major}_{minor}" string form
    std::string ToString() const;

    /// Parse from string like "atlas_abi_v1_0" or "atlas_abi_v1"
    static ABIVersion FromString(const std::string& str);

    /// Check if this version is compatible with another (same major)
    bool IsCompatibleWith(const ABIVersion& other) const;
};

/// C-compatible function table exposed by an ABI capsule.
/// No templates, no STL, no C++ name mangling in the actual ABI boundary.
/// This struct models what would be a C function pointer table.
struct ABIFunctionTable {
    uint32_t tableVersion = 0;

    // System lifecycle
    std::function<bool(const char* systemName)> RegisterSystem;
    std::function<void()> StepSimulation;
    std::function<uint64_t()> GetCurrentTick;

    // Component access
    std::function<bool(uint32_t entityId, uint32_t componentType, void* outData, uint32_t dataSize)> GetComponent;
    std::function<bool(uint32_t entityId, uint32_t componentType, const void* data, uint32_t dataSize)> SetComponent;

    // Input submission
    std::function<void(const void* inputData, uint32_t inputSize)> SubmitInput;

    // Event emission
    std::function<void(uint32_t eventType, const void* data, uint32_t dataSize)> EmitEvent;

    // Replay
    std::function<uint64_t()> GetWorldHash;

    /// Check if all required functions are bound
    bool IsComplete() const;

    /// Count of bound functions
    uint32_t BoundCount() const;
};

/// An ABI capsule wraps a frozen function table at a specific version.
/// Engine loads the capsule, binds current implementations to the frozen
/// interface, then hands the table to the project binary.
class ABICapsule {
public:
    ABICapsule() = default;
    ABICapsule(ABIVersion version, const std::string& description);

    /// Get the ABI version
    const ABIVersion& Version() const;

    /// Human-readable description
    const std::string& Description() const;

    /// Get the function table
    ABIFunctionTable& Functions();
    const ABIFunctionTable& Functions() const;

    /// Whether the capsule is fully initialized (all functions bound)
    bool IsReady() const;

    /// Seal the capsule — no further modifications to the function table
    void Seal();
    bool IsSealed() const;

private:
    ABIVersion m_version;
    std::string m_description;
    ABIFunctionTable m_functions;
    bool m_sealed = false;
};

} // namespace atlas::abi
