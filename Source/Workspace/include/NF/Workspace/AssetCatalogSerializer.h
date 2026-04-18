#pragma once
// NF::Workspace — AssetCatalogSerializer: serialize/deserialize AssetCatalog.
//
// The AssetCatalogSerializer produces a compact text record format for the
// AssetCatalog — one line per asset descriptor — and parses it back into a
// catalog. It integrates with WorkspaceProjectFile by writing into / reading
// from a named "AssetCatalog" section.
//
// Wire format (one record per line within the section):
//   asset.<n>=<id>|<typeTag>|<importState>|<catalogPath>|<sourcePath>|<displayName>
//
// Metadata is written as:
//   asset.<n>.meta.<key>=<value>   (one entry per metadata pair)
//
// Goals:
//   - Lossless round-trip for all AssetDescriptor fields including metadata
//   - Rejection of malformed records (returns descriptive error)
//   - No filesystem calls

#include "NF/Workspace/AssetCatalog.h"
#include "NF/Workspace/WorkspaceProjectFile.h"
#include <cstdint>
#include <sstream>
#include <string>

namespace NF {

// ── Catalog Serializer Result ──────────────────────────────────────

struct CatalogSerializeResult {
    bool        succeeded = false;
    size_t      assetCount = 0;
    std::string errorMessage;

    [[nodiscard]] bool failed() const { return !succeeded; }

    static CatalogSerializeResult ok(size_t n) { return {true, n, {}}; }
    static CatalogSerializeResult fail(const std::string& msg) { return {false, 0, msg}; }
};

// ── Asset Catalog Serializer ───────────────────────────────────────

class AssetCatalogSerializer {
public:
    static constexpr const char* SECTION_NAME = "AssetCatalog";

    // ── Serialize ─────────────────────────────────────────────────

    // Write catalog into the "AssetCatalog" section of |file|.
    static CatalogSerializeResult serialize(const AssetCatalog& catalog,
                                            WorkspaceProjectFile& file) {
        auto& sec = file.section(SECTION_NAME);
        sec.set("count", std::to_string(catalog.count()));

        size_t n = 0;
        for (const auto* desc : catalog.all()) {
            if (!desc || !desc->isValid()) continue;
            std::string prefix = "asset." + std::to_string(n);

            // Pack the descriptor fields into a pipe-delimited record
            std::string record = std::to_string(desc->id)
                + "|" + std::to_string(static_cast<uint8_t>(desc->typeTag))
                + "|" + std::to_string(static_cast<uint8_t>(desc->importState))
                + "|" + escapePipe(desc->catalogPath)
                + "|" + escapePipe(desc->sourcePath)
                + "|" + escapePipe(desc->displayName);

            sec.set(prefix, record);

            // Metadata — each entry as a separate key
            size_t mi = 0;
            for (const auto& [k, v] : desc->metadata.entries()) {
                sec.set(prefix + ".meta." + std::to_string(mi) + ".k", k);
                sec.set(prefix + ".meta." + std::to_string(mi) + ".v", v);
                ++mi;
            }
            if (mi > 0)
                sec.set(prefix + ".meta.count", std::to_string(mi));

            ++n;
        }
        sec.set("written", std::to_string(n));
        return CatalogSerializeResult::ok(n);
    }

    // ── Deserialize ───────────────────────────────────────────────

    // Read the "AssetCatalog" section of |file| into |catalog|.
    // Any pre-existing catalog content is preserved (assets are added).
    static CatalogSerializeResult deserialize(const WorkspaceProjectFile& file,
                                              AssetCatalog& catalog) {
        const auto* sec = file.findSection(SECTION_NAME);
        if (!sec) return CatalogSerializeResult::fail("no AssetCatalog section");

        const auto* writtenStr = sec->get("written");
        if (!writtenStr) return CatalogSerializeResult::fail("missing 'written' key");

        size_t written = static_cast<size_t>(std::stoul(*writtenStr));
        size_t loaded  = 0;

        for (size_t n = 0; n < written; ++n) {
            std::string prefix  = "asset." + std::to_string(n);
            const auto* recPtr  = sec->get(prefix);
            if (!recPtr) continue;

            AssetDescriptor desc;
            if (!unpackRecord(*recPtr, desc)) continue;

            // Metadata
            const auto* metaCountStr = sec->get(prefix + ".meta.count");
            if (metaCountStr) {
                size_t mc = static_cast<size_t>(std::stoul(*metaCountStr));
                for (size_t mi = 0; mi < mc; ++mi) {
                    auto kp = sec->get(prefix + ".meta." + std::to_string(mi) + ".k");
                    auto vp = sec->get(prefix + ".meta." + std::to_string(mi) + ".v");
                    if (kp && vp) desc.metadata.set(*kp, *vp);
                }
            }

            if (catalog.add(desc) != INVALID_ASSET_ID) ++loaded;
        }

        return CatalogSerializeResult::ok(loaded);
    }

    // ── Round-trip helper ─────────────────────────────────────────

    static CatalogSerializeResult roundTrip(const AssetCatalog& src, AssetCatalog& out) {
        WorkspaceProjectFile file;
        file.setProjectId("rt"); file.setProjectName("rt");
        auto res = serialize(src, file);
        if (res.failed()) return res;

        std::string text = file.serialize();
        WorkspaceProjectFile parsed;
        if (!WorkspaceProjectFile::parse(text, parsed))
            return CatalogSerializeResult::fail("parse failed");

        return deserialize(parsed, out);
    }

private:
    // Escape '|' in field values with '\P' so the delimiter is unambiguous.
    static std::string escapePipe(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            if (c == '|') out += "\\P";
            else          out += c;
        }
        return out;
    }

    static std::string unescapePipe(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size() && s[i+1] == 'P') {
                out += '|'; ++i;
            } else {
                out += s[i];
            }
        }
        return out;
    }

    // Split on first N '|' separators (fields may contain escaped pipes).
    // Returns field vector; up to 6 fields expected.
    static std::vector<std::string> splitFields(const std::string& rec) {
        std::vector<std::string> fields;
        std::string cur;
        for (size_t i = 0; i < rec.size(); ++i) {
            if (rec[i] == '\\' && i + 1 < rec.size() && rec[i+1] == 'P') {
                cur += '|'; ++i;
            } else if (rec[i] == '|' && fields.size() < 5) {
                fields.push_back(std::move(cur));
                cur.clear();
            } else {
                cur += rec[i];
            }
        }
        fields.push_back(std::move(cur));
        return fields;
    }

    // Unpack a pipe-delimited record into a descriptor (id field is discarded —
    // AssetCatalog::add() assigns a new id).
    static bool unpackRecord(const std::string& rec, AssetDescriptor& out) {
        auto fields = splitFields(rec);
        if (fields.size() < 6) return false;

        // fields[0] = id (we ignore, catalog assigns)
        uint8_t typeTagRaw    = static_cast<uint8_t>(std::stoul(fields[1]));
        uint8_t importStateRaw = static_cast<uint8_t>(std::stoul(fields[2]));
        out.catalogPath  = fields[3]; // already unescaped by splitFields
        out.sourcePath   = fields[4];
        out.displayName  = fields[5];
        out.typeTag      = static_cast<AssetTypeTag>(typeTagRaw);
        out.importState  = static_cast<AssetImportState>(importStateRaw);
        return !out.sourcePath.empty() && !out.catalogPath.empty()
            && out.typeTag != AssetTypeTag::Unknown;
    }
};

} // namespace NF
