#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/TextureEditor.h"

using namespace NF;

TEST_CASE("TextureFormat names cover all 8 values", "[Editor][S35]") {
    REQUIRE(std::string(textureFormatName(TextureFormat::R8))      == "R8");
    REQUIRE(std::string(textureFormatName(TextureFormat::RG8))     == "RG8");
    REQUIRE(std::string(textureFormatName(TextureFormat::RGB8))    == "RGB8");
    REQUIRE(std::string(textureFormatName(TextureFormat::RGBA8))   == "RGBA8");
    REQUIRE(std::string(textureFormatName(TextureFormat::R16F))    == "R16F");
    REQUIRE(std::string(textureFormatName(TextureFormat::RG16F))   == "RG16F");
    REQUIRE(std::string(textureFormatName(TextureFormat::RGB16F))  == "RGB16F");
    REQUIRE(std::string(textureFormatName(TextureFormat::RGBA16F)) == "RGBA16F");
}

TEST_CASE("TextureFilter names cover all 4 values", "[Editor][S35]") {
    REQUIRE(std::string(textureFilterName(TextureFilter::Nearest))              == "Nearest");
    REQUIRE(std::string(textureFilterName(TextureFilter::Linear))               == "Linear");
    REQUIRE(std::string(textureFilterName(TextureFilter::NearestMipmapNearest)) == "NearestMipmapNearest");
    REQUIRE(std::string(textureFilterName(TextureFilter::LinearMipmapLinear))   == "LinearMipmapLinear");
}

TEST_CASE("TextureWrapMode names cover all 4 values", "[Editor][S35]") {
    REQUIRE(std::string(textureWrapModeName(TextureWrapMode::Repeat))         == "Repeat");
    REQUIRE(std::string(textureWrapModeName(TextureWrapMode::MirroredRepeat)) == "MirroredRepeat");
    REQUIRE(std::string(textureWrapModeName(TextureWrapMode::ClampToEdge))    == "ClampToEdge");
    REQUIRE(std::string(textureWrapModeName(TextureWrapMode::ClampToBorder))  == "ClampToBorder");
}

TEST_CASE("TextureAsset default values", "[Editor][S35]") {
    TextureAsset tex("diffuse", 512, 512);
    REQUIRE(tex.format()         == TextureFormat::RGBA8);
    REQUIRE(tex.filter()         == TextureFilter::Linear);
    REQUIRE(tex.wrapMode()       == TextureWrapMode::Repeat);
    REQUIRE_FALSE(tex.mipmapsEnabled());
    REQUIRE_FALSE(tex.isDirty());
    REQUIRE(tex.width()  == 512);
    REQUIRE(tex.height() == 512);
    REQUIRE(tex.pixelCount() == 512u * 512u);
}

TEST_CASE("TextureAsset isHDR detects 16F formats", "[Editor][S35]") {
    TextureAsset ldr("ldr"); ldr.setFormat(TextureFormat::RGBA8);
    REQUIRE_FALSE(ldr.isHDR());

    TextureAsset hdr("hdr"); hdr.setFormat(TextureFormat::RGBA16F);
    REQUIRE(hdr.isHDR());

    TextureAsset hdr2("hdr2"); hdr2.setFormat(TextureFormat::RGB16F);
    REQUIRE(hdr2.isHDR());
}

TEST_CASE("TextureAsset setters round-trip", "[Editor][S35]") {
    TextureAsset tex("normal", 256, 256);
    tex.setFormat(TextureFormat::RG8);
    tex.setFilter(TextureFilter::NearestMipmapNearest);
    tex.setWrapMode(TextureWrapMode::ClampToEdge);
    tex.setMipmapsEnabled(true);
    tex.setDirty(true);
    tex.resize(128, 64);

    REQUIRE(tex.format()          == TextureFormat::RG8);
    REQUIRE(tex.filter()          == TextureFilter::NearestMipmapNearest);
    REQUIRE(tex.wrapMode()        == TextureWrapMode::ClampToEdge);
    REQUIRE(tex.mipmapsEnabled());
    REQUIRE(tex.isDirty());
    REQUIRE(tex.width()  == 128);
    REQUIRE(tex.height() == 64);
    REQUIRE(tex.pixelCount() == 128u * 64u);
}

TEST_CASE("TextureEditor addTexture and duplicate rejection", "[Editor][S35]") {
    TextureEditor editor;
    TextureAsset a("texA", 256, 256);
    TextureAsset b("texB", 512, 512);
    TextureAsset dup("texA");

    REQUIRE(editor.addTexture(a));
    REQUIRE(editor.addTexture(b));
    REQUIRE_FALSE(editor.addTexture(dup));
    REQUIRE(editor.textureCount() == 2);
}

TEST_CASE("TextureEditor removeTexture clears activeTexture", "[Editor][S35]") {
    TextureEditor editor;
    TextureAsset a("active");
    editor.addTexture(a);
    editor.setActiveTexture("active");
    REQUIRE(editor.activeTexture() == "active");

    editor.removeTexture("active");
    REQUIRE(editor.textureCount()     == 0);
    REQUIRE(editor.activeTexture().empty());
}

TEST_CASE("TextureEditor setActiveTexture returns false for missing texture", "[Editor][S35]") {
    TextureEditor editor;
    REQUIRE_FALSE(editor.setActiveTexture("ghost"));
    REQUIRE(editor.activeTexture().empty());
}

TEST_CASE("TextureEditor findTexture returns pointer or nullptr", "[Editor][S35]") {
    TextureEditor editor;
    TextureAsset a("findme", 64, 64);
    editor.addTexture(a);

    REQUIRE(editor.findTexture("findme") != nullptr);
    REQUIRE(editor.findTexture("findme")->name() == "findme");
    REQUIRE(editor.findTexture("unknown") == nullptr);
}

TEST_CASE("TextureEditor dirtyCount, hdrCount, mipmapCount", "[Editor][S35]") {
    TextureEditor editor;
    TextureAsset a("a");  a.setDirty(true); a.setFormat(TextureFormat::RGBA16F);
    TextureAsset b("b");  b.setMipmapsEnabled(true);
    TextureAsset c("c");  c.setDirty(true); c.setFormat(TextureFormat::R16F); c.setMipmapsEnabled(true);

    editor.addTexture(a);
    editor.addTexture(b);
    editor.addTexture(c);

    REQUIRE(editor.dirtyCount()  == 2);
    REQUIRE(editor.hdrCount()    == 2);
    REQUIRE(editor.mipmapCount() == 2);
}

TEST_CASE("TextureEditor MAX_TEXTURES limit enforced", "[Editor][S35]") {
    TextureEditor editor;
    for (size_t i = 0; i < TextureEditor::MAX_TEXTURES; ++i) {
        TextureAsset t("t" + std::to_string(i));
        REQUIRE(editor.addTexture(t));
    }
    TextureAsset overflow("overflow");
    REQUIRE_FALSE(editor.addTexture(overflow));
    REQUIRE(editor.textureCount() == TextureEditor::MAX_TEXTURES);
}

TEST_CASE("TextureAsset pixelCount large dimensions", "[Editor][S35]") {
    TextureAsset tex("big", 4096, 4096);
    REQUIRE(tex.pixelCount() == 4096ull * 4096ull);
}
