#pragma once

#include <memory>
#include <vector>
#include <string>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace VR_DAW {

class AudioEngine {
public:
    static AudioEngine& getInstance();

    // Initialisierung und Konfiguration
    void initialize();
    void shutdown();
    
    // Audio-Stream-Verwaltung
    void startAudioStream();
    void stopAudioStream();
    
    // Plugin-Verwaltung
    void loadPlugin(const std::string& path);
    void unloadPlugin(const std::string& pluginId);
    
    // Effekt-Verwaltung
    void addEffect(const std::string& effectType);
    void removeEffect(const std::string& effectId);
    
    // Dolby Atmos Integration
    void enableDolbyAtmos(bool enable);
    void configureDolbyAtmos(const std::string& config);
    
    // Recording
    void startRecording();
    void stopRecording();
    void saveRecording(const std::string& path);
    
    // Mixing & Mastering
    void setMasterVolume(float volume);
    void setChannelVolume(int channel, float volume);
    void setPan(int channel, float pan);
    
    // AI-Funktionen
    void applyAIMastering();
    void applyAIMixing();
    
    // Sample-Verwaltung
    void loadSample(const std::string& path);
    void updateSampleBank();
    
    // Bouncing
    void bounceToFile(const std::string& path, const std::string& format);

private:
    AudioEngine() = default;
    ~AudioEngine() = default;
    
    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;
    
    // JUCE Audio-Komponenten
    std::unique_ptr<juce::AudioDeviceManager> deviceManager;
    std::unique_ptr<juce::AudioProcessorPlayer> processorPlayer;
    
    // Plugin-Hosting
    std::vector<std::unique_ptr<juce::AudioPluginInstance>> loadedPlugins;
    
    // Effekt-Kette
    std::vector<std::unique_ptr<juce::AudioProcessor>> effects;
    
    // Dolby Atmos
    bool dolbyAtmosEnabled = false;
    std::unique_ptr<class DolbyAtmosProcessor> atmosProcessor;
    
    // Recording
    std::unique_ptr<juce::AudioFormatWriter> recordingWriter;
    std::unique_ptr<juce::FileOutputStream> recordingStream;
    
    // Sample-Bank
    std::vector<std::unique_ptr<juce::AudioFormatReader>> sampleBank;
    
    // AI-Komponenten
    std::unique_ptr<class AIMasteringProcessor> aiMastering;
    std::unique_ptr<class AIMixingProcessor> aiMixing;
}; 