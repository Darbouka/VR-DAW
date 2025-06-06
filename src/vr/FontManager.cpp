#include "FontManager.hpp"
#include <stdexcept>
#include <algorithm>

namespace VR_DAW {

FontManager& FontManager::getInstance() {
    static FontManager instance;
    return instance;
}

FontManager::FontManager() {
    initializeFreeType();
}

FontManager::~FontManager() {
    cleanup();
}

bool FontManager::loadFont(const std::string& name, const std::string& path, float size) {
    // Prüfen ob Font bereits geladen ist
    auto it = fonts.find(name);
    if (it != fonts.end()) {
        if (it->second.path == path && it->second.size == size) {
            return true; // Font bereits geladen
        }
        unloadFont(name); // Font neu laden
    }
    
    // Font laden
    FontInfo fontInfo;
    fontInfo.path = path;
    fontInfo.size = size;
    
    FT_Error error = FT_New_Face(library, path.c_str(), 0, &fontInfo.face);
    if (error) {
        throw std::runtime_error("Konnte Font nicht laden: " + path);
    }
    
    // Font-Größe setzen
    error = FT_Set_Char_Size(fontInfo.face, 0, static_cast<FT_F26Dot6>(size * 64), 96, 96);
    if (error) {
        FT_Done_Face(fontInfo.face);
        throw std::runtime_error("Konnte Font-Größe nicht setzen: " + path);
    }
    
    // Atlas erstellen
    createFontAtlas(name, size);
    
    // Font speichern
    fonts[name] = fontInfo;
    
    return true;
}

void FontManager::unloadFont(const std::string& name) {
    auto it = fonts.find(name);
    if (it != fonts.end()) {
        FT_Done_Face(it->second.face);
        fonts.erase(it);
    }
}

FontManager::TextMetrics FontManager::getTextMetrics(const std::string& text, const std::string& fontName, float fontSize) {
    auto it = fonts.find(fontName);
    if (it == fonts.end()) {
        throw std::runtime_error("Font nicht gefunden: " + fontName);
    }
    
    TextMetrics metrics = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    
    // Font-Metriken abrufen
    metrics.ascender = it->second.face->size->metrics.ascender / 64.0f;
    metrics.descender = it->second.face->size->metrics.descender / 64.0f;
    metrics.baseline = metrics.ascender;
    
    // Text-Metriken berechnen
    float currentX = 0.0f;
    float maxHeight = 0.0f;
    
    for (char32_t c : text) {
        loadGlyph(it->second, c);
        auto& glyph = it->second.glyphs[c];
        
        currentX += glyph.advance;
        maxHeight = std::max(maxHeight, glyph.size.y);
    }
    
    metrics.width = currentX;
    metrics.height = maxHeight;
    
    return metrics;
}

FontManager::GlyphInfo FontManager::getGlyphInfo(const std::string& fontName, char32_t character, float fontSize) {
    auto it = fonts.find(fontName);
    if (it == fonts.end()) {
        throw std::runtime_error("Font nicht gefunden: " + fontName);
    }
    
    loadGlyph(it->second, character);
    return it->second.glyphs[character];
}

void FontManager::createFontAtlas(const std::string& fontName, float fontSize) {
    auto it = fonts.find(fontName);
    if (it == fonts.end()) {
        throw std::runtime_error("Font nicht gefunden: " + fontName);
    }
    
    // Atlas-Name generieren
    std::string atlasName = fontName + "_" + std::to_string(static_cast<int>(fontSize));
    it->second.atlasName = atlasName;
    
    // Atlas erstellen
    auto& textureManager = TextureManager::getInstance();
    
    // Glyphen in Atlas laden
    for (char32_t c = 32; c < 128; ++c) { // ASCII-Zeichen
        loadGlyph(it->second, c);
        auto& glyph = it->second.glyphs[c];
        
        // Glyph-Textur in Atlas kopieren
        auto region = textureManager.addToAtlas(atlasName, "glyph_" + std::to_string(c));
        glyph.texCoords[0] = region.texCoords[0];
        glyph.texCoords[1] = region.texCoords[1];
        glyph.texCoords[2] = region.texCoords[2];
        glyph.texCoords[3] = region.texCoords[3];
    }
}

TextureManager::AtlasRegion FontManager::getGlyphRegion(const std::string& fontName, char32_t character, float fontSize) {
    auto it = fonts.find(fontName);
    if (it == fonts.end()) {
        throw std::runtime_error("Font nicht gefunden: " + fontName);
    }
    
    loadGlyph(it->second, character);
    auto& glyph = it->second.glyphs[character];
    
    TextureManager::AtlasRegion region;
    region.position = glyph.texCoords[0];
    region.size = glyph.size;
    for (int i = 0; i < 4; ++i) {
        region.texCoords[i] = glyph.texCoords[i];
    }
    
    return region;
}

void FontManager::clear() {
    for (auto& font : fonts) {
        FT_Done_Face(font.second.face);
    }
    fonts.clear();
}

void FontManager::cleanup() {
    clear();
    cleanupFreeType();
}

void FontManager::initializeFreeType() {
    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        throw std::runtime_error("Konnte FreeType nicht initialisieren");
    }
}

void FontManager::cleanupFreeType() {
    FT_Done_FreeType(library);
}

void FontManager::loadGlyph(FontInfo& font, char32_t character) {
    // Prüfen ob Glyph bereits geladen ist
    if (font.glyphs.find(character) != font.glyphs.end()) {
        return;
    }
    
    // Glyph laden
    FT_Error error = FT_Load_Char(font.face, character, FT_LOAD_RENDER);
    if (error) {
        throw std::runtime_error("Konnte Glyph nicht laden: " + std::to_string(character));
    }
    
    // Glyph-Textur erstellen
    GlyphInfo info;
    createGlyphTexture(font, character, info);
    
    // Glyph-Info speichern
    font.glyphs[character] = info;
}

void FontManager::createGlyphTexture(FontInfo& font, char32_t character, GlyphInfo& info) {
    FT_GlyphSlot glyph = font.face->glyph;
    
    // Glyph-Metriken speichern
    info.size = glm::vec2(glyph->bitmap.width, glyph->bitmap.rows);
    info.bearing = glm::vec2(glyph->bitmap_left, glyph->bitmap_top);
    info.advance = glyph->advance.x / 64.0f;
    
    // Glyph-Textur erstellen
    if (glyph->bitmap.buffer) {
        auto& textureManager = TextureManager::getInstance();
        info.textureId = textureManager.createTexture("glyph_" + std::to_string(character),
                                                    glyph->bitmap.width,
                                                    glyph->bitmap.rows,
                                                    glyph->bitmap.buffer);
    }
}

} // namespace VR_DAW 