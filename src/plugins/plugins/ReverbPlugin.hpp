#pragma once

#include "../PluginInterface.hpp"
#include <array>
#include <vector>

namespace VR_DAW {

class ReverbPlugin : public PluginInterface {
public:
    ReverbPlugin();
    ~ReverbPlugin() override;

    // Plugin-Informationen
    std::string getName() const override { return "VR Reverb"; }
    std::string getVendor() const override { return "VR DAW"; }
    std::string getCategory() const override { return "Reverb"; }
    int getVersion() const override { return 1; }

    // Plugin-Lebenszyklus
    bool initialize(int sampleRate, int bufferSize) override;
    void shutdown() override;
    void reset() override;

    // Parameter-Management
    std::vector<Parameter> getParameters() const override;
    void setParameter(const std::string& name, float value) override;
    float getParameter(const std::string& name) const override;
    void setParameterAutomation(const std::string& name, bool automated) override;

    // Audio-Verarbeitung
    void processAudio(AudioBuffer& input, AudioBuffer& output) override;
    void processMidi(const std::vector<uint8_t>& midiData) override;

    // UI-Integration
    void* createUI() override;
    void destroyUI(void* uiHandle) override;
    void updateUI(void* uiHandle) override;
    void resizeUI(void* uiHandle, int width, int height) override;

    // Preset-Management
    std::vector<std::string> getPresets() const override;
    void loadPreset(const std::string& name) override;
    void savePreset(const std::string& name) override;

    // Bypass
    void setBypass(bool bypass) override;
    bool isBypassed() const override;

    // Latency
    int getLatency() const override;
    void setLatency(int samples) override;

private:
    struct ReverbParameters {
        float roomSize;
        float damping;
        float wetLevel;
        float dryLevel;
        float width;
        float freezeMode;
    };

    void updateReverbParameters();
    void processReverb(float* input, float* output, int numSamples);

    ReverbParameters parameters;
    bool bypassed;
    int latency;
    int sampleRate;
    int bufferSize;

    // Reverb-Implementierung
    static constexpr size_t MAX_DELAY_LENGTH = 44100 * 2; // 2 Sekunden bei 44.1kHz
    std::array<std::vector<float>, 2> delayLines;
    std::array<float, 8> feedbackMatrix;
    float feedbackGain;
    float diffusionGain;
};

class ReverbPluginFactory : public PluginFactory {
public:
    std::unique_ptr<PluginInterface> createPlugin() override {
        return std::make_unique<ReverbPlugin>();
    }

    std::string getPluginType() const override {
        return "Reverb";
    }
};

} // namespace VR_DAW 