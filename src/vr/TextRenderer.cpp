#include "TextRenderer.hpp"
#include <stdexcept>
#include <algorithm>
#include <chrono>

namespace VR_DAW {

TextRenderer& TextRenderer::getInstance() {
    static TextRenderer instance;
    return instance;
}

TextRenderer::TextRenderer()
    : initialized(false)
    , renderScale(1.0f)
    , renderQuality(1)
    , debugEnabled(false)
{
    initializeFreeType();
}

TextRenderer::~TextRenderer() {
    cleanup();
}

void TextRenderer::renderText(const std::string& text, const glm::vec3& position, float fontSize, const glm::vec4& color) {
    if (!initialized) return;

    auto startTime = std::chrono::high_resolution_clock::now();
    metrics.drawCalls = 0;
    metrics.glyphsRendered = 0;

    // Standard-Font verwenden
    auto it = fonts.find("default");
    if (it == fonts.end()) {
        throw std::runtime_error("Kein Standard-Font geladen");
    }

    Font& font = it->second;
    float scale = fontSize / font.face->size->metrics.height * renderScale;
    glm::vec3 currentPos = position;

    // Text rendern
    for (char32_t c : text) {
        loadGlyph(font, c);
        auto& glyph = font.glyphs[c];

        // Glyph-Position berechnen
        glm::vec3 glyphPos = currentPos;
        glyphPos.x += glyph.bearing.x * scale;
        glyphPos.y += (font.face->size->metrics.ascender - glyph.bearing.y) * scale;

        // Glyph rendern
        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glyphPos);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(glyph.size * scale, 1.0f));

        // Textur binden und Glyph rendern
        glBindTexture(GL_TEXTURE_2D, glyph.textureId);
        // TODO: Shader-Programm binden und Uniforms setzen
        // TODO: Vertex-Buffer für Glyph erstellen und rendern

        currentPos.x += glyph.advance * scale;
        metrics.glyphsRendered++;
    }

    metrics.drawCalls++;
    
    auto endTime = std::chrono::high_resolution_clock::now();
    metrics.renderTime = std::chrono::duration<float>(endTime - startTime).count();
}

void TextRenderer::renderText3D(const std::string& text, const glm::vec3& position, float scale, const glm::vec4& color) {
    if (!initialized) return;
    
    // 3D-Transformationen
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, glm::vec3(scale));
    
    // Extrusion für 3D-Effekt
    float depth = 0.1f * scale;
    
    // Shader-Programm aktivieren
    glUseProgram(textShaderProgram);
    
    // Uniforms setzen
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform4fv(textColorLoc, 1, glm::value_ptr(color));
    glUniform1i(useLightingLoc, GL_TRUE);
    glUniform1f(ambientStrengthLoc, 0.3f);
    glUniform1f(specularStrengthLoc, 0.5f);
    
    // Für jedes Zeichen
    float x = 0.0f;
    for (char c : text) {
        GlyphInfo glyph = getGlyphInfo(c);
        if (!glyph.textureID) continue;
        
        // Vorder- und Rückseite des 3D-Textes
        renderGlyph3D(glyph, x, depth);
        x += glyph.advance * scale;
    }
}

