#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace VR_DAW {

class PluginInterface {
public:
    struct Parameter {
        std::string name;
        float value;
        float minValue;
        float maxValue;
        float defaultValue;
        bool isAutomated;
    };

    struct AudioBuffer {
        float* data;
        size_t size;
        int channels;
        int sampleRate;
    };

    virtual ~PluginInterface() = default;

    // Plugin-Informationen
    virtual std::string getName() const = 0;
    virtual std::string getVendor() const = 0;
    virtual std::string getCategory() const = 0;
    virtual int getVersion() const = 0;

    // Plugin-Lebenszyklus
    virtual bool initialize(int sampleRate, int bufferSize) = 0;
    virtual void shutdown() = 0;
    virtual void reset() = 0;

    // Parameter-Management
    virtual std::vector<Parameter> getParameters() const = 0;
    virtual void setParameter(const std::string& name, float value) = 0;
    virtual float getParameter(const std::string& name) const = 0;
    virtual void setParameterAutomation(const std::string& name, bool automated) = 0;

    // Audio-Verarbeitung
    virtual void processAudio(AudioBuffer& input, AudioBuffer& output) = 0;
    virtual void processMidi(const std::vector<uint8_t>& midiData) = 0;

    // UI-Integration
    virtual void* createUI() = 0;
    virtual void destroyUI(void* uiHandle) = 0;
    virtual void updateUI(void* uiHandle) = 0;
    virtual void resizeUI(void* uiHandle, int width, int height) = 0;

    // Preset-Management
    virtual std::vector<std::string> getPresets() const = 0;
    virtual void loadPreset(const std::string& name) = 0;
    virtual void savePreset(const std::string& name) = 0;

    // Bypass
    virtual void setBypass(bool bypass) = 0;
    virtual bool isBypassed() const = 0;

    // Latency
    virtual int getLatency() const = 0;
    virtual void setLatency(int samples) = 0;
};

// Plugin-Factory für die Erstellung von Plugin-Instanzen
class PluginFactory {
public:
    virtual ~PluginFactory() = default;
    virtual std::unique_ptr<PluginInterface> createPlugin() = 0;
    virtual std::string getPluginType() const = 0;
};

// Plugin-Manager für die Verwaltung aller Plugins
class PluginManager {
public:
    static PluginManager& getInstance() {
        static PluginManager instance;
        return instance;
    }

    void registerPluginFactory(std::unique_ptr<PluginFactory> factory) {
        factories[factory->getPluginType()] = std::move(factory);
    }

    std::unique_ptr<PluginInterface> createPlugin(const std::string& type) {
        auto it = factories.find(type);
        if (it != factories.end()) {
            return it->second->createPlugin();
        }
        return nullptr;
    }

    std::vector<std::string> getAvailablePluginTypes() const {
        std::vector<std::string> types;
        for (const auto& [type, _] : factories) {
            types.push_back(type);
        }
        return types;
    }

private:
    PluginManager() = default;
    std::map<std::string, std::unique_ptr<PluginFactory>> factories;
};

} // namespace VR_DAW 