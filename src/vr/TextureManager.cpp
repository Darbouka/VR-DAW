#include "TextureManager.hpp"
#include <stb_image.h>
#include <stb_image_write.h>
#include <glad/glad.h>
#include <stdexcept>
#include <algorithm>

namespace VR_DAW {

TextureManager& TextureManager::getInstance() {
    static TextureManager instance;
    return instance;
}

TextureManager::~TextureManager() {
    cleanup();
}

unsigned int TextureManager::loadTexture(const std::string& path) {
    // Prüfen ob Textur bereits geladen ist
    auto it = textures.find(path);
    if (it != textures.end()) {
        return it->second.id;
    }
    
    // Bilddaten laden
    int width, height, channels;
    unsigned char* data = nullptr;
    if (!loadImageData(path, width, height, channels, &data)) {
        throw std::runtime_error("Konnte Textur nicht laden: " + path);
    }
    
    // OpenGL Textur erstellen
    unsigned int textureId = createGLTexture(width, height, channels, data);
    
    // Speicher freigeben
    stbi_image_free(data);
    
    // Textur-Info speichern
    TextureInfo info;
    info.id = textureId;
    info.width = width;
    info.height = height;
    info.channels = channels;
    info.isAtlas = false;
    textures[path] = info;
    
    return textureId;
}

unsigned int TextureManager::createTexture(const std::string& name, int width, int height, const unsigned char* data) {
    // Prüfen ob Textur bereits existiert
    auto it = textures.find(name);
    if (it != textures.end()) {
        deleteGLTexture(it->second.id);
    }
    
    // OpenGL Textur erstellen
    unsigned int textureId = createGLTexture(width, height, 4, data); // RGBA
    
    // Textur-Info speichern
    TextureInfo info;
    info.id = textureId;
    info.width = width;
    info.height = height;
    info.channels = 4;
    info.isAtlas = false;
    textures[name] = info;
    
    return textureId;
}

void TextureManager::deleteTexture(const std::string& name) {
    auto it = textures.find(name);
    if (it != textures.end()) {
        deleteGLTexture(it->second.id);
        textures.erase(it);
    }
}

unsigned int TextureManager::getTexture(const std::string& name) const {
    auto it = textures.find(name);
    if (it != textures.end()) {
        return it->second.id;
    }
    return 0;
}

glm::vec2 TextureManager::getTextureSize(const std::string& name) const {
    auto it = textures.find(name);
    if (it != textures.end()) {
        return glm::vec2(it->second.width, it->second.height);
    }
    return glm::vec2(0.0f);
}

void TextureManager::updateTexture(const std::string& name, const unsigned char* data) {
    auto it = textures.find(name);
    if (it != textures.end()) {
        glBindTexture(GL_TEXTURE_2D, it->second.id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, it->second.width, it->second.height,
                       it->second.channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    }
}

void TextureManager::updateTextureRegion(const std::string& name, int x, int y, int width, int height, const unsigned char* data) {
    auto it = textures.find(name);
    if (it != textures.end()) {
        glBindTexture(GL_TEXTURE_2D, it->second.id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height,
                       it->second.channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    }
}

void TextureManager::setTextureFiltering(const std::string& name, bool linear) {
    auto it = textures.find(name);
    if (it != textures.end()) {
        glBindTexture(GL_TEXTURE_2D, it->second.id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);
    }
}

void TextureManager::setTextureWrapping(const std::string& name, bool repeat) {
    auto it = textures.find(name);
    if (it != textures.end()) {
        glBindTexture(GL_TEXTURE_2D, it->second.id);
        GLint wrapMode = repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    }
}

TextureManager::AtlasRegion TextureManager::addToAtlas(const std::string& atlasName, const std::string& textureName) {
    // Atlas erstellen falls nicht vorhanden
    auto atlasIt = atlases.find(atlasName);
    if (atlasIt == atlases.end()) {
        AtlasInfo atlasInfo;
        atlasInfo.width = 2048;  // Standard-Atlas-Größe
        atlasInfo.height = 2048;
        atlasInfo.id = createGLTexture(atlasInfo.width, atlasInfo.height, 4, nullptr);
        atlases[atlasName] = atlasInfo;
        atlasIt = atlases.find(atlasName);
    }
    
    // Textur laden
    int width, height, channels;
    unsigned char* data = nullptr;
    if (!loadImageData(textureName, width, height, channels, &data)) {
        throw std::runtime_error("Konnte Textur nicht für Atlas laden: " + textureName);
    }
    
    // Region im Atlas finden
    AtlasRegion region;
    region.size = glm::vec2(width, height);
    
    // Einfache Packing-Strategie: Von links nach rechts, von oben nach unten
    float currentX = 0.0f;
    float currentY = 0.0f;
    float maxHeight = 0.0f;
    
    for (const auto& existingRegion : atlasIt->second.regions) {
        currentX = std::max(currentX, existingRegion.second.position.x + existingRegion.second.size.x);
        maxHeight = std::max(maxHeight, existingRegion.second.size.y);
    }
    
    if (currentX + width > atlasIt->second.width) {
        currentX = 0.0f;
        currentY += maxHeight;
        maxHeight = 0.0f;
    }
    
    if (currentY + height > atlasIt->second.height) {
        throw std::runtime_error("Atlas ist voll: " + atlasName);
    }
    
    region.position = glm::vec2(currentX, currentY);
    
    // Texturkoordinaten berechnen
    float texX = currentX / atlasIt->second.width;
    float texY = currentY / atlasIt->second.height;
    float texWidth = width / atlasIt->second.width;
    float texHeight = height / atlasIt->second.height;
    
    region.texCoords[0] = glm::vec2(texX, texY);
    region.texCoords[1] = glm::vec2(texX + texWidth, texY);
    region.texCoords[2] = glm::vec2(texX + texWidth, texY + texHeight);
    region.texCoords[3] = glm::vec2(texX, texY + texHeight);
    
    // Textur in Atlas kopieren
    glBindTexture(GL_TEXTURE_2D, atlasIt->second.id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, currentX, currentY, width, height,
                   channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    
    // Speicher freigeben
    stbi_image_free(data);
    
    // Region speichern
    atlasIt->second.regions[textureName] = region;
    
    return region;
}

TextureManager::AtlasRegion TextureManager::getAtlasRegion(const std::string& atlasName, const std::string& textureName) const {
    auto atlasIt = atlases.find(atlasName);
    if (atlasIt != atlases.end()) {
        auto regionIt = atlasIt->second.regions.find(textureName);
        if (regionIt != atlasIt->second.regions.end()) {
            return regionIt->second;
        }
    }
    return AtlasRegion();
}

void TextureManager::clear() {
    for (const auto& texture : textures) {
        deleteGLTexture(texture.second.id);
    }
    textures.clear();
    
    for (const auto& atlas : atlases) {
        deleteGLTexture(atlas.second.id);
    }
    atlases.clear();
}

void TextureManager::cleanup() {
    clear();
}

unsigned int TextureManager::createGLTexture(int width, int height, int channels, const unsigned char* data) {
    unsigned int textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // Standard-Textur-Parameter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Textur-Daten hochladen
    GLenum format = channels == 4 ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    
    return textureId;
}

void TextureManager::deleteGLTexture(unsigned int id) {
    glDeleteTextures(1, &id);
}

bool TextureManager::loadImageData(const std::string& path, int& width, int& height, int& channels, unsigned char** data) {
    *data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    return *data != nullptr;
}

} // namespace VR_DAW 