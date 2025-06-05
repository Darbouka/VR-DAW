#include "ReverbPlugin.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace VR_DAW {

ReverbPlugin::ReverbPlugin()
    : bypassed(false)
    , latency(0)
    , sampleRate(44100)
    , bufferSize(1024)
    , feedbackGain(0.5f)
    , diffusionGain(0.7f)
{
    // Standard-Parameter initialisieren
    parameters.roomSize = 0.5f;
    parameters.damping = 0.5f;
    parameters.wetLevel = 0.33f;
    parameters.dryLevel = 0.4f;
    parameters.width = 1.0f;
    parameters.freezeMode = 0.0f;

    // Delay-Lines initialisieren
    for (auto& line : delayLines) {
        line.resize(MAX_DELAY_LENGTH, 0.0f);
    }

    // Feedback-Matrix initialisieren
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    for (auto& value : feedbackMatrix) {
        value = dist(gen);
    }
}

ReverbPlugin::~ReverbPlugin() {
    shutdown();
}

bool ReverbPlugin::initialize(int sampleRate, int bufferSize) {
    this->sampleRate = sampleRate;
    this->bufferSize = bufferSize;
    reset();
    return true;
}

void ReverbPlugin::shutdown() {
    for (auto& line : delayLines) {
        line.clear();
    }
}

void ReverbPlugin::reset() {
    for (auto& line : delayLines) {
        std::fill(line.begin(), line.end(), 0.0f);
    }
}

std::vector<PluginInterface::Parameter> ReverbPlugin::getParameters() const {
    return {
        {"Room Size", parameters.roomSize, 0.0f, 1.0f, 0.5f, false},
        {"Damping", parameters.damping, 0.0f, 1.0f, 0.5f, false},
        {"Wet Level", parameters.wetLevel, 0.0f, 1.0f, 0.33f, false},
        {"Dry Level", parameters.dryLevel, 0.0f, 1.0f, 0.4f, false},
        {"Width", parameters.width, 0.0f, 1.0f, 1.0f, false},
        {"Freeze", parameters.freezeMode, 0.0f, 1.0f, 0.0f, false}
    };
}

void ReverbPlugin::setParameter(const std::string& name, float value) {
    if (name == "Room Size") parameters.roomSize = value;
    else if (name == "Damping") parameters.damping = value;
    else if (name == "Wet Level") parameters.wetLevel = value;
    else if (name == "Dry Level") parameters.dryLevel = value;
    else if (name == "Width") parameters.width = value;
    else if (name == "Freeze") parameters.freezeMode = value;

    updateReverbParameters();
}

float ReverbPlugin::getParameter(const std::string& name) const {
    if (name == "Room Size") return parameters.roomSize;
    if (name == "Damping") return parameters.damping;
    if (name == "Wet Level") return parameters.wetLevel;
    if (name == "Dry Level") return parameters.dryLevel;
    if (name == "Width") return parameters.width;
    if (name == "Freeze") return parameters.freezeMode;
    return 0.0f;
}

void ReverbPlugin::setParameterAutomation(const std::string& name, bool automated) {
    // TODO: Parameter-Automation implementieren
}

void ReverbPlugin::processAudio(AudioBuffer& input, AudioBuffer& output) {
    if (bypassed) {
        std::copy(input.data, input.data + input.size, output.data);
        return;
    }

    processReverb(input.data, output.data, input.size);
}

void ReverbPlugin::processMidi(const std::vector<uint8_t>& midiData) {
    // TODO: MIDI-Verarbeitung implementieren
}

void* ReverbPlugin::createUI() {
    // TODO: UI-Erstellung implementieren
    return nullptr;
}

void ReverbPlugin::destroyUI(void* uiHandle) {
    // TODO: UI-Zerstörung implementieren
}

void ReverbPlugin::updateUI(void* uiHandle) {
    // TODO: UI-Aktualisierung implementieren
}

void ReverbPlugin::resizeUI(void* uiHandle, int width, int height) {
    // TODO: UI-Größenänderung implementieren
}

std::vector<std::string> ReverbPlugin::getPresets() const {
    return {"Default", "Small Room", "Large Hall", "Plate"};
}

void ReverbPlugin::loadPreset(const std::string& name) {
    if (name == "Default") {
        parameters.roomSize = 0.5f;
        parameters.damping = 0.5f;
        parameters.wetLevel = 0.33f;
        parameters.dryLevel = 0.4f;
        parameters.width = 1.0f;
        parameters.freezeMode = 0.0f;
    }
    else if (name == "Small Room") {
        parameters.roomSize = 0.3f;
        parameters.damping = 0.7f;
        parameters.wetLevel = 0.2f;
        parameters.dryLevel = 0.6f;
        parameters.width = 0.8f;
        parameters.freezeMode = 0.0f;
    }
    else if (name == "Large Hall") {
        parameters.roomSize = 0.8f;
        parameters.damping = 0.3f;
        parameters.wetLevel = 0.4f;
        parameters.dryLevel = 0.3f;
        parameters.width = 1.0f;
        parameters.freezeMode = 0.0f;
    }
    else if (name == "Plate") {
        parameters.roomSize = 0.6f;
        parameters.damping = 0.2f;
        parameters.wetLevel = 0.5f;
        parameters.dryLevel = 0.3f;
        parameters.width = 0.9f;
        parameters.freezeMode = 0.0f;
    }

    updateReverbParameters();
}

void ReverbPlugin::savePreset(const std::string& name) {
    // TODO: Preset-Speicherung implementieren
}

void ReverbPlugin::setBypass(bool bypass) {
    bypassed = bypass;
}

bool ReverbPlugin::isBypassed() const {
    return bypassed;
}

int ReverbPlugin::getLatency() const {
    return latency;
}

void ReverbPlugin::setLatency(int samples) {
    latency = samples;
}

void ReverbPlugin::updateReverbParameters() {
    // Feedback-Gain basierend auf Room-Size anpassen
    feedbackGain = parameters.roomSize * 0.9f;
    
    // Diffusion-Gain basierend auf Damping anpassen
    diffusionGain = (1.0f - parameters.damping) * 0.8f;
}

void ReverbPlugin::processReverb(float* input, float* output, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        float inputSample = input[i];
        float outputSample = 0.0f;

        // Delay-Line-Verarbeitung
        for (size_t j = 0; j < delayLines.size(); ++j) {
            float delayedSample = delayLines[j][0];
            delayLines[j].erase(delayLines[j].begin());
            delayLines[j].push_back(inputSample + delayedSample * feedbackGain);
            
            outputSample += delayedSample * diffusionGain;
        }

        // Feedback-Matrix anwenden
        for (float feedback : feedbackMatrix) {
            outputSample += outputSample * feedback * feedbackGain;
        }

        // Wet/Dry-Mix
        output[i] = inputSample * parameters.dryLevel + outputSample * parameters.wetLevel;
    }
}

} // namespace VR_DAW 