void TextRenderer::renderGlyph3D(const GlyphInfo& glyph, float x, float depth) {
    // Vertex-Daten für 3D-Glyph
    struct Vertex {
        glm::vec3 position;
        glm::vec2 texCoord;
        glm::vec3 normal;
    };
    
    // Vorder- und Rückseite des Glyphs
    std::vector<Vertex> vertices = {
        // Vorderseite
        {{x + glyph.bearing.x, glyph.bearing.y, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{x + glyph.bearing.x + glyph.size.x, glyph.bearing.y, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{x + glyph.bearing.x + glyph.size.x, glyph.bearing.y - glyph.size.y, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        {{x + glyph.bearing.x, glyph.bearing.y - glyph.size.y, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        
        // Rückseite
        {{x + glyph.bearing.x, glyph.bearing.y, -depth}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
        {{x + glyph.bearing.x + glyph.size.x, glyph.bearing.y, -depth}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
        {{x + glyph.bearing.x + glyph.size.x, glyph.bearing.y - glyph.size.y, -depth}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
        {{x + glyph.bearing.x, glyph.bearing.y - glyph.size.y, -depth}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}}
    };
    
    // Indizes für die Dreiecke
    std::vector<unsigned int> indices = {
        // Vorderseite
        0, 1, 2,
        2, 3, 0,
        // Rückseite
        4, 5, 6,
        6, 7, 4,
        // Seiten
        0, 4, 7,
        7, 3, 0,
        1, 5, 6,
        6, 2, 1,
        0, 1, 5,
        5, 4, 0,
        3, 2, 6,
        6, 7, 3
    };
    
    // VAO und VBO erstellen
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    // Vertex-Attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    
    // Glyph-Textur binden
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, glyph.textureID);
    
    // Glyph rendern
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    
    // Aufräumen
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

bool TextRenderer::loadFont(const std::string& name, const std::string& path) {
    // Prüfen ob Font bereits geladen ist
    auto it = fonts.find(name);
    if (it != fonts.end()) {
        if (it->second.path == path) {
            return true; // Font bereits geladen
        }
        unloadFont(name); // Font neu laden
    }
    
    // Font laden
    Font font;
    font.path = path;
    
    FT_Error error = FT_New_Face(library, path.c_str(), 0, &font.face);
    if (error) {
        throw std::runtime_error("Konnte Font nicht laden: " + path);
    }
    
    // Font-Größe setzen
    error = FT_Set_Char_Size(font.face, 0, 64 * 64, 96, 96);
    if (error) {
        FT_Done_Face(font.face);
        throw std::runtime_error("Konnte Font-Größe nicht setzen: " + path);
    }
    
    // Atlas erstellen
    createFontAtlas(name);
    
    // Font speichern
    fonts[name] = font;
    
    return true;
}

void TextRenderer::unloadFont(const std::string& name) {
    auto it = fonts.find(name);
    if (it != fonts.end()) {
        FT_Done_Face(it->second.face);
        fonts.erase(it);
    }
}

glm::vec2 TextRenderer::getTextSize(const std::string& text, float fontSize) {
    if (!initialized) return glm::vec2(0.0f);

    auto it = fonts.find("default");
    if (it == fonts.end()) {
        throw std::runtime_error("Kein Standard-Font geladen");
    }

    Font& font = it->second;
    float scale = fontSize / font.face->size->metrics.height;
    glm::vec2 size(0.0f);

    for (char32_t c : text) {
        loadGlyph(font, c);
        auto& glyph = font.glyphs[c];
        size.x += glyph.advance * scale;
        size.y = std::max(size.y, glyph.size.y * scale);
    }

    return size;
}

float TextRenderer::getTextHeight(float fontSize) {
    if (!initialized) return 0.0f;

    auto it = fonts.find("default");
    if (it == fonts.end()) {
        throw std::runtime_error("Kein Standard-Font geladen");
    }

    return fontSize;
}

void TextRenderer::setRenderScale(float scale) {
    renderScale = std::max(0.1f, scale);
}

void TextRenderer::setRenderQuality(int quality) {
    renderQuality = std::max(0, std::min(3, quality));
}

void TextRenderer::enableDebugRendering(bool enable) {
    debugEnabled = enable;
}

void TextRenderer::clear() {
    for (auto& font : fonts) {
        FT_Done_Face(font.second.face);
    }
    fonts.clear();
}

void TextRenderer::cleanup() {
    clear();
    cleanupFreeType();
}

void TextRenderer::initializeFreeType() {
    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        throw std::runtime_error("Konnte FreeType nicht initialisieren");
    }
    
    // Shader-Programme initialisieren
    initializeShaders();
    
    initialized = true;
}

void TextRenderer::initializeShaders() {
    // Vertex-Shader kompilieren
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexSource = R"(
        #version 450 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        layout (location = 2) in vec3 aNormal;
        
        out vec2 TexCoord;
        out vec3 Normal;
        out vec3 FragPos;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            TexCoord = aTexCoord;
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);
    
    // Fragment-Shader kompilieren
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentSource = R"(
        #version 450 core
        in vec2 TexCoord;
        in vec3 Normal;
        in vec3 FragPos;
        
        out vec4 FragColor;
        
        uniform sampler2D text;
        uniform vec4 textColor;
        uniform vec3 lightPos;
        uniform vec3 viewPos;
        uniform bool useLighting;
        uniform float ambientStrength;
        uniform float specularStrength;
        
        void main() {
            vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoord).r);
            vec4 color = textColor * sampled;
            
            if (useLighting) {
                vec3 ambient = ambientStrength * vec3(1.0);
                vec3 norm = normalize(Normal);
                vec3 lightDir = normalize(lightPos - FragPos);
                float diff = max(dot(norm, lightDir), 0.0);
                vec3 diffuse = diff * vec3(1.0);
                vec3 viewDir = normalize(viewPos - FragPos);
                vec3 reflectDir = reflect(-lightDir, norm);
                float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
                vec3 specular = specularStrength * spec * vec3(1.0);
                vec3 result = (ambient + diffuse + specular) * color.rgb;
                FragColor = vec4(result, color.a);
            } else {
                FragColor = color;
            }
        }
    )";
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);
    
    // Shader-Programm erstellen
    textShaderProgram = glCreateProgram();
    glAttachShader(textShaderProgram, vertexShader);
    glAttachShader(textShaderProgram, fragmentShader);
    glLinkProgram(textShaderProgram);
    
    // Shader aufräumen
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Uniform-Locations speichern
    modelLoc = glGetUniformLocation(textShaderProgram, "model");
    viewLoc = glGetUniformLocation(textShaderProgram, "view");
    projectionLoc = glGetUniformLocation(textShaderProgram, "projection");
    textColorLoc = glGetUniformLocation(textShaderProgram, "textColor");
    lightPosLoc = glGetUniformLocation(textShaderProgram, "lightPos");
    viewPosLoc = glGetUniformLocation(textShaderProgram, "viewPos");
    useLightingLoc = glGetUniformLocation(textShaderProgram, "useLighting");
    ambientStrengthLoc = glGetUniformLocation(textShaderProgram, "ambientStrength");
    specularStrengthLoc = glGetUniformLocation(textShaderProgram, "specularStrength");
}

void TextRenderer::cleanupFreeType() {
    FT_Done_FreeType(library);
    initialized = false;
}

void TextRenderer::createFontAtlas(const std::string& fontName) {
    auto it = fonts.find(fontName);
    if (it == fonts.end()) {
        throw std::runtime_error("Font nicht gefunden: " + fontName);
    }
    
    // Atlas-Name generieren
    std::string atlasName = fontName + "_atlas";
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

void TextRenderer::loadGlyph(Font& font, char32_t character) {
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

void TextRenderer::createGlyphTexture(Font& font, char32_t character, GlyphInfo& info) {
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