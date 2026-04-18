#pragma once
// NF::Workspace — AtlasProjectFileLoader: lightweight .atlas project file parser.
//
// An .atlas file is a JSON document that conforms to the atlas.project.v1 schema.
// This parser extracts the fields needed to boot a workspace session without
// depending on a heavyweight JSON library. It uses a minimal line-oriented
// tokeniser that handles the flat/nested structure of the schema.
//
// Key fields extracted:
//   schema      — must equal "atlas.project.v1" for the file to be accepted
//   name        — human-readable project name
//   version     — semver string
//   adapter     — IGameProjectAdapter factory ID (e.g. "novaforge")
//   capabilities— list of capability strings
//   content     — content root path (from modules.content)
//   entryWorld  — entry world path (from runtime.entryWorld)
//   tickRate    — simulation tick rate (from runtime.tickRate)
//   maxPlayers  — max player count (from runtime.maxPlayers)
//
// Usage:
//   AtlasProjectFileLoader loader;
//   if (loader.loadFromFile("path/to/NovaForge.atlas")) {
//       const AtlasProjectManifest& m = loader.manifest();
//       registry.loadProject(m.adapterId);
//   }

#include "NF/Workspace/ProjectLoadContract.h"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace NF {

// ── AtlasProjectManifest ──────────────────────────────────────────────────
// In-memory representation of an .atlas project file.

struct AtlasProjectManifest {
    std::string name;
    std::string version;
    std::string description;
    std::string adapterId;         // "adapter" field — maps to a ProjectRegistry factory

    // capabilities array
    std::vector<std::string> capabilities;

    // modules section
    std::string contentRoot;       // modules.content
    std::string worldGraph;        // modules.worldGraph
    bool        aiEnabled = false; // modules.ai

    // runtime section
    std::string entryWorld;        // runtime.entryWorld
    int         tickRate   = 30;   // runtime.tickRate
    int         maxPlayers = 20;   // runtime.maxPlayers

    // assets section
    std::string assetsRoot;        // assets.root

    // ── v2 manifest fields (new format) ─────────────────────────
    std::string projectId;        // top-level "projectId"
    int         projectVersion = 0; // top-level "projectVersion"

    // roots section (v2)
    std::string dataRoot;         // roots.data
    std::string configRoot;       // roots.config
    std::string schemasRoot;      // roots.schemas
    std::string generatedRoot;    // roots.generated
    std::string cacheRoot;        // roots.cache

    // registries section (v2)
    std::string registryAsset;    // registries.asset
    std::string registryDocument; // registries.document
    std::string registrySchema;   // registries.schema
    std::string registryPCG;      // registries.pcg
    std::string registryBuild;    // registries.build
    std::string registryLaunch;   // registries.launch

    // startup section (v2)
    std::string startupLayout;    // startup.layout
    std::string startupTool;      // startup.tool
    std::string startupWorld;     // startup.world

    // config section (v2)
    std::string configProjectSettings;    // config.projectSettings
    std::string configPIESettings;        // config.pieSettings
    std::string configPCGPreviewDefaults; // config.pcgPreviewDefaults

    [[nodiscard]] bool isValid() const {
        if (name.empty()) return false;
        // v1 requires a version string; v2 uses projectVersion integer or projectId
        return !version.empty() || projectVersion > 0 || !projectId.empty();
    }

    [[nodiscard]] bool hasAdapter() const {
        return !adapterId.empty();
    }
};

// ── ProjectBootstrapResult ────────────────────────────────────────────────
// Produced by AtlasProjectFileLoader::bootstrap() after validating the manifest.
// Carries the parsed manifest plus structured validation entries.

struct ProjectBootstrapResult {
    AtlasProjectManifest manifest;
    std::vector<ProjectValidationEntry> validationEntries;
    bool success = false;

    [[nodiscard]] bool hasErrors() const {
        for (const auto& e : validationEntries)
            if (e.severity == ProjectValidationSeverity::Fatal ||
                e.severity == ProjectValidationSeverity::Error)
                return true;
        return false;
    }
};

