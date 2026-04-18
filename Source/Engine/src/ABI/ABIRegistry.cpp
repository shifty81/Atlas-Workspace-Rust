#include "ABIRegistry.h"

namespace atlas::abi {

void ABIRegistry::RegisterCapsule(std::shared_ptr<ABICapsule> capsule) {
    if (!capsule)
        return;
    m_capsules[capsule->Version().ToString()] = std::move(capsule);
}

std::shared_ptr<ABICapsule> ABIRegistry::GetCapsule(const ABIVersion& version) const {
    auto it = m_capsules.find(version.ToString());
    if (it != m_capsules.end())
        return it->second;
    return nullptr;
}

std::shared_ptr<ABICapsule> ABIRegistry::FindCompatible(const ABIVersion& requested) const {
    std::shared_ptr<ABICapsule> best;

    for (const auto& [key, capsule] : m_capsules) {
        const ABIVersion& v = capsule->Version();
        if (v.major != requested.major)
            continue;
        if (v.minor > requested.minor)
            continue;
        if (!best || best->Version() < v)
            best = capsule;
    }

    return best;
}

bool ABIRegistry::HasVersion(const ABIVersion& version) const {
    return m_capsules.find(version.ToString()) != m_capsules.end();
}

std::vector<ABIVersion> ABIRegistry::RegisteredVersions() const {
    std::vector<ABIVersion> versions;
    versions.reserve(m_capsules.size());
    for (const auto& [key, capsule] : m_capsules)
        versions.push_back(capsule->Version());
    return versions;
}

size_t ABIRegistry::CapsuleCount() const {
    return m_capsules.size();
}

bool ABIRegistry::BindProject(const ProjectABITarget& target) {
    auto capsule = FindCompatible(target.targetABI);
    if (!capsule)
        return false;
    m_bindings[target.projectName] = std::move(capsule);
    return true;
}

std::shared_ptr<ABICapsule> ABIRegistry::GetProjectCapsule(const std::string& projectName) const {
    auto it = m_bindings.find(projectName);
    if (it != m_bindings.end())
        return it->second;
    return nullptr;
}

bool ABIRegistry::IsProjectBound(const std::string& projectName) const {
    return m_bindings.find(projectName) != m_bindings.end();
}

void ABIRegistry::UnbindProject(const std::string& projectName) {
    m_bindings.erase(projectName);
}

std::vector<std::string> ABIRegistry::BoundProjects() const {
    std::vector<std::string> names;
    names.reserve(m_bindings.size());
    for (const auto& [name, capsule] : m_bindings)
        names.push_back(name);
    return names;
}

} // namespace atlas::abi
