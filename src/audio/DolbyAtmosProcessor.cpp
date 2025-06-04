#include "DolbyAtmosProcessor.hpp"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>

namespace VR_DAW {

DolbyAtmosProcessor& DolbyAtmosProcessor::getInstance() {
    static DolbyAtmosProcessor instance;
    return instance;
}

void DolbyAtmosProcessor::initialize() {
    initializeRenderer();
    initializeMixer();
    initializeSpatializer();
    enabled = true;
}

void DolbyAtmosProcessor::shutdown() {
    enabled = false;
    renderer.reset();
    mixer.reset();
    spatializer.reset();
    objects.clear();
}

void DolbyAtmosProcessor::configure(const std::string& config) {
    // Konfiguration aus JSON parsen und anwenden
    // Hier würde die Implementierung der Konfigurationslogik folgen
}

void DolbyAtmosProcessor::setChannelCount(int count) {
    channelCount = count;
    if (mixer) {
        mixer->setChannelCount(count);
    }
}

void DolbyAtmosProcessor::setObjectCount(int count) {
    objectCount = count;
    objects.resize(count);
}

void DolbyAtmosProcessor::processBuffer(juce::AudioBuffer<float>& buffer) {
    if (!enabled) return;
    
    // Verarbeite den Haupt-Audio-Buffer
    if (mixer) {
        mixer->process(buffer);
    }
    
    // Verarbeite alle Objekte
    processObjects();
    
    // Rendere das Ergebnis
    if (currentMode == "Binaural") {
        renderToBinaural(buffer);
    } else if (currentMode == "Multichannel") {
        renderToMultichannel(buffer);
    }
}

void DolbyAtmosProcessor::processObject(const std::string& objectId, const juce::AudioBuffer<float>& buffer) {
    auto it = std::find_if(objects.begin(), objects.end(),
        [&objectId](const AtmosObject& obj) { return obj.id == objectId; });
    
    if (it != objects.end()) {
        it->buffer = buffer;
    }
}

void DolbyAtmosProcessor::renderToBinaural(juce::AudioBuffer<float>& output) {
    if (renderer) {
        renderer->renderBinaural(output);
    }
}

void DolbyAtmosProcessor::renderToMultichannel(juce::AudioBuffer<float>& output) {
    if (renderer) {
        renderer->renderMultichannel(output);
    }
}

void DolbyAtmosProcessor::addObject(const std::string& objectId, const std::string& metadata) {
    AtmosObject obj;
    obj.id = objectId;
    obj.metadata = metadata;
    obj.position[0] = obj.position[1] = obj.position[2] = 0.0f;
    objects.push_back(obj);
}

void DolbyAtmosProcessor::removeObject(const std::string& objectId) {
    objects.erase(
        std::remove_if(objects.begin(), objects.end(),
            [&objectId](const AtmosObject& obj) { return obj.id == objectId; }),
        objects.end());
}

void DolbyAtmosProcessor::updateObjectPosition(const std::string& objectId, float x, float y, float z) {
    auto it = std::find_if(objects.begin(), objects.end(),
        [&objectId](const AtmosObject& obj) { return obj.id == objectId; });
    
    if (it != objects.end()) {
        it->position[0] = x;
        it->position[1] = y;
        it->position[2] = z;
    }
}

void DolbyAtmosProcessor::enable(bool enable) {
    enabled = enable;
}

void DolbyAtmosProcessor::setMode(const std::string& mode) {
    currentMode = mode;
    if (renderer) {
        renderer->setMode(mode);
    }
}

void DolbyAtmosProcessor::setQuality(const std::string& quality) {
    currentQuality = quality;
    if (renderer) {
        renderer->setQuality(quality);
    }
}

void DolbyAtmosProcessor::initializeRenderer() {
    renderer = std::make_unique<AtmosRenderer>();
    renderer->initialize();
    renderer->setMode(currentMode);
    renderer->setQuality(currentQuality);
}

void DolbyAtmosProcessor::initializeMixer() {
    mixer = std::make_unique<AtmosMixer>();
    mixer->initialize();
    mixer->setChannelCount(channelCount);
}

void DolbyAtmosProcessor::initializeSpatializer() {
    spatializer = std::make_unique<AtmosSpatializer>();
    spatializer->initialize();
}

void DolbyAtmosProcessor::updateObjectBuffers() {
    for (auto& obj : objects) {
        if (spatializer) {
            spatializer->processObject(obj.buffer, obj.position);
        }
    }
}

void DolbyAtmosProcessor::processObjects() {
    updateObjectBuffers();
    if (mixer) {
        for (const auto& obj : objects) {
            mixer->addObject(obj.buffer, obj.position);
        }
    }
}

} // namespace VR_DAW 