// ── AtlasProjectFileLoader ────────────────────────────────────────────────
// Stateless parser; call loadFromText() or loadFromFile() then read manifest().
//
// Parser design notes:
//   - No third-party JSON library dependency.
//   - Handles the specific flat/nested structure of atlas.project.v1.
//   - Accepts well-formed JSON only; malformed files return false.
//   - All string values are unquoted (surrounding double-quotes stripped).
//   - Unknown keys are silently ignored (forward-compatibility).

class AtlasProjectFileLoader {
public:
    // Load from a file path.  Returns true on success.
    [[nodiscard]] bool loadFromFile(const std::string& path) {
        std::ifstream f(path);
        if (!f.is_open()) return false;
        std::ostringstream buf;
        buf << f.rdbuf();
        return loadFromText(buf.str(), path);
    }

    // Load from a JSON string.  |sourceHint| is used in error messages only.
    [[nodiscard]] bool loadFromText(const std::string& text,
                                    const std::string& sourceHint = "") {
        m_manifest  = {};
        m_error     = {};
        m_sourcePath = sourceHint;

        if (text.empty()) {
            m_error = "empty input";
            return false;
        }

        // Tokenise into a flat list of key-value pairs by scanning the JSON
        // text line by line.  This handles the two-level nesting (top-level
        // keys and one level of object/array nesting) present in atlas.project.v1.

        std::istringstream ss(text);
        std::string line;

        std::string currentSection; // e.g. "modules", "runtime", "assets", "capabilities"
        bool        inArray = false;

        while (std::getline(ss, line)) {
            // Strip leading/trailing whitespace
            auto stripped = trim(line);
            if (stripped.empty()) continue;

            // Detect section entry: "key": { or "key": [
            {
                auto kv = tryParseObjectKey(stripped);
                if (!kv.first.empty()) {
                    if (kv.second == "{") {
                        currentSection = kv.first;
                        inArray = false;
                        continue;
                    }
                    if (kv.second == "[") {
                        currentSection = kv.first;
                        inArray = true;
                        continue;
                    }
                }
            }

            // Detect section close
            if (stripped == "}" || stripped == "}," ||
                stripped == "]" || stripped == "],") {
                currentSection.clear();
                inArray = false;
                continue;
            }

            // Inside an array (capabilities): collect bare string values
            if (inArray && currentSection == "capabilities") {
                auto val = extractStringValue(stripped);
                if (!val.empty())
                    m_manifest.capabilities.push_back(val);
                continue;
            }

            // Parse "key": value or "key": "value"
            auto [key, value] = parseKeyValue(stripped);
            if (key.empty()) continue;

            if (currentSection.empty()) {
                // Top-level fields
                applyTopLevel(key, value);
            } else if (currentSection == "modules") {
                applyModules(key, value);
            } else if (currentSection == "runtime") {
                applyRuntime(key, value);
            } else if (currentSection == "assets") {
                applyAssets(key, value);
            } else if (currentSection == "roots") {
                applyRoots(key, value);
            } else if (currentSection == "registries") {
                applyRegistries(key, value);
            } else if (currentSection == "startup") {
                applyStartup(key, value);
            } else if (currentSection == "config") {
                applyConfigSection(key, value);
            }
            // Other sections (buildProfiles, etc.) are ignored
        }

        // Accept old format (schema field) OR new v2 format (projectId present)
        bool validFormat = (m_schemaValue == "atlas.project.v1") || !m_manifest.projectId.empty();
        if (!validFormat) {
            m_error = "schema field missing or not 'atlas.project.v1' (got '"
                    + m_schemaValue + "') and no projectId found in " + sourceHint;
            return false;
        }

        if (!m_manifest.isValid()) {
            m_error = "manifest is missing required 'name' or 'version' in " + sourceHint;
            return false;
        }

        return true;
    }

    // ── Accessors ─────────────────────────────────────────────────

    [[nodiscard]] const AtlasProjectManifest& manifest()    const { return m_manifest;   }
    [[nodiscard]] const std::string&          error()       const { return m_error;       }
    [[nodiscard]] const std::string&          sourcePath()  const { return m_sourcePath;  }
    [[nodiscard]] bool                        succeeded()   const { return m_error.empty() && m_manifest.isValid(); }

