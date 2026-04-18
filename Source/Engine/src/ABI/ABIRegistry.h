#pragma once
#include "ABICapsule.h"
#include <memory>
#include <unordered_map>

namespace atlas::abi {

/// Project manifest's ABI targeting info
struct ProjectABITarget {
    std::string projectName;
    ABIVersion targetABI;
    std::string determinismProfile;
};

/// Manages ABI capsule versions and project binding
class ABIRegistry {
public:
    /// Register an ABI capsule
    void RegisterCapsule(std::shared_ptr<ABICapsule> capsule);

    /// Get a capsule by exact version
    std::shared_ptr<ABICapsule> GetCapsule(const ABIVersion& version) const;

    /// Find the best compatible capsule for the requested version.
    /// Returns capsule with same major version and highest minor <= requested.
    std::shared_ptr<ABICapsule> FindCompatible(const ABIVersion& requested) const;

    /// Check if a specific version is available
    bool HasVersion(const ABIVersion& version) const;

    /// Get all registered versions
    std::vector<ABIVersion> RegisteredVersions() const;

    /// Number of registered capsules
    size_t CapsuleCount() const;

    /// Bind a project to its target ABI capsule.
    /// Returns true if a compatible capsule was found and bound.
    bool BindProject(const ProjectABITarget& target);

    /// Get the capsule currently bound for a project
    std::shared_ptr<ABICapsule> GetProjectCapsule(const std::string& projectName) const;

    /// Check if a project is bound
    bool IsProjectBound(const std::string& projectName) const;

    /// Unbind a project
    void UnbindProject(const std::string& projectName);

    /// Get all bound project names
    std::vector<std::string> BoundProjects() const;

private:
    std::unordered_map<std::string, std::shared_ptr<ABICapsule>> m_capsules; // key: version string
    std::unordered_map<std::string, std::shared_ptr<ABICapsule>> m_bindings; // key: project name
};

} // namespace atlas::abi
