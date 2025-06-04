#pragma once

#include <memory>
#include <vector>
#include <string>
#include <juce_audio_basics/juce_audio_basics.h>

namespace VR_DAW {

class DolbyAtmosProcessor {
public:
    static DolbyAtmosProcessor& getInstance();
    
    // Initialisierung
    void initialize();
    void shutdown();
    
    // Konfiguration
    void configure(const std::string& config);
    void setChannelCount(int count);
    void setObjectCount(int count);
    
    // Verarbeitung
    void processBuffer(juce::AudioBuffer<float>& buffer);
    void processObject(const std::string& objectId, const juce::AudioBuffer<float>& buffer);
    
    // Rendering
    void renderToBinaural(juce::AudioBuffer<float>& output);
    void renderToMultichannel(juce::AudioBuffer<float>& output);
    
    // Objekt-Management
    void addObject(const std::string& objectId, const std::string& metadata);
    void removeObject(const std::string& objectId);
    void updateObjectPosition(const std::string& objectId, float x, float y, float z);
    
    // Bedienung
    void enable(bool enable);
    void setMode(const std::string& mode); // "Binaural", "Multichannel", "Hybrid"
    void setQuality(const std::string& quality); // "Low", "Medium", "High"
    
private:
    DolbyAtmosProcessor() = default;
    ~DolbyAtmosProcessor() = default;
    
    DolbyAtmosProcessor(const DolbyAtmosProcessor&) = delete;
    DolbyAtmosProcessor& operator=(const DolbyAtmosProcessor&) = delete;
    
    // Interne Zustandsvariablen
    bool enabled = false;
    int channelCount = 2;
    int objectCount = 0;
    std::string currentMode = "Binaural";
    std::string currentQuality = "High";
    
    // Objekt-Speicher
    struct AtmosObject {
        std::string id;
        std::string metadata;
        float position[3];
        juce::AudioBuffer<float> buffer;
    };
    std::vector<AtmosObject> objects;
    
    // Verarbeitungs-Komponenten
    std::unique_ptr<class AtmosRenderer> renderer;
    std::unique_ptr<class AtmosMixer> mixer;
    std::unique_ptr<class AtmosSpatializer> spatializer;
    
    // Interne Hilfsfunktionen
    void initializeRenderer();
    void initializeMixer();
    void initializeSpatializer();
    void updateObjectBuffers();
    void processObjects();
};

} // namespace VR_DAW 