    // Parse and validate a .atlas file. Returns a ProjectBootstrapResult with
    // the manifest and any validation entries. When checkPathsOnDisk is true,
    // verifies that contentRoot and assetsRoot paths exist on disk.
    [[nodiscard]] ProjectBootstrapResult bootstrap(const std::string& path,
                                                    bool checkPathsOnDisk = false) {
        ProjectBootstrapResult result;

        if (!loadFromFile(path)) {
            result.validationEntries.push_back({
                ProjectValidationSeverity::Fatal,
                "parse_failure",
                "Failed to parse .atlas file: " + (m_error.empty() ? path : m_error)
            });
            result.success = false;
            return result;
        }

        result.manifest = m_manifest;

        if (result.manifest.contentRoot.empty()) {
            result.validationEntries.push_back({
                ProjectValidationSeverity::Warning,
                "empty_content_root",
                "modules.content is empty in " + path
            });
        }

        if (result.manifest.assetsRoot.empty()) {
            result.validationEntries.push_back({
                ProjectValidationSeverity::Warning,
                "empty_assets_root",
                "assets.root is empty in " + path
            });
        }

        if (result.manifest.adapterId.empty()) {
            result.validationEntries.push_back({
                ProjectValidationSeverity::Error,
                "missing_adapter_id",
                "adapter field is missing or empty in " + path
            });
        }

        if (checkPathsOnDisk) {
            namespace fs = std::filesystem;
            if (!result.manifest.contentRoot.empty() &&
                !fs::exists(result.manifest.contentRoot)) {
                result.validationEntries.push_back({
                    ProjectValidationSeverity::Error,
                    "missing_content_root",
                    "contentRoot path does not exist on disk: " + result.manifest.contentRoot
                });
            }
            if (!result.manifest.assetsRoot.empty() &&
                !fs::exists(result.manifest.assetsRoot)) {
                result.validationEntries.push_back({
                    ProjectValidationSeverity::Error,
                    "missing_assets_root",
                    "assetsRoot path does not exist on disk: " + result.manifest.assetsRoot
                });
            }
        }

        result.success = !result.hasErrors();
        return result;
    }

private:
    AtlasProjectManifest m_manifest;
    std::string          m_error;
    std::string          m_sourcePath;
    std::string          m_schemaValue;

    // ── Field appliers ─────────────────────────────────────────────

    void applyTopLevel(const std::string& key, const std::string& value) {
        if      (key == "schema")        m_schemaValue          = value;
        else if (key == "name")          m_manifest.name        = value;
        else if (key == "version")       m_manifest.version     = value;
        else if (key == "description")   m_manifest.description = value;
        else if (key == "adapter")       m_manifest.adapterId   = value;
        else if (key == "projectId")      { m_manifest.projectId = value; if (m_manifest.name.empty()) m_manifest.name = value; }
        else if (key == "projectName")    { m_manifest.name = value; }
        else if (key == "projectType")    { if (m_manifest.adapterId.empty()) m_manifest.adapterId = toLower(value); }
        else if (key == "projectVersion") { m_manifest.projectVersion = parseIntOr(value, 0); }
    }

    void applyModules(const std::string& key, const std::string& value) {
        if      (key == "content")    m_manifest.contentRoot = value;
        else if (key == "worldGraph") m_manifest.worldGraph  = value;
        else if (key == "ai")         m_manifest.aiEnabled   = (value == "true");
    }

    void applyRuntime(const std::string& key, const std::string& value) {
        if      (key == "entryWorld")  m_manifest.entryWorld  = value;
        else if (key == "tickRate")    m_manifest.tickRate    = parseIntOr(value, 30);
        else if (key == "maxPlayers")  m_manifest.maxPlayers  = parseIntOr(value, 20);
    }

    void applyAssets(const std::string& key, const std::string& value) {
        if (key == "root") m_manifest.assetsRoot = value;
    }

    void applyRoots(const std::string& key, const std::string& value) {
        if      (key == "content")   m_manifest.contentRoot   = value;
        else if (key == "data")      m_manifest.dataRoot      = value;
        else if (key == "config")    m_manifest.configRoot    = value;
        else if (key == "schemas")   m_manifest.schemasRoot   = value;
        else if (key == "generated") m_manifest.generatedRoot = value;
        else if (key == "cache")     m_manifest.cacheRoot     = value;
    }

