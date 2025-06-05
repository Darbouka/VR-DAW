#pragma once

#include <string>
#include <map>
#include <vector>
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "TextureManager.hpp"

namespace VR_DAW {

class TextRenderer {
public:
    static TextRenderer& getInstance();

    // Text-Rendering
    void renderText(const std::string& text, const glm::vec3& position, float fontSize, const glm::vec4& color = glm::vec4(1.0f));
    void renderText3D(const std::string& text, const glm::vec3& position, float fontSize, const glm::vec4& color = glm::vec4(1.0f));
    
    // Font-Management
    bool loadFont(const std::string& name, const std::string& path);
    void unloadFont(const std::string& name);
    
    // Text-Metriken
    glm::vec2 getTextSize(const std::string& text, float fontSize);
    float getTextHeight(float fontSize);
    
    // Rendering-Einstellungen
    void setRenderScale(float scale);
    void setRenderQuality(int quality);
    void enableDebugRendering(bool enable);
    
    // Speicherverwaltung
    void clear();
    void cleanup();

private:
    TextRenderer();
    ~TextRenderer();
    
    // Singleton-Pattern
    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;
    
    // Font-Struktur
    struct Font {
        FT_Face face;
        std::string path;
        std::map<char32_t, GlyphInfo> glyphs;
        std::string atlasName;
    };
    
    // Glyph-Informationen
    struct GlyphInfo {
        glm::vec2 size;
        glm::vec2 bearing;
        float advance;
        GLuint textureId;
        glm::vec2 texCoords[4];
    };
    
    // Rendering-Hilfsfunktionen
    void initializeFreeType();
    void cleanupFreeType();
    void createFontAtlas(const std::string& fontName);
    void loadGlyph(Font& font, char32_t character);
    void createGlyphTexture(Font& font, char32_t character, GlyphInfo& info);
    
    // Member-Variablen
    std::map<std::string, Font> fonts;
    FT_Library library;
    bool initialized;
    float renderScale;
    int renderQuality;
    bool debugEnabled;
    
    // Performance-Metriken
    struct Metrics {
        size_t drawCalls;
        size_t glyphsRendered;
        float renderTime;
    } metrics;
};

} // namespace VR_DAW 