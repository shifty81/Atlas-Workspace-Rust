#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::ui {

/// Handle to a loaded font resource.
using FontHandle = uint32_t;
constexpr FontHandle kInvalidFont = 0;

/// Font bootstrap system — initialises the default UI font after the
/// renderer has been set up.  Custom implementation, no ImGui.
class FontBootstrap {
public:
    /// Initialise the font system.  Call once after renderer init.
    /// @param assetRoot  Root asset directory (e.g. "assets").
    /// @param dpiScale   Current display DPI scale factor.
    /// @return true if a usable font was loaded.
    bool Init(const std::string& assetRoot, float dpiScale = 1.0f);

    /// Rebuild the font atlas texture (e.g. after a renderer reset).
    void RebuildFontAtlas();

    /// Shut down the font system and release resources.
    void Shutdown();

    /// Returns the default font handle, or kInvalidFont if not loaded.
    FontHandle GetDefaultFont() const;

    /// Returns true when a valid font is loaded and ready for use.
    bool IsReady() const;

    /// Returns the base font size (before DPI scaling).
    float GetBaseFontSize() const;

    /// Returns the DPI scale that was used during initialisation.
    float GetDPIScale() const;

    /// Returns the name of the loaded font ("builtin-fallback" if using fallback)
    const std::string& GetFontName() const;

    /// Returns true if using the built-in fallback font
    bool IsUsingFallback() const;

    /// Register an additional font search directory.
    void AddFontSearchPath(const std::string& path);

    /// Returns all registered font search paths.
    const std::vector<std::string>& GetFontSearchPaths() const;

    /// Scan all search paths and return discovered font file paths (.ttf, .otf).
    std::vector<std::string> DiscoverFonts() const;

    /// Attempt to load a specific font file by full path.
    /// Returns true if the font was successfully validated and loaded.
    bool LoadFont(const std::string& fontPath);

    /// Returns the path to the currently loaded font file (empty if using fallback).
    const std::string& GetLoadedFontPath() const;

private:
    FontHandle m_defaultFont = kInvalidFont;
    float m_baseFontSize = 18.0f;
    float m_dpiScale = 1.0f;
    bool m_ready = false;
    std::string m_fontName;
    bool m_usingFallback = false;
    std::vector<std::string> m_searchPaths;
    std::string m_loadedFontPath;
};

} // namespace atlas::ui
