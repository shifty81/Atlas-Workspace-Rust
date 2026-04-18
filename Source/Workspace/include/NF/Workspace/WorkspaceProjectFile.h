#pragma once
// NF::Workspace — WorkspaceProjectFile: canonical .atlasproject file schema.
//
// A WorkspaceProjectFile is the in-memory representation of a .atlasproject file.
// It contains all data needed to restore a workspace session: project identity,
// named sections of key-value pairs, and a schema version for forward/backward
// compatibility.
//
// Design goals:
//   - No filesystem I/O (callers read/write strings; this class owns the schema)
//   - Deterministic serialization (write → parse round-trip is lossless)
//   - Schema versioning so readers can detect incompatible files gracefully
//   - Named sections for extensibility (Core, Assets, Layout, Settings, …)
//
// Components:
//   ProjectFileVersion  — major.minor.patch schema version
//   ProjectFileSection  — named group of key-value string pairs
//   WorkspaceProjectFile — root document: version + project identity + sections
//
// Wire format (text):
//   #atlasproject:<major>.<minor>.<patch>
//   project.id=<uuid-string>
//   project.name=<display-name>
//   project.contentRoot=<path>
//   [SectionName]
//   key=value
//   …

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace NF {

// ── Project File Version ───────────────────────────────────────────

struct ProjectFileVersion {
    uint32_t major = 1;
    uint32_t minor = 0;
    uint32_t patch = 0;

    [[nodiscard]] std::string toString() const {
        return std::to_string(major) + "."
             + std::to_string(minor) + "."
             + std::to_string(patch);
    }

    [[nodiscard]] bool isCompatible(const ProjectFileVersion& reader) const {
        // Major version must match; file minor <= reader minor is fine
        // (i.e. a reader at 1.2.x can read files written at 1.0.x).
        return major == reader.major && minor <= reader.minor;
    }

    [[nodiscard]] bool operator==(const ProjectFileVersion& o) const {
        return major == o.major && minor == o.minor && patch == o.patch;
    }
    [[nodiscard]] bool operator!=(const ProjectFileVersion& o) const { return !(*this == o); }

    static ProjectFileVersion current() { return {1, 0, 0}; }

    static bool parse(const std::string& s, ProjectFileVersion& out) {
        // Expects "major.minor.patch"
        uint32_t ma = 0, mi = 0, pa = 0;
        if (std::sscanf(s.c_str(), "%u.%u.%u", &ma, &mi, &pa) != 3) return false;
        out = {ma, mi, pa};
        return true;
    }
};

// ── Project File Section ───────────────────────────────────────────
// Named group of string key-value pairs within the project file.

class ProjectFileSection {
public:
    static constexpr size_t MAX_ENTRIES = 256;

    explicit ProjectFileSection(std::string name) : m_name(std::move(name)) {}
    ProjectFileSection() = default;

    [[nodiscard]] const std::string& name() const { return m_name; }

    void set(const std::string& key, const std::string& value) {
        for (auto& [k, v] : m_entries) {
            if (k == key) { v = value; return; }
        }
        if (m_entries.size() < MAX_ENTRIES)
            m_entries.push_back({key, value});
    }

    [[nodiscard]] const std::string* get(const std::string& key) const {
        for (const auto& [k, v] : m_entries)
            if (k == key) return &v;
        return nullptr;
    }

    [[nodiscard]] std::string getOr(const std::string& key, const std::string& def) const {
        const auto* v = get(key);
        return v ? *v : def;
    }

    [[nodiscard]] bool has(const std::string& key) const { return get(key) != nullptr; }

    bool remove(const std::string& key) {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->first == key) { m_entries.erase(it); return true; }
        }
        return false;
    }

    void clear() { m_entries.clear(); }

    [[nodiscard]] size_t count()  const { return m_entries.size(); }
    [[nodiscard]] bool   empty()  const { return m_entries.empty(); }

    [[nodiscard]] const std::vector<std::pair<std::string, std::string>>& entries() const {
        return m_entries;
    }

private:
    std::string m_name;
    std::vector<std::pair<std::string, std::string>> m_entries;
};

// ── Workspace Project File ─────────────────────────────────────────

class WorkspaceProjectFile {
public:
    static constexpr size_t MAX_SECTIONS = 64;
    static constexpr const char* FILE_MAGIC = "#atlasproject";

