#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>

namespace VR_DAW {

class TextureManager {
public:
    static TextureManager& getInstance();

    // Textur laden und verwalten
    unsigned int loadTexture(const std::string& path);
    unsigned int createTexture(const std::string& name, int width, int height, const unsigned char* data);
    void deleteTexture(const std::string& name);
    
    // Textur abrufen
    unsigned int getTexture(const std::string& name) const;
    glm::vec2 getTextureSize(const std::string& name) const;
    
    // Textur aktualisieren
    void updateTexture(const std::string& name, const unsigned char* data);
    void updateTextureRegion(const std::string& name, int x, int y, int width, int height, const unsigned char* data);
    
    // Textur-Parameter setzen
    void setTextureFiltering(const std::string& name, bool linear);
    void setTextureWrapping(const std::string& name, bool repeat);
    
    // Textur-Atlas Funktionen
    struct AtlasRegion {
        glm::vec2 position;
        glm::vec2 size;
        glm::vec2 texCoords[4];
    };
    
    AtlasRegion addToAtlas(const std::string& atlasName, const std::string& textureName);
    AtlasRegion getAtlasRegion(const std::string& atlasName, const std::string& textureName) const;
    
    // Speicherverwaltung
    void clear();
    void cleanup();

private:
    TextureManager() = default;
    ~TextureManager();
    
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    
    struct TextureInfo {
        unsigned int id;
        int width;
        int height;
        int channels;
        bool isAtlas;
    };
    
    std::unordered_map<std::string, TextureInfo> textures;
    
    // Atlas-Verwaltung
    struct AtlasInfo {
        unsigned int id;
        int width;
        int height;
        std::unordered_map<std::string, AtlasRegion> regions;
    };
    
    std::unordered_map<std::string, AtlasInfo> atlases;
    
    // Hilfsfunktionen
    unsigned int createGLTexture(int width, int height, int channels, const unsigned char* data);
    void deleteGLTexture(unsigned int id);
    bool loadImageData(const std::string& path, int& width, int& height, int& channels, unsigned char** data);
}; 