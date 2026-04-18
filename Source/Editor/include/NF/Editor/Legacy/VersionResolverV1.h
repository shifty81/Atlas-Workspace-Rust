#pragma once
// NF::Editor — Version resolver v1: semantic versioning, constraints, best-match resolution
#include "NF/Core/Core.h"
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

struct VersionSpec {
    uint32_t    major      = 0;
    uint32_t    minor      = 0;
    uint32_t    patch      = 0;
    std::string prerelease;

    [[nodiscard]] bool isValid() const { return true; }
    [[nodiscard]] std::string toString() const {
        std::string s = std::to_string(major) + "." +
                        std::to_string(minor) + "." +
                        std::to_string(patch);
        if (!prerelease.empty()) s += "-" + prerelease;
        return s;
    }
    bool operator<(const VersionSpec& o) const {
        if (major != o.major) return major < o.major;
        if (minor != o.minor) return minor < o.minor;
        return patch < o.patch;
    }
    bool operator==(const VersionSpec& o) const {
        return major == o.major && minor == o.minor && patch == o.patch &&
               prerelease == o.prerelease;
    }
    bool satisfies(const VersionSpec& required, bool allowPatch = true, bool allowMinor = false) const {
        if (major != required.major) return false;
        if (allowMinor) return minor >= required.minor;
        if (minor != required.minor) return false;
        if (allowPatch) return patch >= required.patch;
        return patch == required.patch;
    }
};

struct VrConstraint {
    std::string pkgName;
    VersionSpec minVersion;
    VersionSpec maxVersion;
};

class VersionResolverV1 {
public:
    void addConstraint(const std::string& pkgName,
                       const VersionSpec& minVer,
                       const VersionSpec& maxVer) {
        for (auto& c : m_constraints) {
            if (c.pkgName == pkgName) {
                c.minVersion = minVer;
                c.maxVersion = maxVer;
                return;
            }
        }
        m_constraints.push_back({pkgName, minVer, maxVer});
    }

    bool removeConstraint(const std::string& pkgName) {
        for (auto it = m_constraints.begin(); it != m_constraints.end(); ++it) {
            if (it->pkgName == pkgName) { m_constraints.erase(it); return true; }
        }
        return false;
    }

    VersionSpec resolve(const std::string& pkgName,
                        const std::vector<VersionSpec>& available) const {
        VersionSpec best;
        bool found = false;

        const VrConstraint* con = nullptr;
        for (const auto& c : m_constraints)
            if (c.pkgName == pkgName) { con = &c; break; }

        for (const auto& v : available) {
            if (con) {
                if (v < con->minVersion) continue;
                if (con->maxVersion < v) continue;
            }
            if (!found || best < v) { best = v; found = true; }
        }
        return best;
    }

    [[nodiscard]] size_t constraintCount() const { return m_constraints.size(); }

private:
    std::vector<VrConstraint> m_constraints;
};

} // namespace NF
