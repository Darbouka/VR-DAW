#pragma once

#include <string>
#include <map>
#include <vector>
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "TextureManager.hpp"
#include <memory>
#include <functional>

namespace VR_DAW {

class TextRenderer {
public:
    static TextRenderer& getInstance();

    // Text-Rendering
    void renderText(const std::string& text, const glm::vec3& position, float scale, const glm::vec4& color);
    void renderText3D(const std::string& text, const glm::vec3& position, float scale, const glm::vec4& color);
    
    // Font-Management
    bool loadFont(const std::string& fontPath, float fontSize);
    void unloadFont(const std::string& fontName);
    
    // Text-Metriken
    glm::vec2 getTextSize(const std::string& text, float fontSize);
    float getTextHeight(float fontSize);
    
    // Rendering-Einstellungen
    void setRenderScale(float scale);
    void setRenderQuality(int quality);
    void enableDebugRendering(bool enable);
    
    // Speicher-Management
    void clear();
    void cleanup();

    // Neue Funktionen für verbessertes Text-Rendering
    void setTextAlignment(TextAlignment alignment);
    void setTextStyle(const TextStyle& style);
    void setTextShadow(const TextShadow& shadow);
    void setTextOutline(const TextOutline& outline);
    void setTextGradient(const TextGradient& gradient);
    void setTextAnimation(const TextAnimation& animation);
    
    // Performance-Optimierungen
    void preloadGlyphs(const std::string& text);
    void clearGlyphCache();
    void setGlyphCacheSize(size_t size);

private:
    TextRenderer();
    ~TextRenderer();
    
    // Singleton-Pattern
    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;
    
    // Font-Struktur
    struct FontInfo {
        FT_Face face;
        std::map<char, GlyphInfo> glyphs;
        float fontSize;
    };
    
    // Glyph-Informationen
    struct GlyphInfo {
        GLuint textureID;
        glm::ivec2 size;
        glm::ivec2 bearing;
        float advance;
    };
    
    // Rendering-Hilfsfunktionen
    void initializeFreeType();
    void cleanupFreeType();
    void initializeShaders();
    void createFontAtlas(const std::string& fontName);
    void loadGlyph(FontInfo& font, char c);
    void createGlyphTexture(FontInfo& font, char c, GlyphInfo& info);
    
    // Member-Variablen
    std::map<std::string, FontInfo> fonts;
    FT_Library library;
    bool initialized;
    float renderScale;
    int renderQuality;
    bool debugEnabled;
    
    // Shader-Programm
    GLuint textShaderProgram;
    GLint modelLoc;
    GLint viewLoc;
    GLint projectionLoc;
    GLint textColorLoc;
    GLint lightPosLoc;
    GLint viewPosLoc;
    GLint useLightingLoc;
    GLint ambientStrengthLoc;
    GLint specularStrengthLoc;
    
    // Neue Member-Variablen
    TextAlignment currentAlignment;
    TextStyle currentStyle;
    TextShadow currentShadow;
    TextOutline currentOutline;
    TextGradient currentGradient;
    TextAnimation currentAnimation;
    GlyphCache glyphCache;
    
    // Performance-Metriken
    struct Metrics {
        size_t drawCalls;
        size_t glyphsRendered;
        float renderTime;
    } metrics;
    
    // Private Hilfsfunktionen
    GlyphInfo loadGlyph(char c, const FontInfo& font);
    void createGlyphTexture(FT_GlyphSlot glyph, GlyphInfo& info);
    void renderGlyph3D(const GlyphInfo& glyph, float x, float depth);
    void applyTextEffects(const std::string& text, const glm::vec3& position, float scale);
    void updateTextAnimation(float deltaTime);
};

} // namespace VR_DAW 