#pragma once

#include <string>
#include <vector>
#include <memory>
#include <juce_audio_basics/juce_audio_basics.h>

namespace VR_DAW {

class DolbyAtmosManager {
public:
    static DolbyAtmosManager& getInstance();
    
    // Initialisierung und Konfiguration
    bool initialize();
    void shutdown();
    bool isInitialized() const;
    
    // Rendering-Modus
    enum class RenderMode {
        Binaural,      // Für Kopfhörer
        ObjectBased,   // Für Lautsprecher
        Hybrid         // Kombiniert beide Modi
    };
    
    void setRenderMode(RenderMode mode);
    RenderMode getRenderMode() const;
    
    // Objekt-basierte Audio-Verwaltung
    struct AudioObject {
        std::string id;
        float position[3];  // x, y, z
        float size;        // Objektgröße
        float spread;      // Ausbreitung
        bool isDynamic;    // Dynamische Positionierung
    };
    
    void addAudioObject(const AudioObject& object);
    void removeAudioObject(const std::string& objectId);
    void updateAudioObject(const AudioObject& object);
    std::vector<AudioObject> getAudioObjects() const;
    
    // Bedienfeld-Konfiguration
    struct BedConfig {
        int numSpeakers;
        float speakerPositions[32][3];  // Maximale 32 Lautsprecher
        float speakerGains[32];
    };
    
    void setBedConfig(const BedConfig& config);
    BedConfig getBedConfig() const;
    
    // Verarbeitung
    void processAudioBlock(juce::AudioBuffer<float>& buffer);
    void processAudioObject(const std::string& objectId, juce::AudioBuffer<float>& buffer);
    
    // Metadaten
    struct AtmosMetadata {
        float dialnorm;
        float drcProfile;
        bool downmix;
    };
    
    void setMetadata(const AtmosMetadata& metadata);
    AtmosMetadata getMetadata() const;
    
    // Status und Fehlerbehandlung
    bool hasError() const;
    std::string getLastError() const;
    
private:
    DolbyAtmosManager() = default;
    ~DolbyAtmosManager() = default;
    
    // Singleton-Pattern
    DolbyAtmosManager(const DolbyAtmosManager&) = delete;
    DolbyAtmosManager& operator=(const DolbyAtmosManager&) = delete;
    
    // Interne Zustandsvariablen
    bool initialized = false;
    RenderMode currentMode = RenderMode::Binaural;
    std::vector<AudioObject> audioObjects;
    BedConfig currentBedConfig;
    AtmosMetadata currentMetadata;
    
    // Fehlerbehandlung
    std::string lastError;
    
    // Interne Hilfsfunktionen
    void initializeBinauralRenderer();
    void initializeObjectRenderer();
    void updateObjectPositions();
    void processBinaural(juce::AudioBuffer<float>& buffer);
    void processObjectBased(juce::AudioBuffer<float>& buffer);
    void processHybrid(juce::AudioBuffer<float>& buffer);
};

} // namespace VR_DAW 