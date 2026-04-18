#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace atlas::mod {

struct ModDescriptor {
    std::string id;
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    std::vector<std::string> dependencies;
    std::string entryPath;
    bool enabled = false;
};

enum class ModLoadResult {
    Success,
    NotFound,
    InvalidDescriptor,
    MissingDependency,
    AlreadyLoaded,
};

class ModLoader {
public:
    // Register a mod from descriptor
    ModLoadResult RegisterMod(const ModDescriptor& descriptor);

    // Unregister a mod by id
    bool UnregisterMod(const std::string& id);

    // Enable / disable
    bool EnableMod(const std::string& id);
    bool DisableMod(const std::string& id);

    // Query
    bool HasMod(const std::string& id) const;
    const ModDescriptor* GetMod(const std::string& id) const;
    std::vector<std::string> EnabledMods() const;
    size_t ModCount() const;

    // Dependency validation
    bool ValidateDependencies(const std::string& id) const;
    std::vector<std::string> MissingDependencies(const std::string& id) const;

    // Scan a directory for mod descriptors (looks for mod.json files)
    size_t ScanDirectory(const std::string& dir);

    // Clear all mods
    void Clear();

private:
    std::unordered_map<std::string, ModDescriptor> m_mods;
};

}
