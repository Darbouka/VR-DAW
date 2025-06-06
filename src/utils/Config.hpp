#pragma once

#include <string>
#include <map>
#include <variant>
#include <memory>
#include <mutex>
#include <fstream>
#include <nlohmann/json.hpp>

namespace VR_DAW {

class Config {
public:
    static Config& getInstance();

    // Konfigurationswerte
    struct AudioConfig {
        int sampleRate = 44100;
        int bufferSize = 1024;
        int numChannels = 2;
        bool enableMIDI = true;
        std::string audioDevice = "default";
    };

    struct MIDIConfig {
        bool enableClock = true;
        int clockPPQ = 24;
        float defaultTempo = 120.0f;
        std::vector<std::string> inputDevices;
        std::vector<std::string> outputDevices;
    };

    struct VRConfig {
        float renderScale = 1.0f;
        int renderQuality = 1;
        bool enableDebugRendering = false;
        std::string defaultLayout = "default";
        float uiScale = 1.0f;
        glm::vec3 defaultPosition = glm::vec3(0.0f);
    };

    struct LogConfig {
        std::string logFile = "vrdaw.log";
        std::string logLevel = "info";
        bool enableConsoleOutput = true;
        bool enableFileOutput = true;
        int maxLogSize = 10 * 1024 * 1024; // 10 MB
        int maxLogFiles = 5;
    };

    struct SynthesizerConfig {
        std::string defaultType = "subtractive";
        float defaultVolume = 1.0f;
        float defaultPan = 0.0f;
        bool enableEffects = true;
        int maxVoices = 16;
    };

    // Getter und Setter
    const AudioConfig& getAudioConfig() const { return audioConfig; }
    const MIDIConfig& getMIDIConfig() const { return midiConfig; }
    const VRConfig& getVRConfig() const { return vrConfig; }
    const LogConfig& getLogConfig() const { return logConfig; }
    const SynthesizerConfig& getSynthesizerConfig() const { return synthConfig; }

    void setAudioConfig(const AudioConfig& config);
    void setMIDIConfig(const MIDIConfig& config);
    void setVRConfig(const VRConfig& config);
    void setLogConfig(const LogConfig& config);
    void setSynthesizerConfig(const SynthesizerConfig& config);

    // Konfigurationsdatei
    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& filename) const;
    void resetToDefaults();

    // Einzelne Werte
    template<typename T>
    T getValue(const std::string& key) const {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = values.find(key);
        if (it != values.end()) {
            return std::get<T>(it->second);
        }
        return T();
    }

    template<typename T>
    void setValue(const std::string& key, const T& value) {
        std::lock_guard<std::mutex> lock(mutex);
        values[key] = value;
    }

private:
    Config();
    ~Config() = default;

    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    void initializeDefaults();
    void updateFromJson(const nlohmann::json& json);
    nlohmann::json toJson() const;

    AudioConfig audioConfig;
    MIDIConfig midiConfig;
    VRConfig vrConfig;
    LogConfig logConfig;
    SynthesizerConfig synthConfig;

    std::map<std::string, std::variant<int, float, bool, std::string>> values;
    mutable std::mutex mutex;
};

} // namespace VR_DAW 