    void applyRegistries(const std::string& key, const std::string& value) {
        if      (key == "asset")    m_manifest.registryAsset    = value;
        else if (key == "document") m_manifest.registryDocument = value;
        else if (key == "schema")   m_manifest.registrySchema   = value;
        else if (key == "pcg")      m_manifest.registryPCG      = value;
        else if (key == "build")    m_manifest.registryBuild    = value;
        else if (key == "launch")   m_manifest.registryLaunch   = value;
    }

    void applyStartup(const std::string& key, const std::string& value) {
        if      (key == "layout") m_manifest.startupLayout = value;
        else if (key == "tool")   m_manifest.startupTool   = value;
        else if (key == "world")  m_manifest.startupWorld  = value;
    }

    void applyConfigSection(const std::string& key, const std::string& value) {
        if      (key == "projectSettings")    m_manifest.configProjectSettings    = value;
        else if (key == "pieSettings")        m_manifest.configPIESettings        = value;
        else if (key == "pcgPreviewDefaults") m_manifest.configPCGPreviewDefaults = value;
    }

    // ── JSON micro-parsers ─────────────────────────────────────────

    // Strips leading/trailing ASCII whitespace.
    static std::string trim(const std::string& s) {
        static const char* ws = " \t\r\n";
        auto b = s.find_first_not_of(ws);
        if (b == std::string::npos) return {};
        auto e = s.find_last_not_of(ws);
        return s.substr(b, e - b + 1);
    }

    // Strips a trailing comma from the string.
    static std::string stripTrailingComma(const std::string& s) {
        if (!s.empty() && s.back() == ',')
            return s.substr(0, s.size() - 1);
        return s;
    }

    // Strips surrounding double-quotes (if present).
    static std::string unquote(const std::string& s) {
        if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
            return s.substr(1, s.size() - 2);
        return s;
    }

    // Extracts a string value from a bare array element line (e.g. `"Rendering3D",`).
    static std::string extractStringValue(const std::string& s) {
        auto stripped = stripTrailingComma(trim(s));
        return unquote(stripped);
    }

    // Tries to detect `"key": {` or `"key": [` patterns.
    // Returns {key, "{" or "[" or ""} — empty key means not matched.
    static std::pair<std::string, std::string> tryParseObjectKey(const std::string& s) {
        // Must start with '"'
        if (s.empty() || s[0] != '"') return {{}, {}};

        auto closeQuote = s.find('"', 1);
        if (closeQuote == std::string::npos) return {{}, {}};
        std::string key = s.substr(1, closeQuote - 1);

        auto colonPos = s.find(':', closeQuote + 1);
        if (colonPos == std::string::npos) return {{}, {}};

        auto afterColon = trim(s.substr(colonPos + 1));
        afterColon = stripTrailingComma(afterColon);

        if (afterColon == "{" || afterColon == "[")
            return {key, afterColon};
        return {{}, {}};
    }

    // Parse `"key": value` or `"key": "value"`.
    // Returns {key, unquoted-value}; empty key on failure.
    static std::pair<std::string, std::string> parseKeyValue(const std::string& s) {
        if (s.empty() || s[0] != '"') return {{}, {}};

        auto closeQuote = s.find('"', 1);
        if (closeQuote == std::string::npos) return {{}, {}};
        std::string key = s.substr(1, closeQuote - 1);

        auto colonPos = s.find(':', closeQuote + 1);
        if (colonPos == std::string::npos) return {{}, {}};

        auto valueRaw = trim(s.substr(colonPos + 1));
        valueRaw = stripTrailingComma(valueRaw);

        // Reject object/array openers (handled separately)
        if (valueRaw == "{" || valueRaw == "[") return {{}, {}};

        return {key, unquote(valueRaw)};
    }

    static int parseIntOr(const std::string& s, int fallback) {
        if (s.empty()) return fallback;
        try { return std::stoi(s); }
        catch (const std::invalid_argument&) { return fallback; }
        catch (const std::out_of_range&)     { return fallback; }
    }

    static std::string toLower(const std::string& s) {
        std::string out = s;
        for (auto& c : out)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return out;
    }
};

} // namespace NF
