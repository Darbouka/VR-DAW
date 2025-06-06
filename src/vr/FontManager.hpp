#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include "TextureManager.hpp"

namespace VR_DAW {

class FontManager {
public:
    static FontManager& getInstance();

    // Font-Verwaltung
    bool loadFont(const std::string& name, const std::string& path, float size);
    void unloadFont(const std::string& name);
    
    // Text-Rendering
    struct TextMetrics {
        float width;
        float height;
        float baseline;
        float ascender;
        float descender;
    };
    
    struct GlyphInfo {
        unsigned int textureId;
        glm::vec2 size;
        glm::vec2 bearing;
        float advance;
        glm::vec2 texCoords[4];
    };
    
    TextMetrics getTextMetrics(const std::string& text, const std::string& fontName, float fontSize);
    GlyphInfo getGlyphInfo(const std::string& fontName, char32_t character, float fontSize);
    
    // Textur-Atlas für Fonts
    void createFontAtlas(const std::string& fontName, float fontSize);
    TextureManager::AtlasRegion getGlyphRegion(const std::string& fontName, char32_t character, float fontSize);
    
    // Speicherverwaltung
    void clear();
    void cleanup();

private:
    FontManager();
    ~FontManager();
    
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;
    
    struct FontInfo {
        FT_Face face;
        std::string path;
        float size;
        std::unordered_map<char32_t, GlyphInfo> glyphs;
        std::string atlasName;
    };
    
    FT_Library library;
    std::unordered_map<std::string, FontInfo> fonts;
    
    // Hilfsfunktionen
    void initializeFreeType();
    void cleanupFreeType();
    void loadGlyph(FontInfo& font, char32_t character);
    void createGlyphTexture(FontInfo& font, char32_t character, GlyphInfo& info);
};

} // namespace VR_DAW 