#include "FontBootstrap.h"
#include "../core/Logger.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

namespace atlas::ui {

/// Read a big-endian uint16 from a byte buffer.
static uint16_t ReadU16BE(const uint8_t* p) {
    return static_cast<uint16_t>((p[0] << 8) | p[1]);
}

/// Read a big-endian uint32 from a byte buffer.
static uint32_t ReadU32BE(const uint8_t* p) {
    return (static_cast<uint32_t>(p[0]) << 24) |
           (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8)  |
            static_cast<uint32_t>(p[3]);
}

/// Parse a TTF/OTF file header and extract the font family name (nameID 1).
/// Returns the font name string, or empty string on failure.
static std::string ParseTTFHeader(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return {};

    // Read the offset table (12 bytes).
    uint8_t header[12];
    if (!file.read(reinterpret_cast<char*>(header), 12)) return {};

    uint32_t sfVersion = ReadU32BE(header);
    // 0x00010000 = TrueType, 0x4F54544F ('OTTO') = OpenType/CFF
    if (sfVersion != 0x00010000 && sfVersion != 0x4F54544F) return {};

    uint16_t numTables = ReadU16BE(header + 4);

    // Read the table directory entries (16 bytes each).
    constexpr size_t kTableEntrySize = 16;
    std::vector<uint8_t> tableDir(numTables * kTableEntrySize);
    if (!file.read(reinterpret_cast<char*>(tableDir.data()),
                   static_cast<std::streamsize>(tableDir.size())))
        return {};

    // Locate the 'name' table.
    uint32_t nameOffset = 0;
    uint32_t nameLength = 0;
    for (uint16_t i = 0; i < numTables; ++i) {
        const uint8_t* entry = tableDir.data() + i * kTableEntrySize;
        if (std::memcmp(entry, "name", 4) == 0) {
            nameOffset = ReadU32BE(entry + 8);
            nameLength = ReadU32BE(entry + 12);
            break;
        }
    }
    if (nameOffset == 0 || nameLength == 0) return {};

    // Read the name table.
    std::vector<uint8_t> nameTable(nameLength);
    file.seekg(nameOffset);
    if (!file.read(reinterpret_cast<char*>(nameTable.data()),
                   static_cast<std::streamsize>(nameLength)))
        return {};

    // Name table header: format (2), count (2), stringOffset (2).
    if (nameLength < 6) return {};
    uint16_t nameCount    = ReadU16BE(nameTable.data() + 2);
    uint16_t stringOffset = ReadU16BE(nameTable.data() + 4);

    // Each name record is 12 bytes, starting at offset 6.
    for (uint16_t i = 0; i < nameCount; ++i) {
        size_t recOff = 6 + static_cast<size_t>(i) * 12;
        if (recOff + 12 > nameLength) break;

        const uint8_t* rec = nameTable.data() + recOff;
        uint16_t platformID = ReadU16BE(rec + 0);
        uint16_t encodingID = ReadU16BE(rec + 2);
        uint16_t nameID     = ReadU16BE(rec + 6);
        uint16_t strLength  = ReadU16BE(rec + 8);
        uint16_t strOffset  = ReadU16BE(rec + 10);

        if (nameID != 1) continue; // We want font family name

        size_t absOff = static_cast<size_t>(stringOffset) + static_cast<size_t>(strOffset);
        if (absOff < stringOffset || absOff < strOffset) continue; // overflow check
        if (strLength > nameLength || absOff > nameLength - strLength) continue;

        const uint8_t* strData = nameTable.data() + absOff;

        // Platform 1 (Macintosh), encoding 0 (Roman) — single-byte ASCII.
        if (platformID == 1 && encodingID == 0) {
            return std::string(reinterpret_cast<const char*>(strData), strLength);
        }

        // Platform 3 (Windows), encoding 1 (Unicode BMP) — UTF-16BE.
        if (platformID == 3 && encodingID == 1 && strLength >= 2) {
            std::string result;
            result.reserve(strLength / 2);
            for (uint16_t j = 0; j + 1 < strLength; j += 2) {
                uint16_t ch = ReadU16BE(strData + j);
                if (ch < 128)
                    result.push_back(static_cast<char>(ch));
                else
                    result.push_back('?');
            }
            return result;
        }
    }

    return {};
}

/// Generate a minimal built-in font atlas so the editor can render
/// placeholder glyphs when the real Inter-Regular.ttf is not yet shipped.
/// Creates the directory structure and writes a JSON metrics stub.
static bool GenerateBuiltinFontAtlas(const std::string& assetRoot) {
    std::string fontsDir = assetRoot + "/fonts";
    std::error_code ec;
    std::filesystem::create_directories(fontsDir, ec);
    if (ec) return false;

    std::string metricsPath = fontsDir + "/builtin_fallback.json";
    if (std::filesystem::exists(metricsPath)) return true;

    std::ofstream out(metricsPath);
    if (!out.is_open()) return false;

    // Minimal font atlas metrics for ASCII printable range (32–126).
    // Each glyph is a uniform 10×18 cell — enough for the renderer to
    // produce readable monospaced placeholder text.
    out << "{\n"
        << "  \"font\": \"builtin-fallback\",\n"
        << "  \"size\": 18,\n"
        << "  \"lineHeight\": 22,\n"
        << "  \"glyphs\": {\n";
    for (int ch = 32; ch <= 126; ++ch) {
        int col = (ch - 32) % 16;
        int row = (ch - 32) / 16;
        out << "    \"" << ch << "\": {"
            << "\"x\":" << col * 10 << ","
            << "\"y\":" << row * 18 << ","
            << "\"w\":10,\"h\":18,"
            << "\"xOffset\":0,\"yOffset\":0,"
            << "\"advance\":10"
            << "}";
        if (ch < 126) out << ",";
        out << "\n";
    }
    out << "  }\n}\n";
    return out.good();
}

bool FontBootstrap::Init(const std::string& assetRoot, float dpiScale) {
    m_dpiScale = dpiScale;

    AddFontSearchPath(assetRoot + "/fonts");
    std::string fontPath = assetRoot + "/fonts/Inter-Regular.ttf";

    if (!std::filesystem::exists(fontPath)) {
        Logger::Info("Default font not found at: " + fontPath +
                     " — generating built-in fallback font atlas");

        if (!GenerateBuiltinFontAtlas(assetRoot)) {
            Logger::Warn("Failed to generate built-in fallback font atlas");
            m_defaultFont = kInvalidFont;
            m_ready = false;
            return false;
        }

        // Use the built-in fallback so the editor can render placeholder text.
        m_defaultFont = 1; // reserved handle for fallback font
        m_ready = true;
        m_fontName = "builtin-fallback";
        m_usingFallback = true;

        Logger::Info("Font system initialized with built-in fallback '" + m_fontName
                     + "' (asset root: "
                     + assetRoot + ", DPI scale: "
                     + std::to_string(m_dpiScale) + ")");
        return true;
    }

    // Validate the TTF and extract font metadata.
    std::string parsedName = ParseTTFHeader(fontPath);
    if (!parsedName.empty()) {
        m_fontName = parsedName;
    } else {
        Logger::Warn("Could not parse TTF header; assuming Inter-Regular");
        m_fontName = "Inter-Regular";
    }
    m_usingFallback = false;

    m_defaultFont = 1; // reserved handle for the default font
    m_ready = true;
    m_loadedFontPath = fontPath;

    Logger::Info("Font system initialized with '" + m_fontName
                 + "' (asset root: " + assetRoot +
                 ", DPI scale: " + std::to_string(m_dpiScale) + ")");
    return true;
}

void FontBootstrap::RebuildFontAtlas() {
    if (m_defaultFont == kInvalidFont) return;
    Logger::Info("Font atlas rebuilt");
}

void FontBootstrap::Shutdown() {
    m_defaultFont = kInvalidFont;
    m_ready = false;
    Logger::Info("Font system shut down");
}

FontHandle FontBootstrap::GetDefaultFont() const {
    return m_defaultFont;
}

bool FontBootstrap::IsReady() const {
    return m_ready;
}

float FontBootstrap::GetBaseFontSize() const {
    return m_baseFontSize;
}

float FontBootstrap::GetDPIScale() const {
    return m_dpiScale;
}

const std::string& FontBootstrap::GetFontName() const {
    return m_fontName;
}

bool FontBootstrap::IsUsingFallback() const {
    return m_usingFallback;
}

void FontBootstrap::AddFontSearchPath(const std::string& path) {
    if (path.empty()) return;
    // Avoid duplicates
    for (const auto& existing : m_searchPaths) {
        if (existing == path) return;
    }
    m_searchPaths.push_back(path);
    Logger::Info("Font search path added: " + path);
}

const std::vector<std::string>& FontBootstrap::GetFontSearchPaths() const {
    return m_searchPaths;
}

std::vector<std::string> FontBootstrap::DiscoverFonts() const {
    std::vector<std::string> fonts;
    for (const auto& searchPath : m_searchPaths) {
        std::error_code ec;
        if (!std::filesystem::exists(searchPath, ec)) continue;
        if (!std::filesystem::is_directory(searchPath, ec)) continue;

        for (const auto& entry : std::filesystem::directory_iterator(searchPath, ec)) {
            if (!entry.is_regular_file()) continue;
            auto ext = entry.path().extension().string();
            // Case-insensitive extension check
            for (auto& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            if (ext == ".ttf" || ext == ".otf") {
                fonts.push_back(entry.path().string());
            }
        }
    }
    return fonts;
}

bool FontBootstrap::LoadFont(const std::string& fontPath) {
    if (fontPath.empty()) return false;

    std::error_code ec;
    if (!std::filesystem::exists(fontPath, ec)) {
        Logger::Warn("Font file not found: " + fontPath);
        return false;
    }

    std::string parsedName = ParseTTFHeader(fontPath);
    if (parsedName.empty()) {
        Logger::Warn("Failed to parse font file: " + fontPath);
        return false;
    }

    m_fontName = parsedName;
    m_loadedFontPath = fontPath;
    m_defaultFont = 1;
    m_ready = true;
    m_usingFallback = false;

    Logger::Info("Loaded font '" + m_fontName + "' from " + fontPath);
    return true;
}

const std::string& FontBootstrap::GetLoadedFontPath() const {
    return m_loadedFontPath;
}

} // namespace atlas::ui