    // ── Identity ──────────────────────────────────────────────────

    void setProjectId(const std::string& id)          { m_projectId   = id;   }
    void setProjectName(const std::string& name)       { m_projectName = name; }
    void setContentRoot(const std::string& path)       { m_contentRoot = path; }
    void setVersion(const ProjectFileVersion& ver)     { m_version     = ver;  }

    [[nodiscard]] const std::string&       projectId()   const { return m_projectId;   }
    [[nodiscard]] const std::string&       projectName() const { return m_projectName; }
    [[nodiscard]] const std::string&       contentRoot() const { return m_contentRoot; }
    [[nodiscard]] const ProjectFileVersion& version()    const { return m_version;     }

    // ── Section management ────────────────────────────────────────

    // Returns an existing section or creates a new one.
    ProjectFileSection& section(const std::string& name) {
        for (auto& s : m_sections)
            if (s.name() == name) return s;
        if (m_sections.size() < MAX_SECTIONS) {
            m_sections.emplace_back(name);
            return m_sections.back();
        }
        // Safety: return last section if full
        return m_sections.back();
    }

    [[nodiscard]] const ProjectFileSection* findSection(const std::string& name) const {
        for (const auto& s : m_sections)
            if (s.name() == name) return &s;
        return nullptr;
    }

    [[nodiscard]] bool hasSection(const std::string& name) const {
        return findSection(name) != nullptr;
    }

    bool removeSection(const std::string& name) {
        for (auto it = m_sections.begin(); it != m_sections.end(); ++it) {
            if (it->name() == name) { m_sections.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t sectionCount() const { return m_sections.size(); }
    [[nodiscard]] const std::vector<ProjectFileSection>& sections() const { return m_sections; }

    // ── Validation ────────────────────────────────────────────────

    [[nodiscard]] bool isValid() const {
        return !m_projectId.empty()
            && !m_projectName.empty()
            && m_version.major > 0;
    }

    // ── Serialization ─────────────────────────────────────────────

    [[nodiscard]] std::string serialize() const {
        std::ostringstream out;
        out << FILE_MAGIC << ":" << m_version.toString() << "\n";
        out << "project.id="          << m_projectId   << "\n";
        out << "project.name="        << m_projectName << "\n";
        out << "project.contentRoot=" << m_contentRoot << "\n";

        for (const auto& sec : m_sections) {
            out << "[" << sec.name() << "]\n";
            for (const auto& [k, v] : sec.entries()) {
                out << k << "=" << v << "\n";
            }
        }
        return out.str();
    }

    // Parse returns false and leaves |out| in an undefined state on error.
    static bool parse(const std::string& data, WorkspaceProjectFile& out) {
        if (data.empty()) return false;
        std::istringstream in(data);
        std::string line;
        out = {};
        bool gotMagic = false;
        ProjectFileSection* current = nullptr;

        while (std::getline(in, line)) {
            if (line.empty()) continue;

            // Magic/version header
            if (!gotMagic) {
                if (line.rfind(FILE_MAGIC, 0) != 0) return false;
                auto colonPos = line.find(':');
                if (colonPos == std::string::npos) return false;
                ProjectFileVersion ver;
                if (!ProjectFileVersion::parse(line.substr(colonPos + 1), ver)) return false;
                out.m_version = ver;
                gotMagic = true;
                continue;
            }

            // Section header [Name]
            if (line.front() == '[' && line.back() == ']') {
                std::string secName = line.substr(1, line.size() - 2);
                current = &out.section(secName);
                continue;
            }

            // Key=Value pair
            auto eq = line.find('=');
            if (eq == std::string::npos) continue;
            std::string key   = line.substr(0, eq);
            std::string value = line.substr(eq + 1);

            if (!current) {
                // Top-level identity keys
                if      (key == "project.id")          out.m_projectId   = value;
                else if (key == "project.name")        out.m_projectName = value;
                else if (key == "project.contentRoot") out.m_contentRoot = value;
            } else {
                current->set(key, value);
            }
        }
        return gotMagic;
    }

private:
    ProjectFileVersion m_version = ProjectFileVersion::current();
    std::string m_projectId;
    std::string m_projectName;
    std::string m_contentRoot;
    std::vector<ProjectFileSection> m_sections;
};

} // namespace NF
