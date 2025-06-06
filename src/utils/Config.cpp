#include "Config.hpp"
#include "Logger.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace VR_DAW {

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

Config::Config() {
    initializeDefaults();
}

void Config::initializeDefaults() {
    // Audio-Konfiguration
    audioConfig = AudioConfig();

    // MIDI-Konfiguration
    midiConfig = MIDIConfig();

    // VR-Konfiguration
    vrConfig = VRConfig();

    // Log-Konfiguration
    logConfig = LogConfig();

    // Synthesizer-Konfiguration
    synthConfig = SynthesizerConfig();
}

void Config::setAudioConfig(const AudioConfig& config) {
    std::lock_guard<std::mutex> lock(mutex);
    audioConfig = config;
}

void Config::setMIDIConfig(const MIDIConfig& config) {
    std::lock_guard<std::mutex> lock(mutex);
    midiConfig = config;
}

void Config::setVRConfig(const VRConfig& config) {
    std::lock_guard<std::mutex> lock(mutex);
    vrConfig = config;
}

void Config::setLogConfig(const LogConfig& config) {
    std::lock_guard<std::mutex> lock(mutex);
    logConfig = config;
}

void Config::setSynthesizerConfig(const SynthesizerConfig& config) {
    std::lock_guard<std::mutex> lock(mutex);
    synthConfig = config;
}

bool Config::loadFromFile(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open config file: {}", filename);
            return false;
        }

        nlohmann::json json;
        file >> json;
        updateFromJson(json);

        LOG_INFO("Configuration loaded from {}", filename);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Error loading config file: {}", e.what());
        return false;
    }
}

bool Config::saveToFile(const std::string& filename) const {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open config file for writing: {}", filename);
            return false;
        }

        nlohmann::json json = toJson();
        file << json.dump(4);

        LOG_INFO("Configuration saved to {}", filename);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Error saving config file: {}", e.what());
        return false;
    }
}

void Config::resetToDefaults() {
    std::lock_guard<std::mutex> lock(mutex);
    initializeDefaults();
    LOG_INFO("Configuration reset to defaults");
}

void Config::updateFromJson(const nlohmann::json& json) {
    std::lock_guard<std::mutex> lock(mutex);

    // Audio-Konfiguration
    if (json.contains("audio")) {
        const auto& audio = json["audio"];
        audioConfig.sampleRate = audio.value("sampleRate", audioConfig.sampleRate);
        audioConfig.bufferSize = audio.value("bufferSize", audioConfig.bufferSize);
        audioConfig.numChannels = audio.value("numChannels", audioConfig.numChannels);
        audioConfig.enableMIDI = audio.value("enableMIDI", audioConfig.enableMIDI);
        audioConfig.audioDevice = audio.value("audioDevice", audioConfig.audioDevice);
    }

    // MIDI-Konfiguration
    if (json.contains("midi")) {
        const auto& midi = json["midi"];
        midiConfig.enableClock = midi.value("enableClock", midiConfig.enableClock);
        midiConfig.clockPPQ = midi.value("clockPPQ", midiConfig.clockPPQ);
        midiConfig.defaultTempo = midi.value("defaultTempo", midiConfig.defaultTempo);
        midiConfig.inputDevices = midi.value("inputDevices", midiConfig.inputDevices);
        midiConfig.outputDevices = midi.value("outputDevices", midiConfig.outputDevices);
    }

    // VR-Konfiguration
    if (json.contains("vr")) {
        const auto& vr = json["vr"];
        vrConfig.renderScale = vr.value("renderScale", vrConfig.renderScale);
        vrConfig.renderQuality = vr.value("renderQuality", vrConfig.renderQuality);
        vrConfig.enableDebugRendering = vr.value("enableDebugRendering", vrConfig.enableDebugRendering);
        vrConfig.defaultLayout = vr.value("defaultLayout", vrConfig.defaultLayout);
        vrConfig.uiScale = vr.value("uiScale", vrConfig.uiScale);
        
        if (vr.contains("defaultPosition")) {
            const auto& pos = vr["defaultPosition"];
            vrConfig.defaultPosition = glm::vec3(
                pos[0].get<float>(),
                pos[1].get<float>(),
                pos[2].get<float>()
            );
        }
    }

    // Log-Konfiguration
    if (json.contains("log")) {
        const auto& log = json["log"];
        logConfig.logFile = log.value("logFile", logConfig.logFile);
        logConfig.logLevel = log.value("logLevel", logConfig.logLevel);
        logConfig.enableConsoleOutput = log.value("enableConsoleOutput", logConfig.enableConsoleOutput);
        logConfig.enableFileOutput = log.value("enableFileOutput", logConfig.enableFileOutput);
        logConfig.maxLogSize = log.value("maxLogSize", logConfig.maxLogSize);
        logConfig.maxLogFiles = log.value("maxLogFiles", logConfig.maxLogFiles);
    }

    // Synthesizer-Konfiguration
    if (json.contains("synthesizer")) {
        const auto& synth = json["synthesizer"];
        synthConfig.defaultType = synth.value("defaultType", synthConfig.defaultType);
        synthConfig.defaultVolume = synth.value("defaultVolume", synthConfig.defaultVolume);
        synthConfig.defaultPan = synth.value("defaultPan", synthConfig.defaultPan);
        synthConfig.enableEffects = synth.value("enableEffects", synthConfig.enableEffects);
        synthConfig.maxVoices = synth.value("maxVoices", synthConfig.maxVoices);
    }
}

nlohmann::json Config::toJson() const {
    std::lock_guard<std::mutex> lock(mutex);
    nlohmann::json json;

    // Audio-Konfiguration
    json["audio"] = {
        {"sampleRate", audioConfig.sampleRate},
        {"bufferSize", audioConfig.bufferSize},
        {"numChannels", audioConfig.numChannels},
        {"enableMIDI", audioConfig.enableMIDI},
        {"audioDevice", audioConfig.audioDevice}
    };

    // MIDI-Konfiguration
    json["midi"] = {
        {"enableClock", midiConfig.enableClock},
        {"clockPPQ", midiConfig.clockPPQ},
        {"defaultTempo", midiConfig.defaultTempo},
        {"inputDevices", midiConfig.inputDevices},
        {"outputDevices", midiConfig.outputDevices}
    };

    // VR-Konfiguration
    json["vr"] = {
        {"renderScale", vrConfig.renderScale},
        {"renderQuality", vrConfig.renderQuality},
        {"enableDebugRendering", vrConfig.enableDebugRendering},
        {"defaultLayout", vrConfig.defaultLayout},
        {"uiScale", vrConfig.uiScale},
        {"defaultPosition", {
            vrConfig.defaultPosition.x,
            vrConfig.defaultPosition.y,
            vrConfig.defaultPosition.z
        }}
    };

    // Log-Konfiguration
    json["log"] = {
        {"logFile", logConfig.logFile},
        {"logLevel", logConfig.logLevel},
        {"enableConsoleOutput", logConfig.enableConsoleOutput},
        {"enableFileOutput", logConfig.enableFileOutput},
        {"maxLogSize", logConfig.maxLogSize},
        {"maxLogFiles", logConfig.maxLogFiles}
    };

    // Synthesizer-Konfiguration
    json["synthesizer"] = {
        {"defaultType", synthConfig.defaultType},
        {"defaultVolume", synthConfig.defaultVolume},
        {"defaultPan", synthConfig.defaultPan},
        {"enableEffects", synthConfig.enableEffects},
        {"maxVoices", synthConfig.maxVoices}
    };

    return json;
}

} // namespace VR_DAW 