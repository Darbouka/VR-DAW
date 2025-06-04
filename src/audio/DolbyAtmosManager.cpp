#include "DolbyAtmosManager.hpp"
#include <dolby/dolby_atmos.h>
#include <algorithm>

namespace VR_DAW {

DolbyAtmosManager& DolbyAtmosManager::getInstance() {
    static DolbyAtmosManager instance;
    return instance;
}

bool DolbyAtmosManager::initialize() {
    if (initialized) {
        return true;
    }
    
    try {
        // Dolby Atmos SDK initialisieren
        if (!dolby_atmos_initialize()) {
            lastError = "Fehler bei der Dolby Atmos SDK-Initialisierung";
            return false;
        }
        
        // Standard-Bedienfeld-Konfiguration
        currentBedConfig.numSpeakers = 7;  // 7.1.4 Setup
        
        // Standard-Lautsprecherpositionen (in Metern)
        float defaultPositions[7][3] = {
            {0.0f, 0.0f, 0.0f},      // Center
            {-1.0f, 0.0f, 0.0f},     // Left
            {1.0f, 0.0f, 0.0f},      // Right
            {-0.7f, 0.0f, 0.7f},     // Left Surround
            {0.7f, 0.0f, 0.7f},      // Right Surround
            {0.0f, 0.0f, 1.0f},      // Rear Center
            {0.0f, 0.0f, -1.0f}      // LFE
        };
        
        std::copy(&defaultPositions[0][0], &defaultPositions[0][0] + 21,
                 &currentBedConfig.speakerPositions[0][0]);
        
        // Standard-Lautsprecherverstärkungen
        std::fill(currentBedConfig.speakerGains,
                 currentBedConfig.speakerGains + 32, 1.0f);
        
        // Standard-Metadaten
        currentMetadata.dialnorm = -23.0f;  // Standard-Dialog-Normalisierung
        currentMetadata.drcProfile = 1.0f;  // Standard-DRC-Profil
        currentMetadata.downmix = true;     // Downmix standardmäßig aktiviert
        
        // Renderer initialisieren
        initializeBinauralRenderer();
        initializeObjectRenderer();
        
        initialized = true;
        return true;
    }
    catch (const std::exception& e) {
        lastError = std::string("Initialisierungsfehler: ") + e.what();
        return false;
    }
}

void DolbyAtmosManager::shutdown() {
    if (!initialized) {
        return;
    }
    
    try {
        // Dolby Atmos SDK beenden
        dolby_atmos_shutdown();
        initialized = false;
    }
    catch (const std::exception& e) {
        lastError = std::string("Shutdown-Fehler: ") + e.what();
    }
}

bool DolbyAtmosManager::isInitialized() const {
    return initialized;
}

void DolbyAtmosManager::setRenderMode(RenderMode mode) {
    if (!initialized) {
        return;
    }
    
    currentMode = mode;
    
    // Renderer entsprechend aktualisieren
    switch (mode) {
        case RenderMode::Binaural:
            initializeBinauralRenderer();
            break;
        case RenderMode::ObjectBased:
            initializeObjectRenderer();
            break;
        case RenderMode::Hybrid:
            initializeBinauralRenderer();
            initializeObjectRenderer();
            break;
    }
}

DolbyAtmosManager::RenderMode DolbyAtmosManager::getRenderMode() const {
    return currentMode;
}

void DolbyAtmosManager::addAudioObject(const AudioObject& object) {
    if (!initialized) {
        return;
    }
    
    auto it = std::find_if(audioObjects.begin(), audioObjects.end(),
        [&object](const auto& obj) {
            return obj.id == object.id;
        });
    
    if (it == audioObjects.end()) {
        audioObjects.push_back(object);
    }
}

void DolbyAtmosManager::removeAudioObject(const std::string& objectId) {
    if (!initialized) {
        return;
    }
    
    auto it = std::find_if(audioObjects.begin(), audioObjects.end(),
        [&objectId](const auto& obj) {
            return obj.id == objectId;
        });
    
    if (it != audioObjects.end()) {
        audioObjects.erase(it);
    }
}

void DolbyAtmosManager::updateAudioObject(const AudioObject& object) {
    if (!initialized) {
        return;
    }
    
    auto it = std::find_if(audioObjects.begin(), audioObjects.end(),
        [&object](const auto& obj) {
            return obj.id == object.id;
        });
    
    if (it != audioObjects.end()) {
        *it = object;
    }
}

std::vector<DolbyAtmosManager::AudioObject> DolbyAtmosManager::getAudioObjects() const {
    return audioObjects;
}

void DolbyAtmosManager::setBedConfig(const BedConfig& config) {
    if (!initialized) {
        return;
    }
    
    currentBedConfig = config;
    
    // Dolby Atmos Bedienfeld-Konfiguration aktualisieren
    dolby_atmos_set_bed_config(
        config.numSpeakers,
        reinterpret_cast<const float*>(config.speakerPositions),
        config.speakerGains
    );
}

DolbyAtmosManager::BedConfig DolbyAtmosManager::getBedConfig() const {
    return currentBedConfig;
}

void DolbyAtmosManager::processAudioBlock(juce::AudioBuffer<float>& buffer) {
    if (!initialized) {
        return;
    }
    
    switch (currentMode) {
        case RenderMode::Binaural:
            processBinaural(buffer);
            break;
        case RenderMode::ObjectBased:
            processObjectBased(buffer);
            break;
        case RenderMode::Hybrid:
            processHybrid(buffer);
            break;
    }
}

void DolbyAtmosManager::processAudioObject(const std::string& objectId, juce::AudioBuffer<float>& buffer) {
    if (!initialized) {
        return;
    }
    
    auto it = std::find_if(audioObjects.begin(), audioObjects.end(),
        [&objectId](const auto& obj) {
            return obj.id == objectId;
        });
    
    if (it != audioObjects.end()) {
        // Objekt-basierte Verarbeitung
        dolby_atmos_process_object(
            objectId.c_str(),
            buffer.getReadPointer(0),
            buffer.getReadPointer(1),
            buffer.getNumSamples(),
            it->position,
            it->size,
            it->spread
        );
    }
}

void DolbyAtmosManager::setMetadata(const AtmosMetadata& metadata) {
    if (!initialized) {
        return;
    }
    
    currentMetadata = metadata;
    
    // Dolby Atmos Metadaten aktualisieren
    dolby_atmos_set_metadata(
        metadata.dialnorm,
        metadata.drcProfile,
        metadata.downmix
    );
}

DolbyAtmosManager::AtmosMetadata DolbyAtmosManager::getMetadata() const {
    return currentMetadata;
}

bool DolbyAtmosManager::hasError() const {
    return !lastError.empty();
}

std::string DolbyAtmosManager::getLastError() const {
    return lastError;
}

void DolbyAtmosManager::initializeBinauralRenderer() {
    // Binauraler Renderer initialisieren
    dolby_atmos_initialize_binaural(
        currentMetadata.dialnorm,
        currentMetadata.drcProfile
    );
}

void DolbyAtmosManager::initializeObjectRenderer() {
    // Objekt-basierter Renderer initialisieren
    dolby_atmos_initialize_object_based(
        currentBedConfig.numSpeakers,
        reinterpret_cast<const float*>(currentBedConfig.speakerPositions),
        currentBedConfig.speakerGains
    );
}

void DolbyAtmosManager::updateObjectPositions() {
    for (const auto& object : audioObjects) {
        if (object.isDynamic) {
            dolby_atmos_update_object_position(
                object.id.c_str(),
                object.position,
                object.size,
                object.spread
            );
        }
    }
}

void DolbyAtmosManager::processBinaural(juce::AudioBuffer<float>& buffer) {
    // Binaurale Verarbeitung
    dolby_atmos_process_binaural(
        buffer.getReadPointer(0),
        buffer.getReadPointer(1),
        buffer.getNumSamples(),
        currentMetadata.dialnorm,
        currentMetadata.drcProfile
    );
}

void DolbyAtmosManager::processObjectBased(juce::AudioBuffer<float>& buffer) {
    // Objekt-basierte Verarbeitung
    updateObjectPositions();
    
    dolby_atmos_process_object_based(
        buffer.getReadPointer(0),
        buffer.getReadPointer(1),
        buffer.getNumSamples(),
        currentBedConfig.numSpeakers,
        reinterpret_cast<const float*>(currentBedConfig.speakerPositions),
        currentBedConfig.speakerGains
    );
}

void DolbyAtmosManager::processHybrid(juce::AudioBuffer<float>& buffer) {
    // Hybride Verarbeitung
    juce::AudioBuffer<float> tempBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    tempBuffer.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
    tempBuffer.copyFrom(1, 0, buffer, 1, 0, buffer.getNumSamples());
    
    processBinaural(tempBuffer);
    processObjectBased(buffer);
    
    // Ergebnisse kombinieren
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        buffer.addFrom(channel, 0, tempBuffer, channel, 0, buffer.getNumSamples(), 0.5f);
    }
}

} // namespace VR_DAW 